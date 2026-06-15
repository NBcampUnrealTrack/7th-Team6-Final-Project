

#include "PTBLCMiniGame.h"

#include "Components/BoxComponent.h"
#include "Debug/PTBTeamLog.h"
#include "PTBLCLogisticBox.h"


APTBLCMiniGame::APTBLCMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

void APTBLCMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
}

void APTBLCMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
}

void APTBLCMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
}

void APTBLCMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	if (Note.ActionType == EPTBActionType::ActionA ||
		Note.ActionType == EPTBActionType::ActionB ||
		Note.ActionType == EPTBActionType::ActionC)
	{
		if (!LogisticBoxClass)
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] HandleNoteCue skipped: LogisticBoxClass is null. NoteId=%d Action=%d"),
				Note.NoteId,
				static_cast<int32>(Note.ActionType));
			return;
		}

		UWorld* World = GetWorld();
		if (!World)
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] HandleNoteCue skipped: World is null. NoteId=%d Action=%d"),
				Note.NoteId,
				static_cast<int32>(Note.ActionType));
			return;
		}

		FRotator SpawnRotation = FRotator::ZeroRotator;
	
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
		FTransform SpawnTransform(SpawnRotation, BoxSpawnLocation);
	
		APTBLCLogisticBox* SpawnedActor = World->SpawnActor<APTBLCLogisticBox>(
			LogisticBoxClass,
			SpawnTransform,
			SpawnParams
		);
	
		if (SpawnedActor)
		{
			SpawnedActor->SetBoxMaterlalInstanceByActionType(Note.ActionType);
			PTB_RECORD(LogPTBMiniGames, TEXT("[LC] Logistic box spawned. NoteId=%d Action=%d Location=(%.2f, %.2f, %.2f) Class=%s"),
				Note.NoteId,
				static_cast<int32>(Note.ActionType),
				BoxSpawnLocation.X,
				BoxSpawnLocation.Y,
				BoxSpawnLocation.Z,
				*GetNameSafe(SpawnedActor->GetClass()));
		}
		else
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] Logistic box spawn failed. NoteId=%d Action=%d Location=(%.2f, %.2f, %.2f) Class=%s"),
				Note.NoteId,
				static_cast<int32>(Note.ActionType),
				BoxSpawnLocation.X,
				BoxSpawnLocation.Y,
				BoxSpawnLocation.Z,
				*GetNameSafe(LogisticBoxClass));
		}
	}
	else PTB_WARNING(LogPTBMiniGames, TEXT("[LC] HandleNoteCue: Unsupported ActionType. NoteId=%d Action=%d"),
		Note.NoteId,
		static_cast<int32>(Note.ActionType));
}

void APTBLCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
	
	
	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		FPTBNoteEvent EmptyInputNote;
		OnJudgement.Broadcast(Result, EmptyInputNote);
		return;
	}
	
	if (Result.Reason == EPTBJudgementReason::Note || Result.Reason == EPTBJudgementReason::WrongInput)
	{
		FPTBNoteEvent Note;
		OnJudgement.Broadcast(Result, Note);
		
		if (CollisionBox)
		{
			TArray<AActor*> OverlappingActors;
			CollisionBox->GetOverlappingActors(OverlappingActors);
			if (OverlappingActors.Num() == 0)
			{
				return;
			}
			for (AActor* OverlappingActor : OverlappingActors)
			{
				if (OverlappingActor && OverlappingActor->IsA<APTBLCLogisticBox>())
				{
					if (Result.JudgementType == EPTBJudgementType::Good || Result.JudgementType == EPTBJudgementType::Perfect || Result.JudgementType == EPTBJudgementType::HighPerfect)
					{
						APTBLCLogisticBox* LogisticBox = Cast<APTBLCLogisticBox>(OverlappingActor);
						if (LogisticBox && !LogisticBox->GetIsPackaged())
						{
							LogisticBox->ChangeMeshToBox();
							LogisticBox->SetBoxStateByActionType(Result.ActionType);
						}
					}
				}
			}
		}
	}
}
