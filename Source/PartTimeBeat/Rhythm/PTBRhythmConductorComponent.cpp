#include "Rhythm/PTBRhythmConductorComponent.h"

#include "Audio/PTBWwiseAudioManager.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "GameFramework/Actor.h"
#include "Rhythm/PTBRhythmChartAsset.h"

namespace PTBRhythmConductorInternal
{
	constexpr float MillisecondsPerSecond = 1000.0f;
	constexpr float MillisecondsPerMinute = 60000.0f;
	constexpr int32 MinBeatsPerBar = 1;

	float CalculateBeat(float ChartTimeMs, float BPM)
	{
		if (BPM <= 0.0f)
		{
			return 0.0f;
		}

		return (ChartTimeMs / MillisecondsPerMinute) * BPM;
	}
}

UPTBRhythmConductorComponent::UPTBRhythmConductorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ChartAsset = nullptr;
	AudioManager = nullptr;
	RhythmSyncComponent = nullptr;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	WwisePlayingId = 0;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	NextArmIndex = 0;
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

	if (!RhythmSyncComponent && GetOwner())
	{
		SetRhythmSyncComponent(GetOwner()->FindComponentByClass<UPTBWwiseRhythmSyncComponent>());
	}
}

void UPTBRhythmConductorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!bIsPlaying || bIsPaused)
	{
		return;
	}

	float ChartTimeMs = 0.0f;
	float CueBeat = 0.0f;

	if (RhythmSyncComponent)
	{
		CurrentTimeMs = RhythmSyncComponent->GetRawPlaybackTimeMs();
		CurrentBeat = RhythmSyncComponent->GetChartBeat();
		ChartTimeMs = RhythmSyncComponent->GetChartTimeMs();
		CueBeat = RhythmSyncComponent->GetVisualBeat();
	}
	else if (AudioManager && WwisePlayingId != 0)
	{
		CurrentTimeMs = AudioManager->GetPlaybackPositionMs(WwisePlayingId);
		ChartTimeMs = CurrentTimeMs - ChartOffsetMs;
		CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(ChartTimeMs, ChartData.BPM);
		CueBeat = CurrentBeat;
	}
	else
	{
		CurrentTimeMs += DeltaTime * PTBRhythmConductorInternal::MillisecondsPerSecond;
		ChartTimeMs = CurrentTimeMs - ChartOffsetMs;
		CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(ChartTimeMs, ChartData.BPM);
		CueBeat = CurrentBeat;
	}

	const int32 BeatTickIndex = FMath::FloorToInt(CurrentBeat);
	if (BeatTickIndex > LastBeatTickIndex)
	{
		LastBeatTickIndex = BeatTickIndex;
		OnBeatTick.Broadcast(CurrentBeat);
	}

	const int32 SafeBeatsPerBar = FMath::Max(PTBRhythmConductorInternal::MinBeatsPerBar, BeatsPerBar);
	const int32 BarTickIndex = BeatTickIndex / SafeBeatsPerBar;
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
	const float EffectiveArmLeadTimeMs = FMath::Max(0.0f, ArmLeadTimeMs);

	while (Notes.IsValidIndex(NextArmIndex) && Notes[NextArmIndex].TimeMs - EffectiveArmLeadTimeMs <= ChartTimeMs)
	{
		OnNoteArm.Broadcast(Notes[NextArmIndex]);
		++NextArmIndex;
	}

	while (Notes.IsValidIndex(NextCueIndex) && Notes[NextCueIndex].BeatTime - CueLeadBeats <= CueBeat)
	{
		OnNoteCue.Broadcast(Notes[NextCueIndex]);
		++NextCueIndex;
	}

	while (Notes.IsValidIndex(NextNoteIndex) && Notes[NextNoteIndex].TimeMs <= ChartTimeMs)
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
	if (!RhythmSyncComponent && GetOwner())
	{
		SetRhythmSyncComponent(GetOwner()->FindComponentByClass<UPTBWwiseRhythmSyncComponent>());
	}

	UPTBWwiseAudioManager* ResolvedAudioManager = AudioManager.Get();
	if (!ResolvedAudioManager && RhythmSyncComponent)
	{
		ResolvedAudioManager = RhythmSyncComponent->WwiseManager.Get();
	}

	UPTBRhythmChartAsset* TimingOnlyChartAsset = NewObject<UPTBRhythmChartAsset>(this, NAME_None, RF_Transient);
	TimingOnlyChartAsset->ChartId = Data.ChartId;
	TimingOnlyChartAsset->ChartData = Data;
	TimingOnlyChartAsset->NoteEvents.Reset();

	StartConductor(TimingOnlyChartAsset, PlayingId, ResolvedAudioManager);
}

void UPTBRhythmConductorComponent::StartConductor(UPTBRhythmChartAsset* InChartAsset, int32 PlayingId, UPTBWwiseAudioManager* InAudioManager)
{
	ChartAsset = InChartAsset;
	if (!RhythmSyncComponent && GetOwner())
	{
		SetRhythmSyncComponent(GetOwner()->FindComponentByClass<UPTBWwiseRhythmSyncComponent>());
	}

	UPTBWwiseAudioManager* ResolvedAudioManager = InAudioManager;
	if (!ResolvedAudioManager && RhythmSyncComponent)
	{
		ResolvedAudioManager = RhythmSyncComponent->WwiseManager.Get();
	}

	AudioManager = ResolvedAudioManager;
	ChartData = InChartAsset ? InChartAsset->ChartData : FPTBChartData();
	WwisePlayingId = PlayingId;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	NextArmIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = ChartData.OffsetMs;
	bIsPlaying = InChartAsset != nullptr;
	bIsPaused = false;
	bAllNotesPassed = false;

	if (RhythmSyncComponent && bIsPlaying)
	{
		if (AudioManager)
		{
			RhythmSyncComponent->SetWwiseManager(AudioManager.Get());
		}

		RhythmSyncComponent->SetBeatsPerBar(BeatsPerBar);
		RhythmSyncComponent->StartSync(WwisePlayingId, ChartData.BPM, ChartOffsetMs);
	}
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
	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->StopSync();
	}

	ChartAsset = nullptr;
	AudioManager = nullptr;
	CurrentBeat = 0.0f;
	CurrentTimeMs = 0.0f;
	WwisePlayingId = 0;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	NextArmIndex = 0;
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

void UPTBRhythmConductorComponent::SetArmLeadTimeMs(float InArmLeadTimeMs)
{
	ArmLeadTimeMs = FMath::Max(0.0f, InArmLeadTimeMs);
}

void UPTBRhythmConductorComponent::SetBeatsPerBar(int32 InBeatsPerBar)
{
	BeatsPerBar = FMath::Max(PTBRhythmConductorInternal::MinBeatsPerBar, InBeatsPerBar);

	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->SetBeatsPerBar(BeatsPerBar);
	}
}

void UPTBRhythmConductorComponent::SetRhythmSyncComponent(UPTBWwiseRhythmSyncComponent* InRhythmSyncComponent)
{
	RhythmSyncComponent = InRhythmSyncComponent;

	if (RhythmSyncComponent)
	{
		AddTickPrerequisiteComponent(RhythmSyncComponent);
	}
}
