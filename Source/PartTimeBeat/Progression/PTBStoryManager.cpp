// Fill out your copyright notice in the Description page of Project Settings.


#include "Progression/PTBStoryManager.h"

UPTBStoryManager::UPTBStoryManager()
{
}

TArray<FName> UPTBStoryManager::CheckUnlockCondition(const FPTBRoundResult& Result)
{
	return TArray<FName>();
}

bool UPTBStoryManager::ShouldPlayStoryAfterResult(const FPTBRoundResult& Result, FName OutId)
{
	return true;
}

void UPTBStoryManager::PlayChapter(FName ChapterId)
{
}

void UPTBStoryManager::SkipChapter()
{
}

void UPTBStoryManager::MarkAsViewed(FName ChapterId)
{
}

TArray<FPTBStoryChapter> UPTBStoryManager::GetUnviewedChapters() const
{
	return TArray<FPTBStoryChapter>();
}

bool UPTBStoryManager::HasNewStory() const
{
	return true;
}

FName UPTBStoryManager::ResolveEndingByMoney(int32 TotalMoney) const
{
	return FName();
}
