// Fill out your copyright notice in the Description page of Project Settings.

#include "Progression/PTBStageUnlockManager.h"

#include "Debug/PTBTeamLog.h"
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

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] StageUnlockManager 초기화 완료. 임시 스테이지 데이터 수=%d"),
		AllStages.Num());
}

bool UPTBStageUnlockManager::IsStageUnlocked(FName StageId) const
{
	if (StageId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] IsStageUnlocked 실패: StageId가 None입니다."));
		return false;
	}

	const FPTBStageInfo* StageInfo = ProgressMap.Find(StageId);
	if (!StageInfo)
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] IsStageUnlocked 실패: 알 수 없는 StageId=%s"),
			*StageId.ToString());
		return false;
	}

	return StageInfo->bIsUnlocked;
}

bool UPTBStageUnlockManager::CanPlayMiniGame(FName GameId) const
{
	if (GameId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] CanPlayMiniGame 실패: GameId가 None입니다."));
		return false;
	}

	for (const TPair<FName, FPTBStageInfo>& Pair : ProgressMap)
	{
		const FPTBStageInfo& Stage = Pair.Value;

		if (Stage.MiniGameId == GameId)
		{
			PTB_RECORD(LogPTBProgression,
				TEXT("[PTBProgression] 미니게임 플레이 가능 여부 확인: GameId=%s 해금여부=%d"),
				*GameId.ToString(),
				Stage.bIsUnlocked);

			return Stage.bIsUnlocked;
		}
	}

	PTB_WARNING(LogPTBProgression,
		TEXT("[PTBProgression] CanPlayMiniGame 실패: 알 수 없는 GameId=%s"),
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

	PTB_VERBOSE(LogPTBProgression,
		TEXT("[PTBProgression] 해금된 스테이지 수=%d"),
		UnlockedStages.Num());

	return UnlockedStages;
}

FPTBRewardSummary UPTBStageUnlockManager::ApplyRoundResult(const FPTBRoundResult& Result)
{
	FPTBRewardSummary Summary;
	Summary.EarnedMoney = Result.EarnedMoney;
	Summary.EarnedStars = Result.StarCount;
	Summary.TotalMoney = 0; // TODO(Save/Profile): 누적 금액을 추적할 수 있게 되면 실제 총액으로 교체
	Summary.TotalStars = TotalStars;

	if (Result.MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] ApplyRoundResult 실패: MiniGameId가 None입니다."));
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
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] ApplyRoundResult 실패: 알 수 없는 MiniGameId=%s"),
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

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 라운드 결과 반영: 미니게임=%s 점수=%d->%d 별=%d->%d 등급=%d->%d 총별=%d"),
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
		PTB_RECORD(LogPTBProgression,
			TEXT("[PTBProgression] 스테이지 해금: %s"), *StageId.ToString());
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
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] CheckPrerequisites 실패: StageId가 None입니다."));
		return false;
	}

	if (!ProgressMap.Contains(StageId))
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] CheckPrerequisites 실패: 알 수 없는 StageId=%s"),
			*StageId.ToString());
		return false;
	}

	// TODO(Progression): FPTBStageInfo에 PrerequisiteStageIds가 추가되면 
	// 선행 스테이지 클리어 여부 검사.
	return true;
}
