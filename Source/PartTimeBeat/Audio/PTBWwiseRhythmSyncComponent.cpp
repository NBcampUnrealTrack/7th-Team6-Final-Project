#include "Audio/PTBWwiseRhythmSyncComponent.h"

#include "Audio/PTBWwiseAudioManager.h"

namespace PTBWwiseRhythmSyncInternal
{
	constexpr float MillisecondsPerSecond = 1000.0f;
	constexpr float MillisecondsPerMinute = 60000.0f;
	constexpr int32 MinBeatsPerBar = 1;

	int32 GetTempoEventCount(const FPTBChartData& ChartData)
	{
		return FMath::Min(ChartData.TempoChangeBeats.Num(), ChartData.TempoChangeBpms.Num());
	}

	int32 GetTimeSignatureEventCount(const FPTBChartData& ChartData)
	{
		return FMath::Min3(
			ChartData.TimeSignatureChangeBeats.Num(),
			ChartData.TimeSignatureChangeNumerators.Num(),
			ChartData.TimeSignatureChangeDenominators.Num());
	}

	float GetBaseTempoBpm(const FPTBChartData& ChartData)
	{
		if (ChartData.BPM > 0.0f)
		{
			return ChartData.BPM;
		}

		if (ChartData.TempoChangeBpms.IsValidIndex(0))
		{
			return ChartData.TempoChangeBpms[0];
		}

		return 0.0f;
	}

	float CalculateBeatFromChartTimeMs(float ChartTimeMs, const FPTBChartData& ChartData)
	{
		const int32 TempoEventCount = GetTempoEventCount(ChartData);
		if (TempoEventCount <= 0)
		{
			const float BaseBpm = GetBaseTempoBpm(ChartData);
			if (BaseBpm <= 0.0f)
			{
				return 0.0f;
			}

			return (ChartTimeMs / MillisecondsPerMinute) * BaseBpm;
		}

		float SegmentStartBeat = ChartData.TempoChangeBeats[0];
		float SegmentStartTimeMs = 0.0f;
		float SegmentBpm = ChartData.TempoChangeBpms[0];

		for (int32 Index = 1; Index < TempoEventCount; ++Index)
		{
			const float NextBeat = ChartData.TempoChangeBeats[Index];
			const float DeltaBeat = NextBeat - SegmentStartBeat;
			const float NextSegmentStartTimeMs = SegmentStartTimeMs + (DeltaBeat * MillisecondsPerMinute / SegmentBpm);

			if (ChartTimeMs < NextSegmentStartTimeMs)
			{
				break;
			}

			SegmentStartBeat = NextBeat;
			SegmentStartTimeMs = NextSegmentStartTimeMs;
			SegmentBpm = ChartData.TempoChangeBpms[Index];
		}

		return SegmentStartBeat + ((ChartTimeMs - SegmentStartTimeMs) * SegmentBpm / MillisecondsPerMinute);
	}

	int32 ResolveBeatsPerBar(float Beat, const FPTBChartData& ChartData, int32 FallbackBeatsPerBar)
	{
		int32 ResolvedBeatsPerBar = FMath::Max(MinBeatsPerBar, FallbackBeatsPerBar);
		const int32 TimeSignatureEventCount = GetTimeSignatureEventCount(ChartData);
		if (TimeSignatureEventCount <= 0)
		{
			if (ChartData.TimeSignatureNumerator > 0)
			{
				return ChartData.TimeSignatureNumerator;
			}

			return ResolvedBeatsPerBar;
		}

		for (int32 Index = 0; Index < TimeSignatureEventCount; ++Index)
		{
			if (ChartData.TimeSignatureChangeBeats[Index] > Beat)
			{
				break;
			}

			ResolvedBeatsPerBar = FMath::Max(MinBeatsPerBar, ChartData.TimeSignatureChangeNumerators[Index]);
		}

		return ResolvedBeatsPerBar;
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
	SyncData.BPM = PTBWwiseRhythmSyncInternal::GetBaseTempoBpm(ActiveChartData);

	const int32 BeatTickIndex = FMath::FloorToInt(SyncData.CurrentBeat);
	if (BeatTickIndex >= 0 && BeatTickIndex > LastBeatTickIndex)
	{
		LastBeatTickIndex = BeatTickIndex;
		OnBeatTick.Broadcast(SyncData.CurrentBeat);
	}

	const int32 SafeBeatsPerBar = GetCurrentBeatsPerBar();
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

void UPTBWwiseRhythmSyncComponent::StartSync(int32 PlayingId, const FPTBChartData& InChartData)
{
	CurrentPlayingId = PlayingId;
	ActiveChartData = InChartData;
	ChartOffsetMs = InChartData.OffsetMs;
	SyncData = FPTBWwiseSyncData();
	SyncData.BPM = PTBWwiseRhythmSyncInternal::GetBaseTempoBpm(ActiveChartData);
	SyncData.CurrentBeat = GetChartBeat();
	SyncData.bIsPlaying = CurrentPlayingId != 0 || SyncData.BPM > 0.0f;

	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
}

void UPTBWwiseRhythmSyncComponent::StopSync()
{
	CurrentPlayingId = 0;
	ActiveChartData = FPTBChartData();
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
	return PTBWwiseRhythmSyncInternal::CalculateBeatFromChartTimeMs(GetChartTimeMs(), ActiveChartData);
}

float UPTBWwiseRhythmSyncComponent::GetVisualBeat() const
{
	return PTBWwiseRhythmSyncInternal::CalculateBeatFromChartTimeMs(GetVisualChartTimeMs(), ActiveChartData);
}

float UPTBWwiseRhythmSyncComponent::GetAudibleBeat() const
{
	return PTBWwiseRhythmSyncInternal::CalculateBeatFromChartTimeMs(GetAudibleChartTimeMs(), ActiveChartData);
}

int32 UPTBWwiseRhythmSyncComponent::GetCurrentBeatsPerBar() const
{
	return PTBWwiseRhythmSyncInternal::ResolveBeatsPerBar(GetChartBeat(), ActiveChartData, BeatsPerBar);
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
