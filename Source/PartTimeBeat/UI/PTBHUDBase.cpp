// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBHUDBase.h"

#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UPTBHUDBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTBHUDBase::ShowHud()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UPTBHUDBase::HideHud()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UPTBHUDBase::UpdateScore(int32 NewScore)
{
	if (ScoreText)
	{
	}
}

void UPTBHUDBase::UpdateCombo(int32 NewCombo)
{
	if (ComboText)
	{
	}
}

void UPTBHUDBase::ShowJudgement(const FPTBJudgementResult& Result)
{
	if (JudgementText)
	{
	}
}

void UPTBHUDBase::UpdateProgress(float ProgressRatio)
{
	if (ProgressBar)
	{
	}
}