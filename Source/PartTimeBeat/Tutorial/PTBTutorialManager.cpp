#include "Tutorial/PTBTutorialManager.h"
#include "Core/PTBSaveGame.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"

UPTBTutorialManager::UPTBTutorialManager() :
	bCanSkip(false),
	bIsTutorialActive(false),
	CurrentStepIndex(0),
	TutorialDataTable(nullptr),
	CurrentProfileId(TEXT(""))
{
}

bool UPTBTutorialManager::ShouldShowTutorial(FName GameId, const FString& ProfileId) const
{
	UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));
	if (!SaveGame) return true;

	FName Key = FName(*(ProfileId + TEXT("_") + GameId.ToString()));
	bool bAlreadyCompleted = SaveGame->TutorialFlags.FindRef(Key);

	return !bAlreadyCompleted;
}

void UPTBTutorialManager::StartTutorial(FName GameId, const FString& ProfileId, bool bForce)
{
	if (bIsTutorialActive) return;
	if (!TutorialDataTable) return;
	if (!bForce && !ShouldShowTutorial(GameId, ProfileId)) return;

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
	CurrentSteps.Sort([](const FPTBTutorialStepRow& A, const FPTBTutorialStepRow& B)
	{
		return A.StepIndex < B.StepIndex;
	});

	if (CurrentSteps.Num() == 0) return;

	UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame) return;
	
	FName Key = FName(*(ProfileId + TEXT("_") + GameId.ToString()));
	int32 LastViewedStep = -1;
	if (SaveGame)
	{
		int32* Found = SaveGame->TutorialStepFlags.Find(Key);
		if (Found)
			LastViewedStep = *Found;
	}
	for (auto& Step : CurrentSteps)
	{
		if (Step.StepIndex <= LastViewedStep)
			Step.bAllowSkip = true;
	}

	CurrentProfileId = ProfileId;
	CurrentStepIndex = 0;
	bIsTutorialActive = true;
	bCanSkip = CurrentSteps[CurrentStepIndex].bAllowSkip;
	OnTutorialStepChanged.Broadcast(CurrentStepIndex, CurrentSteps[0].InstructionText);
	if (SaveGame)
	{
		SaveGame->UpdateTutorialStep(ProfileId, GameId, 0);
		UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
	}
	PTB_WARNING(LogPTBTutorial, TEXT("TutorialManager: 튜토리얼 시작 - GameId: %s, 스텝 0: %s"),
	       *GameId.ToString(), *CurrentSteps[0].InstructionText.ToString());
}

void UPTBTutorialManager::AdvanceStep()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num()) return;
	CurrentStepIndex++;

	UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
	if (CurrentStepIndex < CurrentSteps.Num())
	{
		bCanSkip = CurrentSteps[CurrentStepIndex].bAllowSkip;
		OnTutorialStepChanged.Broadcast(CurrentStepIndex, CurrentSteps[CurrentStepIndex].InstructionText);
		if (SaveGame)
		{
			SaveGame->UpdateTutorialStep(CurrentProfileId, CurrentSteps[CurrentStepIndex].GameId, CurrentStepIndex);
			bool bSaved= UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
			PTB_WARNING(LogPTBTutorial, TEXT("스텝 %d 저장 %s"), CurrentStepIndex, bSaved ? TEXT("성공") : TEXT("실패"));
		}	
	}
	else
	{
		if (SaveGame)
		{
			SaveGame->UpdateTutorialStep(CurrentProfileId, CurrentSteps[CurrentStepIndex-1].GameId, CurrentStepIndex-1);
			UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
		}
		CompleteTutorial(CurrentSteps[CurrentStepIndex - 1].GameId, CurrentProfileId);
	}
}

void UPTBTutorialManager::SkipStep()
{
	if (!bIsTutorialActive) return;
	if (bCanSkip)
	{
		//UI 스킵 가능 
		AdvanceStep();
	}
	else
	{
		// UI 스킵 불가능
	}
}

void UPTBTutorialManager::SkipTutorial()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num()) return;

	if (bCanSkip)
	{
		//UI 스킵 가능 
		CompleteTutorial(CurrentSteps[CurrentStepIndex].GameId, CurrentProfileId);
	}
	else
	{
		// UI 스킵 불가능
	}
}

void UPTBTutorialManager::CompleteTutorial(FName GameId, const FString& ProfileId)
{
	UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
	if (!SaveGame) return;

	SaveGame->MarkTutorialDone(ProfileId, GameId);
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);

	if (!bSaved)
	{
		PTB_WARNING(LogPTBTutorial,TEXT("TutorialManager: SaveGame 저장 실패 - GameId: %s"), *GameId.ToString());
	}

	bIsTutorialActive = false;
	PTB_WARNING(LogPTBTutorial ,TEXT("TutorialManager: 튜토리얼 완료 - GameId: %s"), *GameId.ToString());
	OnTutorialCompleted.Broadcast(GameId);
}

UPTBSaveGame* UPTBTutorialManager::GetOrCreateSaveGame()
{
	UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(
		UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));
    
	if (!SaveGame)
	{
		SaveGame = Cast<UPTBSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UPTBSaveGame::StaticClass()));
	}
	return SaveGame;
}