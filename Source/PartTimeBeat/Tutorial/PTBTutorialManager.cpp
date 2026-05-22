#include "Tutorial/PTBTutorialManager.h"
#include "Core/PTBSaveGame.h"
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

	UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));

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
	UE_LOG(LogTemp, Log, TEXT("TutorialManager: 튜토리얼 시작 - GameId: %s, 스텝 0: %s"),
	       *GameId.ToString(), *CurrentSteps[0].InstructionText.ToString());
}

void UPTBTutorialManager::AdvanceStep()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num()) return;
	CurrentStepIndex++;

	if (CurrentStepIndex < CurrentSteps.Num())
	{
		bCanSkip = CurrentSteps[CurrentStepIndex].bAllowSkip;
		OnTutorialStepChanged.Broadcast(CurrentStepIndex, CurrentSteps[CurrentStepIndex].InstructionText);
		UPTBSaveGame* SaveGame = Cast<UPTBSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("PTBSave"), 0));
		if (SaveGame)
		{
			SaveGame->UpdateTutorialStep(CurrentProfileId, CurrentSteps[CurrentStepIndex].GameId, CurrentStepIndex);
			UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
		}
		UE_LOG(LogTemp, Log, TEXT("TutorialManager: 스텝 %d: %s"),
		       CurrentStepIndex, *CurrentSteps[CurrentStepIndex].InstructionText.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TutorialManager: 마지막 스텝 → CompleteTutorial 호출"));
		CompleteTutorial(CurrentSteps[CurrentStepIndex - 1].GameId, CurrentProfileId);
	}
}

void UPTBTutorialManager::SkipStep()
{
	if (!bIsTutorialActive) return;if (bCanSkip)
	{
		UE_LOG(LogTemp, Log, TEXT("TutorialManager: 스텝 %d 스킵!"), CurrentStepIndex);
		AdvanceStep();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialManager: 스텝 %d 스킵 불가!"), CurrentStepIndex);
	}
}

void UPTBTutorialManager::SkipTutorial()
{
	if (!bIsTutorialActive) return;
	if (CurrentStepIndex >= CurrentSteps.Num()) return;

	if (bCanSkip)
	{
		UE_LOG(LogTemp, Log, TEXT("TutorialManager: 튜토리얼 스킵 - GameId: %s"),
		       *CurrentSteps[CurrentStepIndex].GameId.ToString());
		CompleteTutorial(CurrentSteps[CurrentStepIndex].GameId, CurrentProfileId);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialManager: 스킵 불가 - 현재 스텝 %d"), CurrentStepIndex);
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

	SaveGame->MarkTutorialDone(ProfileId, GameId);
	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);

	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("TutorialManager: SaveGame 저장 실패 - GameId: %s"), *GameId.ToString());
	}

	bIsTutorialActive = false;
	UE_LOG(LogTemp, Log, TEXT("TutorialManager: 튜토리얼 완료 - GameId: %s"), *GameId.ToString());
	OnTutorialCompleted.Broadcast(GameId);
}
