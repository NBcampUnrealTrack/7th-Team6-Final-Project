#include "Audio/PTBWwiseRhythmSyncComponent.h"

#include "Audio/PTBWwiseAudioManager.h"

namespace PTBWwiseRhythmSyncInternal
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

	if (!SyncData.bIsPlaying)
	{
		return;
	}

	float PlaybackMs = SyncData.CurrentPlaybackMs;
	if (WwiseManager && CurrentPlayingId != 0)
	{
		if (!WwiseManager->IsEventPlaying(CurrentPlayingId))
		{
			SyncData.bIsPlaying = false;
			return;
		}

		PlaybackMs = WwiseManager->GetPlaybackPositionMs(CurrentPlayingId);
	}
	else
	{
		PlaybackMs += DeltaTime * PTBWwiseRhythmSyncInternal::MillisecondsPerSecond;
	}

	SyncData.CurrentPlaybackMs = FMath::Max(0.0f, PlaybackMs);
	SyncData.CurrentBeat = GetChartBeat();

	const int32 BeatTickIndex = FMath::FloorToInt(SyncData.CurrentBeat);
	if (BeatTickIndex >= 0 && BeatTickIndex > LastBeatTickIndex)
	{
		LastBeatTickIndex = BeatTickIndex;
		OnBeatTick.Broadcast(SyncData.CurrentBeat);
	}

	const int32 SafeBeatsPerBar = FMath::Max(PTBWwiseRhythmSyncInternal::MinBeatsPerBar, BeatsPerBar);
	const int32 BarTickIndex = BeatTickIndex / SafeBeatsPerBar;
	if (BarTickIndex >= 0 && BarTickIndex > LastBarTickIndex)
	{
		LastBarTickIndex = BarTickIndex;
		OnBarTick.Broadcast(BarTickIndex);
	}
}

void UPTBWwiseRhythmSyncComponent::SetWwiseManager(UPTBWwiseAudioManager* InWwiseManager)
{
	WwiseManager = InWwiseManager;
}

void UPTBWwiseRhythmSyncComponent::SetUserOffsets(float InInputOffsetMs, float InVisualOffsetMs, float InSoundOffsetMs)
{
	InputOffsetMs = InInputOffsetMs;
	VisualOffsetMs = InVisualOffsetMs;
	SoundOffsetMs = InSoundOffsetMs;
}

void UPTBWwiseRhythmSyncComponent::SetBeatsPerBar(int32 InBeatsPerBar)
{
	BeatsPerBar = FMath::Max(PTBWwiseRhythmSyncInternal::MinBeatsPerBar, InBeatsPerBar);
}

void UPTBWwiseRhythmSyncComponent::StartSync(int32 PlayingId, float BPM, float OffsetMs)
{
	CurrentPlayingId = PlayingId;
	ChartOffsetMs = OffsetMs;
	SyncData = FPTBWwiseSyncData();
	SyncData.BPM = FMath::Max(0.0f, BPM);
	SyncData.CurrentBeat = GetChartBeat();
	SyncData.bIsPlaying = CurrentPlayingId != 0 || SyncData.BPM > 0.0f;

	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
}

void UPTBWwiseRhythmSyncComponent::StopSync()
{
	CurrentPlayingId = 0;
	ChartOffsetMs = 0.0f;
	SyncData = FPTBWwiseSyncData();

	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
}

float UPTBWwiseRhythmSyncComponent::GetRawPlaybackTimeMs() const
{
	return SyncData.CurrentPlaybackMs;
}

float UPTBWwiseRhythmSyncComponent::GetChartTimeMs() const
{
	return GetRawPlaybackTimeMs() - ChartOffsetMs;
}

float UPTBWwiseRhythmSyncComponent::GetAudibleChartTimeMs() const
{
	return GetChartTimeMs() + SoundOffsetMs;
}

float UPTBWwiseRhythmSyncComponent::GetVisualChartTimeMs() const
{
	return GetChartTimeMs() + VisualOffsetMs;
}

float UPTBWwiseRhythmSyncComponent::GetInputJudgeTimeMs() const
{
	return GetChartTimeMs() + InputOffsetMs;
}

float UPTBWwiseRhythmSyncComponent::GetChartBeat() const
{
	return PTBWwiseRhythmSyncInternal::CalculateBeat(GetChartTimeMs(), SyncData.BPM);
}

float UPTBWwiseRhythmSyncComponent::GetVisualBeat() const
{
	return PTBWwiseRhythmSyncInternal::CalculateBeat(GetVisualChartTimeMs(), SyncData.BPM);
}

float UPTBWwiseRhythmSyncComponent::GetAudibleBeat() const
{
	return PTBWwiseRhythmSyncInternal::CalculateBeat(GetAudibleChartTimeMs(), SyncData.BPM);
}

float UPTBWwiseRhythmSyncComponent::GetCurrentBeat() const
{
	return GetChartBeat();
}

float UPTBWwiseRhythmSyncComponent::GetCurrentTimeMs() const
{
	return GetRawPlaybackTimeMs();
}

void UPTBWwiseRhythmSyncComponent::CalibrateInputOffset(float DeltaMs)
{
	InputOffsetMs += DeltaMs;
}
