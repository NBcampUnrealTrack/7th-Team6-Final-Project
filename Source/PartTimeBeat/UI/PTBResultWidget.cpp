// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPTBResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonRetry)
	{
	}

	if (ButtonNext)
	{
	}
}

void UPTBResultWidget::SetRoundResult(const FPTBRoundResult& RoundResult)
{
	CurrentRoundResult = RoundResult;

	if (RankText)
	{
	}

	if (ScoreText)
	{
	}
}

void UPTBResultWidget::ShowRewardSummary(const FPTBRewardSummary& RewardSummary)
{
	if (RewardPanel)
	{
		RewardPanel->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPTBResultWidget::OnRetryClicked()
{
	OnRetryRequested.Broadcast();
}

void UPTBResultWidget::OnNextClicked()
{
	OnNextRequested.Broadcast();
}