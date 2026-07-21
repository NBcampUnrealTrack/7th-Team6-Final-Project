// Fill out your copyright notice in the Description page of Project Settings.


#include "FSWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "PartTimeBeat/MiniGames/FS/PTBFSMiniGame.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"


void UFSWidget::InitializeWidget(APTBFSMiniGame* InGame) { FSMiniGame = InGame; }

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
		if (ArrowRight) ArrowRight->SetVisibility(ESlateVisibility::Visible);
	}
	else if (Action == EPTBActionType::ActionC) {
		ArrowStateC.TargetTimeMs = InTargetTimeMs;
		ArrowStateC.CueLeadTimeMs = InCueLeadTimeMs;
		ArrowStateC.bIsActive = true;
		if (ArrowUp) ArrowUp->SetVisibility(ESlateVisibility::Visible);
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

	UpdateArrow(ArrowStateA, ArrowLeft,  FVector2D(100, 540),  FVector2D(760, 540));
	UpdateArrow(ArrowStateB, ArrowRight, FVector2D(1820, 540), FVector2D(1160, 540));
	UpdateArrow(ArrowStateC, ArrowUp,    FVector2D(960, 100),  FVector2D(960, 540));
}		