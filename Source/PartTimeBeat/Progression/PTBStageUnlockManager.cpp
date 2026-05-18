// Fill out your copyright notice in the Description page of Project Settings.


#include "Progression/PTBStageUnlockManager.h"

#include "Core/PTBStructEnums.h"

bool UPTBStageUnlockManager::IsStageUnlocked(FName StageId) const
{
	return false;
}

UPTBStageUnlockManager::UPTBStageUnlockManager()
{
}

bool UPTBStageUnlockManager::CanPlayMiniGame(FName GameId) const
{
	return true;
}

TArray<FPTBStageInfo> UPTBStageUnlockManager::GetUnlockedStages() const
{
	return AllStages;
}

FPTBRewardSummary UPTBStageUnlockManager::ApplyRoundResult(const FPTBRoundResult& Result)
{
	return FPTBRewardSummary();
}
TArray<FName> UPTBStageUnlockManager::CheckNewlyUnlockedStages()
{
	return TArray<FName>();
}

int32 UPTBStageUnlockManager::GetTotalStars() const
{
	return TotalStars;
}

bool UPTBStageUnlockManager::CheckPrerequisites(FName StageId) const
{
	return true;
}