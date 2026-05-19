// Fill out your copyright notice in the Description page of Project Settings.

#include "Progression/PTBStageUnlockManager.h"

#include "Debug/PTBLogChannels.h"
#include "Core/PTBStructEnums.h"

namespace
{
	FPTBStageInfo MakeTemporaryStage(
		FName StageId,
		FName MiniGameId,
		const FString& DisplayName,
		int32 RequiredStars,
		bool bIsUnlocked)
	{
		FPTBStageInfo Stage;
		Stage.StageId = StageId;
		Stage.MiniGameId = MiniGameId;
		Stage.DisplayName = FText::FromString(DisplayName);
		Stage.RequiredStars = RequiredStars;
		Stage.BestStarRating = 0;
		Stage.BestScore = 0;
		Stage.BestGrade = EPTBGradeType::Fail;
		Stage.bIsUnlocked = bIsUnlocked;
		return Stage;
	}
}

UPTBStageUnlockManager::UPTBStageUnlockManager() 
 : TotalStars(0)
{
	// TODO(Data): 스테이지 데이터가 완성되면
	// DT_PTBStageInfo 또는 StageInfo DataAsset 기반 로딩으로 교체
	// 진행도 흐름을 테스트하기 위한 임시 시드 데이터를 넣어 놓음

	AllStages.Add(MakeTemporaryStage(TEXT("Stage_JJ"), TEXT("JJ"), TEXT("Jump Jump"), 0, true));
	AllStages.Add(MakeTemporaryStage(TEXT("Stage_WD"), TEXT("WD"), TEXT("Waddle Waddle"), 1, false));
	AllStages.Add(MakeTemporaryStage(TEXT("Stage_CM"), TEXT("CM"), TEXT("Copycat Animal"), 3, false));

	for (const FPTBStageInfo& Stage : AllStages)
	{
		ProgressMap.Add(Stage.StageId, Stage);
	}

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] StageUnlockManager initialized with temporary seed data. StageCount=%d"),
		AllStages.Num());
}

bool UPTBStageUnlockManager::IsStageUnlocked(FName StageId) const
{
	if (StageId.IsNone())
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] IsStageUnlocked failed: StageId is None."));
		return false;
	}

	const FPTBStageInfo* StageInfo = ProgressMap.Find(StageId);
	if (!StageInfo)
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] IsStageUnlocked failed: Unknown StageId=%s"),
			*StageId.ToString());
		return false;
	}

	return StageInfo->bIsUnlocked;
}

bool UPTBStageUnlockManager::CanPlayMiniGame(FName GameId) const
{
	if (GameId.IsNone())
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] CanPlayMiniGame failed: GameId is None."));
		return false;
	}

	for (const TPair<FName, FPTBStageInfo>& Pair : ProgressMap)
	{
		const FPTBStageInfo& Stage = Pair.Value;

		if (Stage.MiniGameId == GameId)
		{
			UE_LOG(LogProgression, Log, 
				TEXT("[PTBProgression] CanPlayMiniGame: GameId=%s bUnlocked=%d"),
				*GameId.ToString(),
				Stage.bIsUnlocked);

			return Stage.bIsUnlocked;
		}
	}

	UE_LOG(LogProgression, Warning, 
		TEXT("[PTBProgression] CanPlayMiniGame failed: Unknown GameId=%s"),
		*GameId.ToString());

	return false;
}

TArray<FPTBStageInfo> UPTBStageUnlockManager::GetUnlockedStages() const
{
	TArray<FPTBStageInfo> UnlockedStages;

	for (const TPair<FName, FPTBStageInfo>& Pair : ProgressMap)
	{
		const FPTBStageInfo& Stage = Pair.Value;

		if (Stage.bIsUnlocked)
		{
			UnlockedStages.Add(Stage);
		}
	}

	UE_LOG(LogProgression, Verbose, 
		TEXT("[PTBProgression] GetUnlockedStages: Count=%d"),
		UnlockedStages.Num());

	return UnlockedStages;
}

