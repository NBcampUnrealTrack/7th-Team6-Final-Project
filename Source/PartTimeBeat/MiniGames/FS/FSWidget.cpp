// Fill out your copyright notice in the Description page of Project Settings.


#include "FSWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "UI/PTBKeyTextLibrary.h"
#include "PartTimeBeat/MiniGames/FS/PTBFSMiniGame.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"


void UFSWidget::InitializeWidget(APTBFSMiniGame* InGame)
{
	FSMiniGame = InGame;
	RefreshKeyGuideLabels();
	UpdateKeyGuide(0.0f);
}

void UFSWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshKeyGuideLabels();
}

void UFSWidget::RefreshKeyGuideLabels()
{
	if (KeyGuideTextA) { KeyGuideTextA->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionA)); }
	if (KeyGuideTextB) { KeyGuideTextB->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionB)); }
	if (KeyGuideTextC) { KeyGuideTextC->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionC)); }
}

void UFSWidget::UpdateKeyGuide(float VisualTimeMs)
{
	if (KeyGuideLanePositions.Num() < 3)
	{
		return;
	}

	UWidget* Guides[3] = { KeyGuideA, KeyGuideB, KeyGuideC };
	bool bSwapping = false;
	for (int32 Key = 0; Key < 3; ++Key)
	{
		UCanvasPanelSlot* GuideSlot = Guides[Key] ? Cast<UCanvasPanelSlot>(Guides[Key]->Slot) : nullptr;
		if (!GuideSlot)
		{
			continue;
		}

		int32 FromLane = Key;
		int32 ToLane = Key;
		float Alpha = 1.0f;
		if (FSMiniGame)
		{
			FSMiniGame->GetKeyGuideMotion(Key, VisualTimeMs, FromLane, ToLane, Alpha);
		}

		FVector2D Position = KeyGuideLanePositions[ToLane];
		if (FromLane != ToLane && Alpha < 1.0f)
		{
			bSwapping = true;
			// 빠르게 출발해서 부드럽게 멈추는 곡선
			const float Eased = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
			Position = FMath::Lerp(KeyGuideLanePositions[FromLane], KeyGuideLanePositions[ToLane], Eased);
			const float Direction = ToLane > FromLane ? -1.0f : 1.0f;
			Position.Y += Direction * KeyGuideSwapArcHeight * FMath::Sin(PI * Eased);
		}
		GuideSlot->SetPosition(Position);
	}

	if (bSwapping && !bKeyGuideWasSwapping)
	{
		OnKeyGuideSwapStarted();
	}
	bKeyGuideWasSwapping = bSwapping;
}

void UFSWidget::OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs)
{
	if (Action == EPTBActionType::ActionA) {
		ArrowStateA.TargetTimeMs = InTargetTimeMs;
		ArrowStateA.CueLeadTimeMs = InCueLeadTimeMs;
		ArrowStateA.bIsActive = true;
		if (ArrowLeft) ArrowLeft->SetVisibility(ESlateVisibility::Visible);
	}
	else if (Action == EPTBActionType::ActionB) {
		ArrowStateB.TargetTimeMs = InTargetTimeMs;
		ArrowStateB.CueLeadTimeMs = InCueLeadTimeMs;
		ArrowStateB.bIsActive = true;
		if (ArrowUp) ArrowUp->SetVisibility(ESlateVisibility::Visible);
	}
	else if (Action == EPTBActionType::ActionC) {
		ArrowStateC.TargetTimeMs = InTargetTimeMs;
		ArrowStateC.CueLeadTimeMs = InCueLeadTimeMs;
		ArrowStateC.bIsActive = true;
		if (ArrowRight) ArrowRight->SetVisibility(ESlateVisibility::Visible);
	}
}

void UFSWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!FSMiniGame) return;
	UPTBWwiseRhythmSyncComponent* Sync = FSMiniGame->GetUPTBWwiseRhythmSyncComponent();
	if (!Sync) return;
	float CurrentTime = Sync->GetVisualChartTimeMs();

	auto UpdateArrow = [&](FFSArrowState& State, UImage* Arrow, FVector2D StartPos, FVector2D EndPos)
	{
		if (!State.bIsActive || !Arrow) return;
		float Alpha = FMath::Clamp((State.TargetTimeMs - CurrentTime) / State.CueLeadTimeMs, 0.0f, 1.0f);
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Arrow->Slot))
			Slot->SetPosition(FMath::Lerp(EndPos, StartPos, Alpha));
		if (Alpha <= 0.0f) {
			State.bIsActive = false;
			Arrow->SetVisibility(ESlateVisibility::Hidden);
		}
	};

	UpdateKeyGuide(CurrentTime);

	UpdateArrow(ArrowStateA, ArrowLeft,  FVector2D(100, 540),  FVector2D(760, 540));
	// B = 가운데(위에서 내려옴), C = 오른쪽 (키 배치 A/B/C = 왼/가운데/오른쪽에 맞춤)
	UpdateArrow(ArrowStateB, ArrowUp,    FVector2D(960, 100),  FVector2D(960, 540));
	UpdateArrow(ArrowStateC, ArrowRight, FVector2D(1820, 540), FVector2D(1160, 540));
}		