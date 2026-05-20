// Fill out your copyright notice in the Description page of Project Settings.

#include "Progression/PTBStoryManager.h"

#include "Debug/PTBTeamLog.h"

namespace
{
	FPTBStoryChapter MakeTemporaryStoryChapter(
		FName ChapterId,
		const FString& Title,
		int32 UnlockConditionStars,
		int32 RequiredMoney)
	{
		FPTBStoryChapter Chapter;
		Chapter.ChapterId = ChapterId;
		Chapter.Title = FText::FromString(Title);
		Chapter.UnlockConditionStars = UnlockConditionStars;
		Chapter.RequiredMoney = RequiredMoney;
		Chapter.DialogueDataTable = nullptr;
		Chapter.bIsViewed = false;
		return Chapter;
	}
}

UPTBStoryManager::UPTBStoryManager() 
 : PendingStoryId(NAME_None)
 , bIsStoryPlaying(false)
{
	// TODO(Data): 스토리 데이터가 완성되면
	// DT_PTBStoryCondition 또는 StoryChapter DataAsset 기반 로딩으로 교체
	// 스토리 해금 흐름을 테스트하기 위한 임시 시드 데이터를 넣어 놓음

	StoryChapters.Add(MakeTemporaryStoryChapter(TEXT("Story_Intro"), TEXT("First Day"), 0, 0));
	StoryChapters.Add(MakeTemporaryStoryChapter(TEXT("Story_After_JJ"), TEXT("After Jump Jump"), 1, 0));
	StoryChapters.Add(MakeTemporaryStoryChapter(TEXT("Story_First_Payday"), TEXT("First Payday"), 3, 1000));

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] StoryManager 초기화 완료. 임시 스토리 챕터 수=%d"),
		StoryChapters.Num());
}

TArray<FName> UPTBStoryManager::CheckUnlockCondition(const FPTBRoundResult& Result)
{
	TArray<FName> NewlyUnlockedChapters;

	if (Result.MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] CheckUnlockCondition: MiniGameId가 비어 있는 결과를 받았습니다."));
	}

	// TODO(Progression): 추후 프로필의 누적 별/누적 알바비 기준으로 교체 필요 
	const int32 CurrentStars = FMath::Max(0, Result.StarCount);
	const int32 CurrentMoney = FMath::Max(0, Result.EarnedMoney);

	for (FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (Chapter.ChapterId.IsNone())
		{
			PTB_WARNING(LogPTBProgression,
				TEXT("[PTBProgression] ChapterId가 비어 있는 스토리 챕터를 건너뜁니다."));
			continue;
		}

		if (StoryFlags.Contains(Chapter.ChapterId))
		{
			continue;
		}

		const bool bMeetsStarCondition = CurrentStars >= Chapter.UnlockConditionStars;
		const bool bMeetsMoneyCondition = CurrentMoney >= Chapter.RequiredMoney;

		if (bMeetsStarCondition && bMeetsMoneyCondition)
		{
			StoryFlags.Add(Chapter.ChapterId);
			NewlyUnlockedChapters.Add(Chapter.ChapterId);

			PTB_RECORD(LogPTBProgression,
				TEXT("[PTBProgression] 스토리 챕터 해금: %s"),
				*Chapter.ChapterId.ToString());

			OnChapterUnlocked.ExecuteIfBound(Chapter.ChapterId);
		}
	}

	return NewlyUnlockedChapters;
}

bool UPTBStoryManager::ShouldPlayStoryAfterResult(const FPTBRoundResult& Result, FName& OutId)
{
	OutId = NAME_None;

	if (!PendingStoryId.IsNone())
	{
		OutId = PendingStoryId;

		PTB_RECORD(LogPTBProgression,
			TEXT("[PTBProgression] 결과 이후 재생 대기 중인 스토리: %s"),
			*OutId.ToString());

		return true;
	}
	
	const TArray<FName> NewlyUnlockedChapters = CheckUnlockCondition(Result);

	if (NewlyUnlockedChapters.Num() <= 0)
	{
		PTB_VERBOSE(LogPTBProgression,
			TEXT("[PTBProgression] 결과 이후 재생할 스토리가 없습니다. 미니게임=%s"),
			*Result.MiniGameId.ToString());

		return false;
	}

	PendingStoryId = NewlyUnlockedChapters[0];
	OutId = PendingStoryId;

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 결과 이후 재생할 스토리 선택: %s"),
		*OutId.ToString());

	return true;
}