FPTBRewardSummary UPTBStageUnlockManager::ApplyRoundResult(const FPTBRoundResult& Result)
{
	FPTBRewardSummary Summary;
	Summary.EarnedMoney = Result.EarnedMoney;
	Summary.EarnedStars = Result.StarCount;
	Summary.TotalMoney = Result.EarnedMoney; // TODO(Save/Profile): 실제 총액으로 교체 필요
	Summary.TotalStars = TotalStars;

	if (Result.MiniGameId.IsNone())
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] ApplyRoundResult failed: MiniGameId is None."));
		return Summary;
	}

	FPTBStageInfo* TargetStage = nullptr;

	for (TPair<FName, FPTBStageInfo>& Pair : ProgressMap)
	{
		if (Pair.Value.MiniGameId == Result.MiniGameId)
		{
			TargetStage = &Pair.Value;
			break;
		}
	}

	if (!TargetStage)
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] ApplyRoundResult failed: Unknown MiniGameId=%s"),
			*Result.MiniGameId.ToString());
		return Summary;
	}

	const int32 PreviousStars = TargetStage->BestStarRating;
	const int32 PreviousScore = TargetStage->BestScore;
	const EPTBGradeType PreviousGrade = TargetStage->BestGrade;

	TargetStage->BestScore = FMath::Max(TargetStage->BestScore, Result.Score);
	TargetStage->BestStarRating = FMath::Max(TargetStage->BestStarRating, 
		Result.StarCount);

	if (Result.Score >= PreviousScore)
	{
		TargetStage->BestGrade = Result.Grade;
	}

	const int32 AddedStars = FMath::Max(0, TargetStage->BestStarRating - PreviousStars);
	TotalStars += AddedStars;

	Summary.EarnedStars = AddedStars;
	Summary.TotalStars = TotalStars;

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] ApplyRoundResult: MiniGame=%s Score=%d->%d Stars=%d->%d Grade=%d->%d TotalStars=%d"),
		*Result.MiniGameId.ToString(),
		PreviousScore,
		TargetStage->BestScore,
		PreviousStars,
		TargetStage->BestStarRating,
		static_cast<uint8>(PreviousGrade),
		static_cast<uint8>(TargetStage->BestGrade),
		TotalStars);

	const TArray<FName> NewlyUnlockedStages = CheckNewlyUnlockedStages();

	for (const FName& StageId : NewlyUnlockedStages)
	{
		UE_LOG(LogProgression, Log, 
			TEXT("[PTBProgression] Stage unlocked: %s"), *StageId.ToString());
		OnStageUnlocked.Broadcast(StageId);
	}

	return Summary;
}

TArray<FName> UPTBStageUnlockManager::CheckNewlyUnlockedStages()
{
	TArray<FName> NewlyUnlockedStages;

	for (TPair<FName, FPTBStageInfo>& Pair : ProgressMap)
	{
		FPTBStageInfo& Stage = Pair.Value;

		if (Stage.bIsUnlocked)
		{
			continue;
		}

		const bool bMeetsStarRequirement = TotalStars >= Stage.RequiredStars;
		const bool bMeetsPrerequisites = CheckPrerequisites(Stage.StageId);

		if (bMeetsStarRequirement && bMeetsPrerequisites)
		{
			Stage.bIsUnlocked = true;
			NewlyUnlockedStages.Add(Stage.StageId);
		}
	}

	return NewlyUnlockedStages;
}

int32 UPTBStageUnlockManager::GetTotalStars() const
{
	return TotalStars;
}

bool UPTBStageUnlockManager::CheckPrerequisites(FName StageId) const
{
	if (StageId.IsNone())
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] CheckPrerequisites failed: StageId is None."));
		return false;
	}

	if (!ProgressMap.Contains(StageId))
	{
		UE_LOG(LogProgression, Warning, 
			TEXT("[PTBProgression] CheckPrerequisites failed: Unknown StageId=%s"),
			*StageId.ToString());
		return false;
	}

	// TODO(Progression): FPTBStageInfo에 PrerequisiteStageIds가 추가되면 
	// 선행 스테이지 클리어 여부 검사.
	return true;
}