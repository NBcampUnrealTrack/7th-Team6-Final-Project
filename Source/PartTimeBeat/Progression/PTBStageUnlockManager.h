// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "PTBStageUnlockManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStageUnlocked, FName, StageId);

/**
 * 미니게임/스테이지 해금 상태와 별점 기반 진행도 관리
 * 미니게임 플레이 가능 여부 확인
 * 라운드 결과를 반영해 최고 점수/최고 별점 갱신
 * 누적 별점 계산
 * 신규 스테이지 해금 판단 및 OnStageUnlocked 브로드캐스트
 */
UCLASS()
class PARTTIMEBEAT_API UPTBStageUnlockManager : public UObject
{
	GENERATED_BODY()

public:
	UPTBStageUnlockManager();
	/** 미니게임 플레이 가능 여부 */
	UFUNCTION()
	bool CanPlayMiniGame(FName GameId) const;
	
	/** 접근 가능 목록 */
	UFUNCTION()
	TArray<FPTBStageInfo>GetUnlockedStages() const;

	/** 결과 반영 -> 보상 */
	UFUNCTION()
	FPTBRewardSummary ApplyRoundResult(const FPTBRoundResult& Result);
	
	/**누적별  */
	UFUNCTION()
	int32 GetTotalStars() const;
	
	//델리게이트용
	/**스테이지 해금시*/
	UPROPERTY()
	FOnStageUnlocked OnStageUnlocked;
	
protected:
	/** 접근 가능 여부 */
	UFUNCTION()
	bool IsStageUnlocked(FName StageId) const;
	
	/**신규 해금 ID 반환 */
	UFUNCTION()
	TArray<FName> CheckNewlyUnlockedStages();
	
	/** 선행조건  */
	UFUNCTION()
	bool CheckPrerequisites(FName StageId) const;
	
private:
	/** 전체 스테이지 정의*/
	UPROPERTY()
	TArray<FPTBStageInfo> AllStages;
	
	/** 프로필별 진행도*/
	UPROPERTY()
	TMap<FName, FPTBStageInfo> ProgressMap;
	
	/** 누적별 */
	UPROPERTY()
	int32 TotalStars;	
};
