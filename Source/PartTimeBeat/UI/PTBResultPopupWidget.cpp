// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBResultPopupWidget.h"
#include "Progression/PTBProgressLibrary.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Flow/PTBGameFlowSubsystem.h"

void UPTBResultPopupWidget::InitPopup(
	const FPTBRoundResult& InResult,
	const FPTBRewardSummary& InReward)
{
	RoundResult   = InResult;
	RewardSummary = InReward;
	RefreshTexts();
	OnPopupDataSet();
}

void UPTBResultPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bHandleButtonsInCode)
	{
		if (Btn_Retry) Btn_Retry->OnClicked.AddUniqueDynamic(this, &UPTBResultPopupWidget::HandleRetryButtonClicked);
		if (Btn_Map)   Btn_Map->OnClicked.AddUniqueDynamic(this, &UPTBResultPopupWidget::HandleMapButtonClicked);
	}
}

void UPTBResultPopupWidget::RefreshTexts()
{
	FNumberFormattingOptions Grouped;
	Grouped.SetUseGrouping(true);

	if (Text_MiniGameName)  Text_MiniGameName->SetText(GetMiniGameName());
	if (Text_ResultMessage) Text_ResultMessage->SetText(GetResultMessage());

	if (Text_PlayMode)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("PlayMode"), GetPlayModeText());
		Text_PlayMode->SetText(FText::Format(PlayModeFormat, Args));
	}
	if (Text_Difficulty)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Difficulty"), GetDifficultyText());
		Text_Difficulty->SetText(FText::Format(DifficultyFormat, Args));
	}
	if (Text_Rank)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Rank"), GetRankText());
		Text_Rank->SetText(FText::Format(RankFormat, Args));
	}
	if (Text_Score)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Score"), FText::AsNumber(RoundResult.Score, &Grouped));
		Args.Add(TEXT("Percent"), GetScorePercentText());
		Text_Score->SetText(FText::Format(ScoreFormat, Args));
	}
	if (Text_EarnedMoney)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("EarnedMoney"), FText::AsNumber(RewardSummary.EarnedMoney, &Grouped));
		Text_EarnedMoney->SetText(FText::Format(EarnedMoneyFormat, Args));
	}
	if (Text_TotalMoney)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("TotalMoney"), FText::AsNumber(RewardSummary.TotalMoney, &Grouped));
		Text_TotalMoney->SetText(FText::Format(TotalMoneyFormat, Args));
	}
}

void UPTBResultPopupWidget::HandleRetryButtonClicked()
{
	OnRetryClicked.Broadcast();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBGameFlowSubsystem* Flow = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			Flow->RetryLastGame();
		}
	}
}

void UPTBResultPopupWidget::HandleMapButtonClicked()
{
	OnMapClicked.Broadcast();
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBGameFlowSubsystem* Flow = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			Flow->ReturnToMiniGameSelect();
		}
	}
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

FText UPTBResultPopupWidget::GetScorePercentText() const
{
	return UPTBProgressLibrary::ScoreToPercentText(RoundResult.Score);
}

FText UPTBResultPopupWidget::GetScoreWithPercentText() const
{
	return UPTBProgressLibrary::ScoreWithPercentText(RoundResult.Score);
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
	if (const UEnum* DifficultyEnum = StaticEnum<EPTBDifficulty>())
	{
		const FText DisplayName = DifficultyEnum->GetDisplayNameTextByValue(static_cast<int64>(Difficulty));
		if (!DisplayName.IsEmpty())
			return DisplayName;
	}

	return FText::FromString(TEXT("-"));
}

FText UPTBResultPopupWidget::PlayModeToText(EPTBPlayMode PlayMode)
{
	if (const UEnum* PlayModeEnum = StaticEnum<EPTBPlayMode>())
	{
		const FText DisplayName = PlayModeEnum->GetDisplayNameTextByValue(static_cast<int64>(PlayMode));
		if (!DisplayName.IsEmpty())
			return DisplayName;
	}

	return FText::FromString(TEXT("-"));
}