void UPTBStoryManager::PlayChapter(FName ChapterId)
{
	if (ChapterId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] PlayChapter 실패: ChapterId가 None입니다."));
		return;
	}

	FPTBStoryChapter* TargetChapter = nullptr;

	for (FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (Chapter.ChapterId == ChapterId)
		{
			TargetChapter = &Chapter;
			break;
		}
	}

	if (!TargetChapter)
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] PlayChapter 실패: 알 수 없는 ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	if (!StoryFlags.Contains(ChapterId))
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] PlayChapter 차단: 해금되지 않은 챕터입니다. ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	bIsStoryPlaying = true;
	PendingStoryId = ChapterId;

	// TODO(UI): StoryViewer 위젯과 DialogueDataTable 재생 흐름이 준비되면 여기에 연결
	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 스토리 챕터 재생: %s"),
		*ChapterId.ToString());
}

void UPTBStoryManager::SkipChapter()
{
	if (!bIsStoryPlaying)
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] SkipChapter 무시: 현재 재생 중인 스토리가 없습니다."));
		return;
	}

	if (PendingStoryId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] SkipChapter 무시: PendingStoryId가 None입니다."));
		bIsStoryPlaying = false;
		return;
	}

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 스토리 챕터 건너뛰기: %s"),
		*PendingStoryId.ToString());

	MarkAsViewed(PendingStoryId);
}

void UPTBStoryManager::MarkAsViewed(FName ChapterId)
{
	if (ChapterId.IsNone())
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] MarkAsViewed 실패: ChapterId가 None입니다."));
		return;
	}

	FPTBStoryChapter* TargetChapter = nullptr;

	for (FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (Chapter.ChapterId == ChapterId)
		{
			TargetChapter = &Chapter;
			break;
		}
	}

	if (!TargetChapter)
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] MarkAsViewed 실패: 알 수 없는 ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	TargetChapter->bIsViewed = true;

	if (PendingStoryId == ChapterId)
	{
		PendingStoryId = NAME_None;
	}

	bIsStoryPlaying = false;

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 스토리 챕터 시청 완료 처리: %s"),
		*ChapterId.ToString());

	OnChapterCompleted.ExecuteIfBound(ChapterId);
}

TArray<FPTBStoryChapter> UPTBStoryManager::GetUnviewedChapters() const
{
	TArray<FPTBStoryChapter> UnviewedChapters;

	for (const FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (Chapter.ChapterId.IsNone())
		{
			continue;
		}

		if (StoryFlags.Contains(Chapter.ChapterId) && !Chapter.bIsViewed)
		{
			UnviewedChapters.Add(Chapter);
		}
	}

	PTB_VERBOSE(LogPTBProgression,
		TEXT("[PTBProgression] 미시청 스토리 챕터 수=%d"),
		UnviewedChapters.Num());

	return UnviewedChapters;
}

bool UPTBStoryManager::HasNewStory() const
{
	if (!PendingStoryId.IsNone())
	{
		return true;
	}

	for (const FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (StoryFlags.Contains(Chapter.ChapterId) && !Chapter.bIsViewed)
		{
			return true;
		}
	}

	return false;
}

FName UPTBStoryManager::ResolveEndingByMoney(int32 TotalMoney) const
{
	// TODO(Data): 추후 엔딩 분기 조건이 늘어나거나 수정되게 된다면
	// 조건/결과를 별도의 private 함수 또는 데이터테이블로 분리 권장
	// 현재는 플레이어가 소지한 알바비 총액에 따라 3가지 엔딩 분기로 나뉘도록 되어 있음
	if (TotalMoney < 0)
	{
		PTB_WARNING(LogPTBProgression,
			TEXT("[PTBProgression] ResolveEndingByMoney: 음수 금액을 받았습니다. 금액=%d"),
			TotalMoney);

		OnEndingTriggered.ExecuteIfBound(TEXT("Ending_Bad"));
		return TEXT("Ending_Bad");
	}

	const auto ResolveEndingId = [](int32 InTotalMoney) -> FName
	{
		if (InTotalMoney >= 10000)
		{
			return TEXT("Ending_Good");
		}

		if (InTotalMoney >= 5000)
		{
			return TEXT("Ending_Normal");
		}

		return TEXT("Ending_Bad");
	};

	const FName EndingId = ResolveEndingId(TotalMoney);

	PTB_RECORD(LogPTBProgression,
		TEXT("[PTBProgression] 금액 기준 엔딩 결정. 총금액=%d 엔딩=%s"),
		TotalMoney,
		*EndingId.ToString());

	OnEndingTriggered.ExecuteIfBound(EndingId);

	return EndingId;
}
