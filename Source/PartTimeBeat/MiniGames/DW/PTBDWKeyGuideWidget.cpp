#include "MiniGames/DW/PTBDWKeyGuideWidget.h"

#include "MiniGames/DW/PTBDWMiniGame.h"
#include "MiniGames/DW/PTBDWKeySlotWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UI/PTBKeyTextLibrary.h"
#include "MiniGames/Common/PTBMiniGameIntroData.h"

void UPTBDWKeyGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CollectSlots();
	bSlotsDirty = true;
	// 처음엔 숨김 → Tick에서 입력 상태에 맞춰 표시
	bGuideShown = true;
	SetGuideShown(bAlwaysVisible);
}

void UPTBDWKeyGuideWidget::NativeDestruct()
{
	if (APTBDWMiniGame* MiniGame = DWMiniGame.Get())
	{
		MiniGame->OnRhythmKeyStateChanged.RemoveDynamic(this, &UPTBDWKeyGuideWidget::HandleRhythmKeyStateChanged);
	}
	Super::NativeDestruct();
}

void UPTBDWKeyGuideWidget::InitializeWithMiniGame(APTBDWMiniGame* InMiniGame)
{
	if (APTBDWMiniGame* Old = DWMiniGame.Get())
	{
		Old->OnRhythmKeyStateChanged.RemoveDynamic(this, &UPTBDWKeyGuideWidget::HandleRhythmKeyStateChanged);
	}

	DWMiniGame = InMiniGame;
	if (InMiniGame)
	{
		InMiniGame->OnRhythmKeyStateChanged.AddUniqueDynamic(this, &UPTBDWKeyGuideWidget::HandleRhythmKeyStateChanged);
	}
	bSlotsDirty = true;
}

void UPTBDWKeyGuideWidget::CollectSlots()
{
	KeySlots.Reset();
	if (!WidgetTree)
	{
		return;
	}
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UPTBDWKeySlotWidget* KeySlot = Cast<UPTBDWKeySlotWidget>(Widget))
		{
			KeySlots.Add(KeySlot);
		}
	});
}

void UPTBDWKeyGuideWidget::RefreshSlots()
{
	APTBDWMiniGame* MiniGame = DWMiniGame.Get();
	for (UPTBDWKeySlotWidget* KeySlot : KeySlots)
	{
		if (!KeySlot)
		{
			continue;
		}
		const bool bUsed = MiniGame ? MiniGame->IsActionUsed(KeySlot->Action) : true;
		KeySlot->SetVisibility(bUsed ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (bUsed)
		{
			const FLinearColor Color = MiniGame ? MiniGame->GetNoteColorForAction(KeySlot->Action) : FLinearColor::White;
			KeySlot->SetupSlot(Color, UPTBKeyTextLibrary::GetActionKeyLabel(this, KeySlot->Action));
		}
		KeySlot->SetPressed(MiniGame ? MiniGame->IsRhythmKeyHeld(KeySlot->Action) : false);
	}
	bSlotsDirty = false;
}

void UPTBDWKeyGuideWidget::HandleRhythmKeyStateChanged(EPTBActionType Action, bool bPressed)
{
	for (UPTBDWKeySlotWidget* KeySlot : KeySlots)
	{
		if (KeySlot && KeySlot->Action == Action)
		{
			KeySlot->SetPressed(bPressed);
		}
	}
}

void UPTBDWKeyGuideWidget::SetGuideShown(bool bShown)
{
	if (bGuideShown == bShown)
	{
		return;
	}
	bGuideShown = bShown;
	// Collapsed/Hidden으로 숨기면 위젯 Tick이 멈춰 다시 보이게 할 수 없음 → 투명도로 숨김 (입력은 항상 통과)
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	SetRenderOpacity(bShown ? 1.0f : 0.0f);
	OnGuideShownChanged(bShown);
}

void UPTBDWKeyGuideWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APTBDWMiniGame* MiniGame = DWMiniGame.Get();
	bool bShouldShow = bAlwaysVisible;
	if (MiniGame && !bShouldShow)
	{
		const bool bTutorialScene = bShowDuringTutorial
			&& (MiniGame->GetScenePhase() == EPTBMiniGameScenePhase::Tutorial || MiniGame->IsTutorialPracticeActive());
		bShouldShow = bTutorialScene || MiniGame->IsRhythmInputActive();
	}

	// 다시 보일 때마다 키 이름·색 갱신 (설정 메뉴에서 키를 바꾸고 돌아온 경우 대비)
	if (bShouldShow && (!bGuideShown || bSlotsDirty))
	{
		RefreshSlots();
	}
	SetGuideShown(bShouldShow);
}
