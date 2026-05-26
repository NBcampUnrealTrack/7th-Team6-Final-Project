// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBResultWidget.h"

void UPTBResultWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PopupTimerHandle);
	}
	Super::NativeDestruct();
}

void UPTBResultWidget::ShowResult(
	const FPTBRoundResult& InResult,
	const FPTBRewardSummary& InReward)
{
	CurrentRoundResult   = InResult;
	CurrentRewardSummary = InReward;

	OnDisplayTypeChanged(InResult.Grade);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PopupTimerHandle,
			this,
			&UPTBResultWidget::FirePopupTimer,
			PopupDelay,
			false
		);
	}
}

void UPTBResultWidget::FirePopupTimer()
{
	OnPopupTimerFired();
}