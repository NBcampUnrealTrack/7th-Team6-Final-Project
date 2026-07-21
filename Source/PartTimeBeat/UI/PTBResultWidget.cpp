// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBResultWidget.h"

#include "Flow/PTBGameFlowSubsystem.h"

void UPTBResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* PC = GetOwningPlayer())
	{
		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	if (bAutoShowFlowResultOnConstruct)
	{
		ShowResultFromFlowSubsystem();
	}
}

void UPTBResultWidget::NativeDestruct()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(PopupTimerHandle);
	}
	Super::NativeDestruct();
}

bool UPTBResultWidget::ShowResultFromFlowSubsystem()
{
	UGameInstance* GameInstance = GetGameInstance();
	UPTBGameFlowSubsystem* FlowSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPTBGameFlowSubsystem>()
		: nullptr;

	if (!FlowSubsystem || !FlowSubsystem->HasRoundResult())
	{
		return false;
	}

	ShowResult(FlowSubsystem->GetLastRoundResult(), FlowSubsystem->GetLastRewardSummary());
	return true;
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
