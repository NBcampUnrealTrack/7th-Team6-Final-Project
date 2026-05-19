// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "PTBStoryManager.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChapterUnlocked,FName, ChapterId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnChapterCompleted,FName, ChapterId);
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEndingTriggered,FName, EndingId);

/**
 * 스토리 챕터의 해금 조건, 시청 상태, 결과 후 스토리 진입 여부 관리
 * 라운드 결과에 따른 신규 스토리 해금 판단
 * 결과 화면 이후 재생할 PendingStoryId 관리
 * 챕터 재생/스킵/시청 완료 상태 관리
 * 누적 알바비 기반 엔딩 분기 결정
 */
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
	bool ShouldPlayStoryAfterResult(const FPTBRoundResult& Result, FName& OutId);
	
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