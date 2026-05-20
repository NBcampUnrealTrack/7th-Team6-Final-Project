// Fill out your copyright notice in the Description page of Project Settings.


#include "Tutorial/PTBTutorialManager.h"
#include "Core/PTBSaveGame.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UPTBTutorialManager::UPTBTutorialManager() :
	bCanSkip(false),
	bIsTutorialActive(false),
	CurrentStepIndex(0),
	TutorialDataTable(nullptr),
	CurrentProfileId(TEXT(""))
{
}

bool UPTBTutorialManager::ShouldShowTutorial(FName GameId, const FString& ProfileId)
{
	UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));
	if (!SaveGame) return true;
	
	FName Key = FName(*(ProfileId + TEXT("_") + GameId.ToString()));
	bool bAlreadyCompleted = SaveGame->TutorialFlags.FindRef(Key);
	
	return !bAlreadyCompleted;
}

void UPTBTutorialManager::StartTutorial(FName GameId, const FString& ProfileId)
{
	if (bIsTutorialActive) return;
	if (!TutorialDataTable) return;
	
	TArray<FPTBTutorialStepRow*> AllRow;
	
	TutorialDataTable->GetAllRows<FPTBTutorialStepRow>(TEXT(""), AllRow);
	
	CurrentSteps.Empty();
	for (FPTBTutorialStepRow* Row : AllRow)
	{
		if (Row && Row->GameId == GameId)
		{
			CurrentSteps.Add(*Row);
		}
	}

	if (CurrentSteps.Num() == 0)return;
	CurrentProfileId = ProfileId;
	CurrentStepIndex = 0;
	bIsTutorialActive = true;
	bCanSkip = CurrentSteps[CurrentStepIndex].bAllowSkip;
	OnTutorialStepChanged.Broadcast(CurrentStepIndex, CurrentSteps[0].InstructionText);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green,
	                                 FString::Printf(
		                                 TEXT("튜토리얼 시작 - 스텝 0: %s"), *CurrentSteps[0].InstructionText.ToString()));
}

void UPTBTutorialManager::AdvanceStep()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num()) return;
	CurrentStepIndex++;

	if (CurrentStepIndex < CurrentSteps.Num())
	{
		bCanSkip = CurrentSteps[CurrentStepIndex].bAllowSkip;
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow,
		                                 FString::Printf(
			                                 TEXT("스텝 %d: %s"), CurrentStepIndex,
			                                 *CurrentSteps[CurrentStepIndex].InstructionText.ToString()));
		OnTutorialStepChanged.Broadcast(CurrentStepIndex, CurrentSteps[CurrentStepIndex].InstructionText);
	}
}

void UPTBTutorialManager::SkipTutorial()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num())return;
	if (bCanSkip)
	{
		CompleteTutorial(CurrentSteps[CurrentStepIndex].GameId, CurrentProfileId);
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Yellow, TEXT("튜토리얼 스킵!"));
	}
}

void UPTBTutorialManager::CompleteTutorial(FName GameId, const FString& ProfileId)
{
	UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));
	if (!SaveGame)
	{
		SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::CreateSaveGameObject(UPTBSaveGame::StaticClass()));
	}
    
	if (!SaveGame) return;
    
	FName Key = FName(*(ProfileId + TEXT("_") + GameId.ToString()));
	SaveGame->MarkTutorialDone(ProfileId, GameId);
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
	bIsTutorialActive = false;
	OnTutorialCompleted.Broadcast(GameId);
}
