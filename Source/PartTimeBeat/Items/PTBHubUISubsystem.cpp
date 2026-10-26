#include "Items/PTBHubUISubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Core/PTBGameModeBase.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/World.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Interface/PTBHubInteractable.h"
#include "Items/PTBItemSettings.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PTBItemPopupWidget.h"

bool UPTBHubUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && (World->WorldType == EWorldType::Game || World->WorldType == EWorldType::PIE);
}

void UPTBHubUISubsystem::Deinitialize()
{
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
	const UWorld* World = GetWorld();
	if (!World || !GetLocalPlayerController() || IsPopupOpen() || !IsHubWorld())
	{
		return false;
	}

	if (UGameplayStatics::IsGamePaused(World))
	{
		return false;
	}

	if (const APTBGameModeBase* GM = World->GetAuthGameMode<APTBGameModeBase>())
	{
		if (GM->bIsPaused || GM->ActiveMiniGame)
		{
			return false;
		}
	}
	return true;
}

APlayerController* UPTBHubUISubsystem::GetLocalPlayerController() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetFirstPlayerController() : nullptr;
}

// ── 팝업 ─────────────────────────────────────────────────────────

bool UPTBHubUISubsystem::OpenLocker()
{
	if (!CanOpenPopup())
	{
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
	if (!CanOpenPopup())
	{
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
}

// ── 상호작용 ─────────────────────────────────────────────────────

void UPTBHubUISubsystem::SetInteractTarget(AActor* Target)
{
	if (InteractTarget.Get() == Target)
	{
		return;
	}
	InteractTarget = Target;
	OnInteractTargetChanged.Broadcast(Target);
}

void UPTBHubUISubsystem::ClearInteractTarget(AActor* Target)
{
	if (InteractTarget.Get() != Target)
	{
		return;
	}
	InteractTarget.Reset();
	OnInteractTargetChanged.Broadcast(nullptr);
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
	if (!IsHubWorld())
	{
		return false;
	}

	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (!Settings)
	{
		return false;
	}

	if (Key == Settings->LockerKey)
	{
		return OpenLocker();
	}

	if (Settings->InteractKeys.Contains(Key))
	{
		// 상호작용 대상이 없으면 소비하지 않음 → 기존 Enter 처리(미니게임 입구 등)가 그대로 동작
		return TryInteract();
	}

	return false;
}
