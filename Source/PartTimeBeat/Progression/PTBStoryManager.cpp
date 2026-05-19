// Fill out your copyright notice in the Description page of Project Settings.

#include "Progression/PTBStoryManager.h"

#include "Debug/PTBLogChannels.h"

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

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] StoryManager initialized with temporary seed data. ChapterCount=%d"),
		StoryChapters.Num());
}

TArray<FName> UPTBStoryManager::CheckUnlockCondition(const FPTBRoundResult& Result)
{
	TArray<FName> NewlyUnlockedChapters;

	if (Result.MiniGameId.IsNone())
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] CheckUnlockCondition received result with empty MiniGameId."));
	}

	// TODO(Progression): 추후 프로필의 누적 별/누적 알바비 기준으로 교체 필요 
	const int32 CurrentStars = FMath::Max(0, Result.StarCount);
	const int32 CurrentMoney = FMath::Max(0, Result.EarnedMoney);

	for (FPTBStoryChapter& Chapter : StoryChapters)
	{
		if (Chapter.ChapterId.IsNone())
		{
			UE_LOG(LogProgression, Warning,
				TEXT("[PTBProgression] Skip story chapter with empty ChapterId."));
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

			if (PendingStoryId.IsNone())
			{
				PendingStoryId = Chapter.ChapterId;
			}

			UE_LOG(LogProgression, Log,
				TEXT("[PTBProgression] Story chapter unlocked: %s"),
				*Chapter.ChapterId.ToString());

			OnChapterUnlocked.ExecuteIfBound(Chapter.ChapterId);
		}
	}

	return NewlyUnlockedChapters;
}

bool UPTBStoryManager::ShouldPlayStoryAfterResult(const FPTBRoundResult& Result, FName& OutId)
{
	OutId = NAME_None;

	const TArray<FName> NewlyUnlockedChapters = CheckUnlockCondition(Result);

	if (!PendingStoryId.IsNone())
	{
		OutId = PendingStoryId;

		UE_LOG(LogProgression, Log,
			TEXT("[PTBProgression] Story pending after result: %s"),
			*OutId.ToString());

		return true;
	}

	if (NewlyUnlockedChapters.Num() > 0)
	{
		OutId = NewlyUnlockedChapters[0];
		PendingStoryId = OutId;

		UE_LOG(LogProgression, Log,
			TEXT("[PTBProgression] Story selected after result: %s"),
			*OutId.ToString());

		return true;
	}

	UE_LOG(LogProgression, Verbose,
		TEXT("[PTBProgression] No story to play after result. MiniGame=%s"),
		*Result.MiniGameId.ToString());

	return false;
}

void UPTBStoryManager::PlayChapter(FName ChapterId)
{
	if (ChapterId.IsNone())
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] PlayChapter failed: ChapterId is None."));
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
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] PlayChapter failed: Unknown ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	if (!StoryFlags.Contains(ChapterId))
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] PlayChapter blocked: Chapter is not unlocked. ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	bIsStoryPlaying = true;
	PendingStoryId = ChapterId;

	// TODO(UI): StoryViewer 위젯과 DialogueDataTable 재생 흐름이 준비되면 여기에 연결
	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] Play story chapter: %s"),
		*ChapterId.ToString());
}

void UPTBStoryManager::SkipChapter()
{
	if (!bIsStoryPlaying)
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] SkipChapter ignored: no story is currently playing."));
		return;
	}

	if (PendingStoryId.IsNone())
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] SkipChapter ignored: PendingStoryId is None."));
		bIsStoryPlaying = false;
		return;
	}

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] Skip story chapter: %s"),
		*PendingStoryId.ToString());

	MarkAsViewed(PendingStoryId);
}

void UPTBStoryManager::MarkAsViewed(FName ChapterId)
{
	if (ChapterId.IsNone())
	{
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] MarkAsViewed failed: ChapterId is None."));
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
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] MarkAsViewed failed: Unknown ChapterId=%s"),
			*ChapterId.ToString());
		return;
	}

	TargetChapter->bIsViewed = true;

	if (PendingStoryId == ChapterId)
	{
		PendingStoryId = NAME_None;
	}

	bIsStoryPlaying = false;

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] Story chapter marked as viewed: %s"),
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

	UE_LOG(LogProgression, Verbose,
		TEXT("[PTBProgression] GetUnviewedChapters: Count=%d"),
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
		UE_LOG(LogProgression, Warning,
			TEXT("[PTBProgression] ResolveEndingByMoney received negative money: %d"),
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

	UE_LOG(LogProgression, Log,
		TEXT("[PTBProgression] Ending resolved by money. TotalMoney=%d Ending=%s"),
		TotalMoney,
		*EndingId.ToString());

	OnEndingTriggered.ExecuteIfBound(EndingId);

	return EndingId;
}
