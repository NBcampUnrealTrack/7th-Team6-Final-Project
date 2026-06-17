

#include "PTBLCMiniGame.h"

#include "Components/BoxComponent.h"
#include "Debug/PTBTeamLog.h"
#include "PTBLCLogisticBox.h"


APTBLCMiniGame::APTBLCMiniGame()
{
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
			SpawnedActor->InitializeFromNote(Note);
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
			
			APTBLCLogisticBox* TargetLogisticBox = nullptr;
			for (AActor* OverlappingActor : OverlappingActors)
			{
				APTBLCLogisticBox* LogisticBox = Cast<APTBLCLogisticBox>(OverlappingActor);
				if (LogisticBox && !LogisticBox->GetIsPackaged() && LogisticBox->GetNoteId() == Result.NoteId)
				{
					TargetLogisticBox = LogisticBox;
					break;
				}
			}
			
			const bool bShouldPackage =
				Result.Reason == EPTBJudgementReason::WrongInput ||
				Result.JudgementType == EPTBJudgementType::Good ||
				Result.JudgementType == EPTBJudgementType::Perfect ||
				Result.JudgementType == EPTBJudgementType::HighPerfect;
			if (TargetLogisticBox && bShouldPackage)
			{
				const EPTBActionType PackageActionType = Result.Reason == EPTBJudgementReason::WrongInput
					? Result.InputActionType
					: Result.ActionType;
				TargetLogisticBox->PackageWithActionType(PackageActionType);
			}
		}
	}
}
