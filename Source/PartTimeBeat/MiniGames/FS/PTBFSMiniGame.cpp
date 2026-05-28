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
	
	CacheFishingRuleSet();
	if (!FishRuleSet) return;
	
	TArray<AActor*> FoundActor;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActor);
	if (!FoundActor.Num()>0)
	{
		FishActor = Cast<AFishActor>(FoundActor[0]);
	}
	
	APTBRhythmCharacterBase* Character = 
	Cast<APTBRhythmCharacterBase>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	
	int32 TotalNoteCount = ChartAsset->NoteEvents.Num();
	
	
	float TotalDistance = FVector::Dist(FishActor->GetActorLocation(), Character->GetActorLocation());
	float StepDistance = TotalDistance /NoteCount;	
}

void APTBFSMiniGame::CacheFishingRuleSet()
{
	FishRuleSet = Cast<UPTBFSMiniGameRuleSet>(RuleSet);
	if (!FishRuleSet)
	{
		PTB_WARNING(LogPTBMiniGames,TEXT("낚시게임 룰셋 생성 실패"));
	}
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
