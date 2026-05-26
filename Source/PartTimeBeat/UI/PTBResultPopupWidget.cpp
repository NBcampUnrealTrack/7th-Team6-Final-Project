// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBResultPopupWidget.h"

void UPTBResultPopupWidget::InitPopup(
	const FPTBRoundResult& InResult,
	const FPTBRewardSummary& InReward)
{
	RoundResult   = InResult;
	RewardSummary = InReward;
	OnPopupDataSet();
}

FText UPTBResultPopupWidget::GetResultMessage() const
{
	if (const FText* Found = ResultMessageMap.Find(RoundResult.Grade))
		return *Found;

	return (RoundResult.Grade == EPTBGradeType::Fail)
		? FText::FromString(TEXT("Fail..."))
		: FText::FromString(TEXT("Clear!"));
}

FText UPTBResultPopupWidget::GetRankText() const
{
	return GradeToText(RoundResult.Grade);
}

FText UPTBResultPopupWidget::GetMiniGameName() const
{
	if (const FText* Found = MiniGameDisplayNameMap.Find(RoundResult.MiniGameId))
		return *Found;

	return FText::FromName(RoundResult.MiniGameId);
}

FText UPTBResultPopupWidget::GetDifficultyText() const
{
	return DifficultyToText(RoundResult.Difficulty);
}

FText UPTBResultPopupWidget::GetPlayModeText() const
{
	return PlayModeToText(RoundResult.PlayMode);
}

FText UPTBResultPopupWidget::GradeToText(EPTBGradeType Grade)
{
	switch (Grade)
	{
	case EPTBGradeType::PerfectFullCombo: return FText::FromString(TEXT("PFC"));
	case EPTBGradeType::FullCombo:        return FText::FromString(TEXT("FC"));
	case EPTBGradeType::S:                return FText::FromString(TEXT("S"));
	case EPTBGradeType::A:                return FText::FromString(TEXT("A"));
	case EPTBGradeType::B:                return FText::FromString(TEXT("B"));
	case EPTBGradeType::C:                return FText::FromString(TEXT("C"));
	case EPTBGradeType::Clear:            return FText::FromString(TEXT("Clear"));
	case EPTBGradeType::Fail:             return FText::FromString(TEXT("Fail"));
	default:                              return FText::FromString(TEXT("-"));
	}
}

FText UPTBResultPopupWidget::DifficultyToText(EPTBDifficulty Difficulty)
{
	switch (Difficulty)
	{
	case EPTBDifficulty::Easy:     return FText::FromString(TEXT("Easy"));
	case EPTBDifficulty::Standard: return FText::FromString(TEXT("Standard"));
	case EPTBDifficulty::Insane:   return FText::FromString(TEXT("Insane"));
	default:                       return FText::FromString(TEXT("-"));
	}
}

FText UPTBResultPopupWidget::PlayModeToText(EPTBPlayMode PlayMode)
{
	switch (PlayMode)
	{
	case EPTBPlayMode::Single:      return FText::FromString(TEXT("싱글"));
	case EPTBPlayMode::Multiplayer: return FText::FromString(TEXT("멀티"));
	default:                        return FText::FromString(TEXT("-"));
	}
}