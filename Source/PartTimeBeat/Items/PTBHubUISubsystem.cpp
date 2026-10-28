#include "Items/PTBHubUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Core/PTBGameModeBase.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/World.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Interface/PTBHubInteractable.h"
#include "Items/PTBItemSettings.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PTBItemPopupWidget.h"
#include "UI/PTBKeyPromptWidget.h"

bool UPTBHubUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UPTBHubUISubsystem::Deinitialize()
{
	if (InteractPrompt)
	{
		InteractPrompt->RemoveFromParent();
		InteractPrompt = nullptr;
	}
	ActivePopup = nullptr;
	InteractTarget.Reset();
	Super::Deinitialize();
}

// ── 상태 ─────────────────────────────────────────────────────────

bool UPTBHubUISubsystem::IsHubWorld() const
{
	FName HubLevelName = TEXT("L_GameMap");
	if (const UGameInstance* GI = GetWorld() ? GetWorld()->GetGameInstance() : nullptr)
	{
		if (const UPTBGameFlowSubsystem* Flow = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			if (!Flow->MiniGameSelectLevelName.IsNone())
			{
				HubLevelName = Flow->MiniGameSelectLevelName;
			}
		}
	}
	return UGameplayStatics::GetCurrentLevelName(GetWorld(), true).Equals(HubLevelName.ToString(), ESearchCase::IgnoreCase);
}

bool UPTBHubUISubsystem::IsPopupOpen() const
{
	return ActivePopup && ActivePopup->IsPopupOpen();
}

bool UPTBHubUISubsystem::CanOpenPopup() const
{
	return GetBlockReason().IsEmpty();
}

FString UPTBHubUISubsystem::GetBlockReason() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return TEXT("World 없음");
	}
	if (!GetLocalPlayerController())
	{
		return TEXT("PlayerController 없음");
	}
	if (IsPopupOpen())
	{
		return TEXT("이미 다른 팝업이 열려 있음");
	}
	if (!IsHubWorld())
	{
		return FString::Printf(TEXT("허브 맵이 아님 (현재 레벨: %s)"), *UGameplayStatics::GetCurrentLevelName(World, true));
	}
	if (UGameplayStatics::IsGamePaused(World))
	{
		return TEXT("게임 일시정지 중");
	}
	if (const APTBGameModeBase* GM = World->GetAuthGameMode<APTBGameModeBase>())
	{
		if (GM->bIsPaused)
		{
			return TEXT("GameMode bIsPaused=true");
		}
		if (GM->ActiveMiniGame)
		{
			return FString::Printf(TEXT("미니게임 진행 중 (%s)"), *GM->ActiveMiniGame->GetName());
		}
	}
	return FString();
}

APlayerController* UPTBHubUISubsystem::GetLocalPlayerController() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}

// ── 팝업 ─────────────────────────────────────────────────────────

bool UPTBHubUISubsystem::OpenLocker()
{
	const FString BlockReason = GetBlockReason();
	if (!BlockReason.IsEmpty())
	{
		PTB_WARNING(LogPTBProfile, TEXT("[HubUI] 사물함 열기 거부: %s"), *BlockReason);
		return false;
	}

	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	UClass* WidgetClass = Settings ? Settings->LockerWidgetClass.LoadSynchronous() : nullptr;
	if (!WidgetClass)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[HubUI] LockerWidgetClass가 설정되지 않음 (Project Settings → Game → PTB 아이템 설정)"));
		return false;
	}

	UPTBLockerWidget* Locker = CreateWidget<UPTBLockerWidget>(GetLocalPlayerController(), WidgetClass);
	return ShowPopup(Locker);
}

bool UPTBHubUISubsystem::OpenShop(const FText& ShopName, const TArray<FName>& SoldCategories, TSubclassOf<UPTBShopWidget> WidgetClassOverride)
{
	const FString BlockReason = GetBlockReason();
	if (!BlockReason.IsEmpty())
	{
		PTB_WARNING(LogPTBProfile, TEXT("[HubUI] 상점 열기 거부: %s"), *BlockReason);
		return false;
	}

	UClass* WidgetClass = WidgetClassOverride.Get();
	if (!WidgetClass)
	{
		const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
		WidgetClass = Settings ? Settings->ShopWidgetClass.LoadSynchronous() : nullptr;
	}
	if (!WidgetClass)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[HubUI] ShopWidgetClass가 설정되지 않음 (PTB 아이템 설정 또는 상점 오브젝트)"));
		return false;
	}

	UPTBShopWidget* Shop = CreateWidget<UPTBShopWidget>(GetLocalPlayerController(), WidgetClass);
	if (Shop)
	{
		Shop->InitShop(ShopName, SoldCategories);
	}
	return ShowPopup(Shop);
}

