#include "Judgement/PTBJudgementDisplayComponent.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/PTBItemSettings.h"
#include "Items/PTBItemSubsystem.h"
#include "Items/PTBItemTypes.h"
#include "Judgement/PTBJudgeSkin.h"
#include "Judgement/PTBJudgementDisplayWidget.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"

UPTBJudgementDisplayComponent::UPTBJudgementDisplayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

APTBBaseMiniGame* UPTBJudgementDisplayComponent::GetMiniGame() const
{
	return Cast<APTBBaseMiniGame>(GetOwner());
}

void UPTBJudgementDisplayComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APTBBaseMiniGame* MiniGame = GetMiniGame())
	{
		MiniGame->OnMiniGameJudgement.AddUniqueDynamic(this, &UPTBJudgementDisplayComponent::HandleMiniGameJudgement);
		MiniGame->OnMiniGameStarted.AddUniqueDynamic(this, &UPTBJudgementDisplayComponent::HandleMiniGameStarted);
	}
	if (UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		Items->OnItemStateChanged.AddUniqueDynamic(this, &UPTBJudgementDisplayComponent::HandleItemStateChanged);
	}
}

UPTBItemSubsystem* UPTBJudgementDisplayComponent::GetItemSubsystem() const
{
	const UWorld* World = GetWorld();
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	return GI ? GI->GetSubsystem<UPTBItemSubsystem>() : nullptr;
}

void UPTBJudgementDisplayComponent::HandleMiniGameStarted()
{
	bSkinResolved = false;
}

void UPTBJudgementDisplayComponent::HandleItemStateChanged(FName Category)
{
	if (Category == PTBItemCategory::JudgeSkin)
	{
		bSkinResolved = false;
	}
}

void UPTBJudgementDisplayComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APTBBaseMiniGame* MiniGame = GetMiniGame())
	{
		MiniGame->OnMiniGameJudgement.RemoveDynamic(this, &UPTBJudgementDisplayComponent::HandleMiniGameJudgement);
		MiniGame->OnMiniGameStarted.RemoveDynamic(this, &UPTBJudgementDisplayComponent::HandleMiniGameStarted);
	}
	if (UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		Items->OnItemStateChanged.RemoveDynamic(this, &UPTBJudgementDisplayComponent::HandleItemStateChanged);
	}

	for (const TPair<int32, TObjectPtr<UPTBJudgementDisplayWidget>>& Pair : Widgets)
	{
		if (Pair.Value)
		{
			Pair.Value->RemoveFromParent();
		}
	}
	Widgets.Reset();

	Super::EndPlay(EndPlayReason);
}

void UPTBJudgementDisplayComponent::RefreshSkin()
{
	bSkinResolved = true;
	ActiveSkin = nullptr;

	// 1순위: 장착한 판정 스킨 아이템의 Asset
	FName EquippedId = NAME_None;
	if (const UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		EquippedId = Items->GetEquippedItemId(PTBItemCategory::JudgeSkin);
		bool bFound = false;
		const FPTBItemRow Row = Items->GetEquippedItemData(PTBItemCategory::JudgeSkin, bFound);
		if (bFound && !Row.Asset.IsNull())
		{
			ActiveSkin = Cast<UPTBJudgeSkin>(Row.Asset.LoadSynchronous());
			if (!ActiveSkin)
			{
				PTB_WARNING(LogPTBMiniGames, TEXT("[JudgeDisplay] 판정 스킨 아이템의 Asset이 PTBJudgeSkin이 아님: %s"), *Row.Asset.ToString());
			}
		}
	}

	// 2순위: PTB 아이템 설정의 기본 판정 스킨 (장착 정보가 없을 때, 직접 PIE 등)
	if (!ActiveSkin)
	{
		if (const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>())
		{
			ActiveSkin = Settings->FallbackJudgeSkin.LoadSynchronous();
		}
	}

	PTB_RECORD(LogPTBMiniGames, TEXT("[JudgeDisplay] 판정 스킨: item=%s skin=%s"), *EquippedId.ToString(), *GetNameSafe(ActiveSkin));

	if (!ActiveSkin)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[JudgeDisplay] 사용할 판정 스킨 없음 (JudgeSkin 아이템 Asset 또는 PTB 아이템 설정의 FallbackJudgeSkin 확인)"));
	}
}

