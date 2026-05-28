// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSMiniGame.h"
#include "Characters/PTBRhythmCharacterBase.h"
#include "FishActor.h"
#include "PTBFSMiniGameRuleSet.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Rhythm/PTBRhythmChartAsset.h"


// Sets default values
APTBFSMiniGame::APTBFSMiniGame()
{
	PrimaryActorTick.bCanEverTick = false;
}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	FishRuleSet = Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	
	APTBRhythmCharacterBase* Character = 
	Cast<APTBRhythmCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
	TArray<AActor*> FoundActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFishActor::StaticClass(), FoundActor);
	if (FoundActor.Num() > 0)
	{
		FishActor = Cast<AFishActor>(FoundActor[0]);
	}
	
	if (!Character)return;
	if (!FishActor)return;
	if (!FishRuleSet) return;
	if (!ChartAsset) return;
	
	int32 TotalNoteCount = ChartAsset->NoteEvents.Num();
	if (TotalNoteCount <= 0)return;
	float TotalDistance = FVector::Dist(FishActor->GetActorLocation(), Character->GetActorLocation());
	StepDistance = TotalDistance /TotalNoteCount;	
}

void APTBFSMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);
}

void APTBFSMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
}

void APTBFSMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
}

void APTBFSMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
}

void APTBFSMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
{
	Super::PlayJudgementFeedback(Result);
}

FPTBMiniGameResultPayload APTBFSMiniGame::BuildResultPayload() const
{
	return Super::BuildResultPayload();
}

void APTBFSMiniGame::OnAllNotesPassedFishing()
{
}

void APTBFSMiniGame::ApplyDistanceDelta(float Delta)
{
}

EFishingLineState APTBFSMiniGame::CalculateLineState(float Distance) const
{
	
	return EFishingLineState::None;
}

void APTBFSMiniGame::HandleActionAInput()
{
	HandleRhythmInput(EPTBActionType::ActionA);
}

void APTBFSMiniGame::HandleActionBInput()
{
	HandleRhythmInput(EPTBActionType::ActionB);
}

void APTBFSMiniGame::HandleActionCInput()
{
	HandleRhythmInput(EPTBActionType::ActionC);
}

void APTBFSMiniGame::HandleActionDInput()
{
	HandleRhythmInput(EPTBActionType::ActionD);
}

void APTBFSMiniGame::HandleActionEInput()
{
	HandleRhythmInput(EPTBActionType::ActionE);
}
