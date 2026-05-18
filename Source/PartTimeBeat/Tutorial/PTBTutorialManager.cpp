// Fill out your copyright notice in the Description page of Project Settings.


#include "Tutorial/PTBTutorialManager.h"
#include "Core/PTBStructEnums.h"
// Sets default values for this component's properties
UPTBTutorialManager::UPTBTutorialManager()
{
}

bool UPTBTutorialManager::ShouldShowTutorial(FName GameId, const FString& ProfileId) const
{
	return true;
}

void UPTBTutorialManager::StartTutorial(FName GameId)
{
   
}

void UPTBTutorialManager::AdvanceStep()
{
    
}

void UPTBTutorialManager::SkipTutorial()
{

}

void UPTBTutorialManager::CompleteTutorial(FName GameId, const FString& ProfileId)
{

}