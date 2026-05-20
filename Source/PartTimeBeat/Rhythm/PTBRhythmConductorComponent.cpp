#include "Rhythm/PTBRhythmConductorComponent.h"

#include "Audio/PTBWwiseAudioManager.h"
#include "Rhythm/PTBRhythmChartAsset.h"

UPTBRhythmConductorComponent::UPTBRhythmConductorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ChartAsset = nullptr;
	AudioManager = nullptr;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	WwisePlayingId = 0;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = 0.0f;
	bIsPlaying = false;
	bIsPaused = false;
	bAllNotesPassed = false;
}

void UPTBRhythmConductorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UPTBRhythmConductorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsPlaying || bIsPaused)
	{
		return;
	}

	if (AudioManager && WwisePlayingId != 0)
	{
		CurrentTimeMs = AudioManager->GetPlaybackPositionMs(WwisePlayingId);
	}
	else
	{
		CurrentTimeMs += DeltaTime * 1000.0f;
	}

	CurrentBeat = ChartData.BPM > 0.0f
		? ((CurrentTimeMs - ChartOffsetMs) / 60000.0f) * ChartData.BPM
		: 0.0f;

	const int32 BeatTickIndex = FMath::FloorToInt(CurrentBeat);
	if (BeatTickIndex > LastBeatTickIndex)
	{
		LastBeatTickIndex = BeatTickIndex;
		OnBeatTick.Broadcast(CurrentBeat);
	}

	const int32 BarTickIndex = BeatTickIndex / 4;
	if (BarTickIndex > LastBarTickIndex)
	{
		LastBarTickIndex = BarTickIndex;
		OnBarTick.Broadcast(BarTickIndex);
	}

	if (!ChartAsset)
	{
		return;
	}

	const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;
	const float CueLeadBeats = FMath::Max(0.0f, LookAheadBeats);

	while (Notes.IsValidIndex(NextCueIndex) && Notes[NextCueIndex].BeatTime - CueLeadBeats <= CurrentBeat)
	{
		OnNoteCue.Broadcast(Notes[NextCueIndex]);
		++NextCueIndex;
	}

	while (Notes.IsValidIndex(NextNoteIndex) && Notes[NextNoteIndex].TimeMs + ChartOffsetMs <= CurrentTimeMs)
	{
		OnNoteEvent.Broadcast(Notes[NextNoteIndex]);
		++NextNoteIndex;
	}

	if (!bAllNotesPassed && NextNoteIndex >= Notes.Num())
	{
		bAllNotesPassed = true;
		OnAllNotesPassed.Broadcast();
	}
}

void UPTBRhythmConductorComponent::StartConductor(const FPTBChartData & Data, int32 PlayingId)
{
	ChartAsset = nullptr;
	AudioManager = nullptr;
	ChartData = Data;
	WwisePlayingId = PlayingId;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = Data.OffsetMs;
	bIsPlaying = true;
	bIsPaused = false;
	bAllNotesPassed = true;
}

void UPTBRhythmConductorComponent::StartConductor(UPTBRhythmChartAsset* InChartAsset, int32 PlayingId, UPTBWwiseAudioManager* InAudioManager)
{
	ChartAsset = InChartAsset;
	AudioManager = InAudioManager;
	ChartData = InChartAsset ? InChartAsset->ChartData : FPTBChartData();
	WwisePlayingId = PlayingId;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = ChartData.OffsetMs;
	bIsPlaying = InChartAsset != nullptr;
	bIsPaused = false;
	bAllNotesPassed = false;
}

void UPTBRhythmConductorComponent::PauseConductor()
{
	bIsPaused = true;
}

void UPTBRhythmConductorComponent::ResumeConductor()
{
	if (bIsPlaying)
	{
		bIsPaused = false;
	}
}

void UPTBRhythmConductorComponent::StopConductor()
{
	ChartAsset = nullptr;
	AudioManager = nullptr;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	WwisePlayingId = 0;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	bIsPlaying = false;
	bIsPaused = false;
	bAllNotesPassed = false;
}

float UPTBRhythmConductorComponent::GetCurrentMusicTimeMs() const
{
	return CurrentTimeMs;
}

float UPTBRhythmConductorComponent::GetCurrentBeat() const
{
	return CurrentBeat;
}
