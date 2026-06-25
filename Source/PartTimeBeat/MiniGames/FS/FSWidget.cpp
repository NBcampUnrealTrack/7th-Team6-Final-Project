// Fill out your copyright notice in the Description page of Project Settings.


#include "FSWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"
#include "PartTimeBeat/MiniGames/FS/PTBFSMiniGame.h" // 여기서 실제 헤더를 인클루드
#include "Audio/PTBWwiseRhythmSyncComponent.h"


void UFSWidget::InitializeWidget(APTBFSMiniGame* InGame) { FSMiniGame = InGame; }

void UFSWidget::OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs)
{
	UE_LOG(LogTemp, Warning, TEXT("[FS] OnNoteEvent 진입 성공!"));
	TargetTimeMs = InTargetTimeMs;
	CueLeadTimeMs = InCueLeadTimeMs;
	CurrentAction = Action;
	bIsActive = true;

	// 모든 화살표 숨김
	if (ArrowLeft) ArrowLeft->SetVisibility(ESlateVisibility::Hidden);
	if (ArrowRight) ArrowRight->SetVisibility(ESlateVisibility::Hidden);
	if (ArrowUp) ArrowUp->SetVisibility(ESlateVisibility::Hidden);

	// 열거형 기반 매핑: A(Left), B(Right), C(Up)
	UImage* TargetArrow = nullptr;
	if (Action == EPTBActionType::ActionA) TargetArrow = ArrowLeft;
	else if (Action == EPTBActionType::ActionB) TargetArrow = ArrowRight;
	else if (Action == EPTBActionType::ActionC) TargetArrow = ArrowUp;

	if (TargetArrow) TargetArrow->SetVisibility(ESlateVisibility::Visible);
}

void UFSWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bIsActive) return;
    
	float CurrentTime = FSMiniGame ? FSMiniGame->GetUPTBWwiseRhythmSyncComponent()->GetVisualChartTimeMs() : 0.0f;
	float Alpha = FMath::Clamp((TargetTimeMs - CurrentTime) / CueLeadTimeMs, 0.0f, 1.0f);

	UImage* ActiveArrow = nullptr;
	FVector2D StartPos;
	FVector2D EndPos = FVector2D(960, 540); // 중앙
	
	if (CurrentAction == EPTBActionType::ActionA) {
		ActiveArrow = ArrowLeft;
		StartPos = FVector2D(100, 540);
	}
	else if (CurrentAction == EPTBActionType::ActionB) {
		ActiveArrow = ArrowRight;
		StartPos = FVector2D(1820, 540);
	}
	else if (CurrentAction == EPTBActionType::ActionC) {
		ActiveArrow = ArrowUp;
		StartPos = FVector2D(960, 100);
	}

	if (!ActiveArrow) return;
    
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(ActiveArrow->Slot))
	{
		CanvasSlot->SetPosition(FMath::Lerp(EndPos, StartPos, Alpha));
	}
	
	if (Alpha <= 0.0f) {
		bIsActive = false;
		ActiveArrow->SetVisibility(ESlateVisibility::Hidden);
		UE_LOG(LogTemp, Warning, TEXT("[FS] 화살표 중앙 도착! 비활성화함."));
	}
}