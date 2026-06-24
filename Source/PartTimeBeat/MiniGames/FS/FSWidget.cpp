// Fill out your copyright notice in the Description page of Project Settings.


#include "FSWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "PartTimeBeat/MiniGames/FS/PTBFSMiniGame.h" // 여기서 실제 헤더를 인클루드
#include "Audio/PTBWwiseRhythmSyncComponent.h"


void UFSWidget::InitializeWidget(APTBFSMiniGame* InGame) { FSMiniGame = InGame; }

void UFSWidget::OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs)
{
	TargetTimeMs = InTargetTimeMs;
	CueLeadTimeMs = InCueLeadTimeMs;
	CurrentAction = Action;
	bIsActive = true;

	// 모든 화살표 숨김
	if (ArrowLeft) ArrowLeft->SetVisibility(ESlateVisibility::Hidden);
	if (ArrowRight) ArrowRight->SetVisibility(ESlateVisibility::Hidden);
	if (ArrowUp) ArrowUp->SetVisibility(ESlateVisibility::Hidden);

	// 액션에 맞는 화살표 표시
	UImage* TargetArrow = (Action == EPTBActionType::ActionA) ? ArrowLeft : 
						  (Action == EPTBActionType::ActionB) ? ArrowRight : ArrowUp;
	if (TargetArrow) TargetArrow->SetVisibility(ESlateVisibility::Visible);
}

void UFSWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bIsActive) return;

	UImage* ActiveArrow = (CurrentAction == EPTBActionType::ActionA) ? ArrowLeft : 
						  (CurrentAction == EPTBActionType::ActionB) ? ArrowRight : ArrowUp;
	if (!ActiveArrow) return;

	float CurrentTime = FSMiniGame ? FSMiniGame->GetUPTBWwiseRhythmSyncComponent()->GetVisualChartTimeMs() : 0.0f;
	float Alpha = FMath::Clamp((TargetTimeMs - CurrentTime) / CueLeadTimeMs, 0.0f, 1.0f);

	// 좌표 계산
	FVector2D StartPos = (CurrentAction == EPTBActionType::ActionA) ? FVector2D(100, 540) :
						 (CurrentAction == EPTBActionType::ActionB) ? FVector2D(1820, 540) : FVector2D(960, 100);
	FVector2D CenterPos = FVector2D(960, 540);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ActiveArrow->Slot))
	{
		CanvasSlot->SetPosition(FMath::Lerp(StartPos, CenterPos, Alpha));
	}

	if (Alpha >= 1.0f) bIsActive = false;
}