#include "Audio/PTBWwiseRhythmSyncComponent.h"

UPTBWwiseRhythmSyncComponent::UPTBWwiseRhythmSyncComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPTBWwiseRhythmSyncComponent::BeginPlay()
{
	Super::BeginPlay();

}

void UPTBWwiseRhythmSyncComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPTBWwiseRhythmSyncComponent::StartSync(int32 PlayingId, float BPM, float OffsetMs)
{

}

void UPTBWwiseRhythmSyncComponent::StopSync()
{

}

float UPTBWwiseRhythmSyncComponent::GetCurrentBeat() const
{
	return 0;
}

float UPTBWwiseRhythmSyncComponent::GetCurrentTimeMs() const
{
	return 0;
}

void UPTBWwiseRhythmSyncComponent::CalibrateOffset(float DeltaMs)
{
}
