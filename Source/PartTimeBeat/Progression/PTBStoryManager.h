// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "PTBStoryManager.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChapterUnlocked,FName, ChapterId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChapterCompleted,FName, ChapterId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEndingTriggered,FName, EndingId);
UCLASS()
class PARTTIMEBEAT_API UPTBStoryManager : public UObject
{
	GENERATED_BODY()
public:
	UPTBStoryManager();
	
	
	/**챕터 재생 시작*/
	UFUNCTION()
	void PlayChapter(FName ChapterId);
	
	/**현재 챕터 스킵*/
	UFUNCTION()
	void SkipChapter();
	
	/**시청 마킹*/
	UFUNCTION()
	void MarkAsViewed(FName ChapterId);
	
	/** 미시청 챕터*/
	UFUNCTION()
	TArray<FPTBStoryChapter> GetUnviewedChapters()const;
	
	/**새스토리 존재*/
	UFUNCTION()
	bool HasNewStory()const;
	
	//델리 게이트용
	/** 챕터해금시*/
	UPROPERTY()
	FOnChapterUnlocked OnChapterUnlocked;
	
	/** 챕터완료시*/
	UPROPERTY()
	FOnChapterCompleted OnChapterCompleted;
	
	/** 엔딩시점*/
	UPROPERTY()
	FOnEndingTriggered OnEndingTriggered;
	
protected:
	/** 새로해금된 챕터*/
	UFUNCTION()
	TArray<FName>CheckUnlockCondition(const FPTBRoundResult& Result);
		
	/**결과직후 재생여부 */
	UFUNCTION()
	bool ShouldPlayStoryAfterResult(const FPTBRoundResult& Result,FName OutId);
	
	/**알바비에 따른 엔딩*/
	UFUNCTION()
	FName ResolveEndingByMoney(int32 TotalMoney)const;
private:
	/** 전체 챕터*/
	UPROPERTY()
	TArray<FPTBStoryChapter> StoryChapters;

	/**진행 플래그*/
	UPROPERTY()
	TSet<FName> StoryFlags;

	/**결과후 재생할 챕터*/
	UPROPERTY()
	FName PendingStoryId;

	/**재생중*/
	UPROPERTY()
	bool bIsStoryPlaying;
};