UPTBJudgementDisplayWidget* UPTBJudgementDisplayComponent::GetOrCreateWidget(int32 SlotIndex)
{
	if (const TObjectPtr<UPTBJudgementDisplayWidget>* Found = Widgets.Find(SlotIndex))
	{
		if (*Found)
		{
			return *Found;
		}
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC)
	{
		return nullptr;
	}

	TSubclassOf<UPTBJudgementDisplayWidget> WidgetClass = UPTBJudgementDisplayWidget::StaticClass();
	if (const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>())
	{
		if (UClass* Custom = Settings->JudgementDisplayWidgetClass.LoadSynchronous())
		{
			WidgetClass = Custom;
		}
	}

	UPTBJudgementDisplayWidget* Widget = CreateWidget<UPTBJudgementDisplayWidget>(PC, WidgetClass);
	if (!Widget)
	{
		return nullptr;
	}

	Widget->AddToViewport(20);
	Widget->SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	Widgets.Add(SlotIndex, Widget);
	return Widget;
}

void UPTBJudgementDisplayComponent::HandleMiniGameJudgement(FPTBJudgementResult Result)
{
	APTBBaseMiniGame* MiniGame = GetMiniGame();
	const UPTBMiniGameRuleSet* RuleSet = MiniGame ? MiniGame->RuleSet.Get() : nullptr;
	if (!RuleSet || !RuleSet->bUseCommonJudgementDisplay)
	{
		return;
	}

	// 판정으로 치지 않는 헛입력은 표시하지 않음
	if (Result.Reason == EPTBJudgementReason::EmptyInput && !RuleSet->ShouldTreatEmptyInputAsMiss())
	{
		return;
	}

	if (!bSkinResolved)
	{
		RefreshSkin();
	}
	if (!ActiveSkin)
	{
		return;
	}

	// 위치: 미니게임이 지정 (월드 / 화면 UI), 없으면 RuleSet 화면 위치
	const FPTBJudgementAnchor Anchor = MiniGame->GetJudgementDisplayAnchor(Result);

	UPTBJudgementDisplayWidget* Widget = GetOrCreateWidget(Anchor.Type == EPTBJudgementAnchorType::Default ? 0 : Anchor.SlotIndex);
	if (!Widget)
	{
		return;
	}

	switch (Anchor.Type)
	{
	case EPTBJudgementAnchorType::WorldLocation:
		Widget->SetWorldAnchor(Anchor.WorldLocation + RuleSet->JudgementWorldOffset);
		break;
	case EPTBJudgementAnchorType::ScreenPosition:
		Widget->SetViewportAnchor(Anchor.ScreenPosition + RuleSet->JudgementScreenAnchorOffset);
		break;
	default:
		Widget->SetScreenAnchor(RuleSet->JudgementScreenPosition);
		break;
	}

	Widget->PlayJudgement(Result.JudgementType, ActiveSkin, RuleSet->JudgementDisplayScale);
}

FPTBJudgementAnchor UPTBJudgementDisplayComponent::MakeJudgementAnchorFromWidget(UWidget* Widget, int32 SlotIndex)
{
	FPTBJudgementAnchor Anchor;
	Anchor.SlotIndex = SlotIndex;

	if (!Widget)
	{
		return Anchor;
	}

	const FGeometry& Geometry = Widget->GetCachedGeometry();
	if (Geometry.GetLocalSize().IsNearlyZero())
	{
		return Anchor; // 아직 그려지지 않음
	}

	// 위젯 중심의 절대 좌표 → 뷰포트 위치 (Set Position in Viewport 단위)
	const FVector2D AbsoluteCenter = Geometry.GetAbsolutePositionAtCoordinates(FVector2D(0.5f, 0.5f));
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::AbsoluteToViewport(Widget, AbsoluteCenter, PixelPosition, ViewportPosition);

	Anchor.Type = EPTBJudgementAnchorType::ScreenPosition;
	Anchor.ScreenPosition = ViewportPosition;
	return Anchor;
}
