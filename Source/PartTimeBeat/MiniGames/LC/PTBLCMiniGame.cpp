

#include "PTBLCMiniGame.h"

#include "Components/BoxComponent.h"
#include "PTBLCLogisticBox.h"


// Sets default values
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
	
	if (Result.Reason == EPTBJudgementReason::Note)
	{
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
						if (LogisticBox)
						{
							LogisticBox->ChangeMeshToBox();
							LogisticBox->SetBoxMaterlalInstanceByActionType(Result.ActionType);
						}
					}
				}
			}
		}
	}
}
