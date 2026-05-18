#include "Rhythm/PTBRhythmConductorComponent.h"

// Sets default values for this component's properties
UPTBRhythmConductorComponent::UPTBRhythmConductorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPTBRhythmConductorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPTBRhythmConductorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPTBRhythmConductorComponent::StartConductor(const FPTBChartData & Data, int32 PlayingId)
{
}
void UPTBRhythmConductorComponent::PauseConductor()
{

}

void UPTBRhythmConductorComponent::ResumeConductor()
{

}

void UPTBRhythmConductorComponent::StopConductor()
{

}

float UPTBRhythmConductorComponent::GetCurrentMusicTimeMs() const
{
	return 0;
}

float UPTBRhythmConductorComponent::GetCurrentBeat() const
{
	return 0;
}