bool UPTBHubUISubsystem::ShowPopup(UPTBHubPopupWidget* Popup)
{
	if (!Popup)
	{
		return false;
	}

	ActivePopup = Popup;
	Popup->OnPopupClosedNative.AddUObject(this, &UPTBHubUISubsystem::HandlePopupClosed);
	Popup->OpenPopup();
	UpdateInteractPrompt(); // 팝업 위로 안내가 겹치지 않게 숨김

	PTB_RECORD(LogPTBProfile, TEXT("[HubUI] Popup opened: %s"), *Popup->GetClass()->GetName());
	return true;
}

void UPTBHubUISubsystem::CloseActivePopup()
{
	if (ActivePopup)
	{
		ActivePopup->ClosePopup(); // → HandlePopupClosed
	}
}

void UPTBHubUISubsystem::HandlePopupClosed()
{
	if (ActivePopup)
	{
		ActivePopup->OnPopupClosedNative.RemoveAll(this);
	}
	ActivePopup = nullptr;
	UpdateInteractPrompt(); // 상점 앞에서 닫았으면 안내 다시 표시
}

// ── 상호작용 ─────────────────────────────────────────────────────

void UPTBHubUISubsystem::SetInteractTarget(AActor* Target)
{
	if (InteractTarget.Get() == Target)
	{
		return;
	}
	InteractTarget = Target;
	UpdateInteractPrompt();
	OnInteractTargetChanged.Broadcast(Target);
}

void UPTBHubUISubsystem::ClearInteractTarget(AActor* Target)
{
	if (InteractTarget.Get() != Target)
	{
		return;
	}
	InteractTarget.Reset();
	UpdateInteractPrompt();
	OnInteractTargetChanged.Broadcast(nullptr);
}

void UPTBHubUISubsystem::UpdateInteractPrompt()
{
	AActor* Target = InteractTarget.Get();
	const bool bShow = Target && !IsPopupOpen()
		&& Target->GetClass()->ImplementsInterface(UPTBHubInteractable::StaticClass());

	if (!bShow)
	{
		if (InteractPrompt)
		{
			InteractPrompt->RemoveFromParent();
		}
		return;
	}

	if (!InteractPrompt)
	{
		const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
		UClass* PromptClass = Settings ? Settings->InteractPromptWidgetClass.LoadSynchronous() : nullptr;
		APlayerController* PC = GetLocalPlayerController();
		if (!PromptClass || !PC)
		{
			return; // 안내 위젯 미지정 시 안내 없이 동작
		}
		InteractPrompt = CreateWidget<UPTBKeyPromptWidget>(PC, PromptClass);
		if (!InteractPrompt)
		{
			return;
		}
	}

	// 위젯 기본 문구를 기억해 두고, 대상이 문구를 지정하면 그걸로 교체
	if (DefaultPromptText.IsEmpty())
	{
		DefaultPromptText = InteractPrompt->GetPromptText();
	}
	const FText TargetText = IPTBHubInteractable::Execute_GetInteractPromptText(Target);
	InteractPrompt->SetPromptText(TargetText.IsEmpty() ? DefaultPromptText : TargetText);

	if (!InteractPrompt->IsInViewport())
	{
		InteractPrompt->AddToViewport(10);
	}
}

bool UPTBHubUISubsystem::TryInteract()
{
	AActor* Target = InteractTarget.Get();
	if (!Target || !Target->GetClass()->ImplementsInterface(UPTBHubInteractable::StaticClass()))
	{
		return false;
	}
	if (!CanOpenPopup())
	{
		return false;
	}
	return IPTBHubInteractable::Execute_OnHubInteract(Target, GetLocalPlayerController());
}

// ── 입력 ─────────────────────────────────────────────────────────

bool UPTBHubUISubsystem::HandleHubKeyPressed(FKey Key)
{
	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (!Settings)
	{
		return false;
	}

	if (Key == Settings->LockerKey)
	{
		// 허브가 아니면 조용히 무시 (미니게임 맵 등에서 로그 도배 방지)
		if (!IsHubWorld())
		{
			return false;
		}
		PTB_RECORD(LogPTBProfile, TEXT("[HubUI] 사물함 키 입력 수신 (%s)"), *Key.ToString());
		return OpenLocker();
	}

	if (Settings->InteractKeys.Contains(Key))
	{
		// 상호작용 대상이 없으면 소비하지 않음 → 기존 Enter 처리(미니게임 입구 등)가 그대로 동작
		return IsHubWorld() && TryInteract();
	}

	return false;
}
