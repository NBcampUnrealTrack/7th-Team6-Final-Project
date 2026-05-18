#include "Rhythm/PTBJudgementSystem.h"

UPTBJudgementSystem::UPTBJudgementSystem()
{

	PrimaryComponentTick.bCanEverTick = true;
}

void UPTBJudgementSystem::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPTBJudgementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UPTBJudgementSystem::Initialize(const FPTBChartData& Chart, float UserOffset)
{

}
void UPTBJudgementSystem::RegisterNoteEvent(const FPTBNoteEvent& Note)
{

}
FPTBJudgementResult UPTBJudgementSystem::EvaluateInput(EPTBActionType Action, float InputTimeMs)
{
	return FPTBJudgementResult();
}
TArray<FPTBJudgementResult> UPTBJudgementSystem::ForceMissExpiredNotes(float CurrentTimeMs)
{
	return TArray<FPTBJudgementResult>();
}
void UPTBJudgementSystem::Reset()
{

}