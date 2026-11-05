#include "Audio/PTBWwiseRhythmSyncComponent.h"

#include "Audio/PTBWwiseAudioManager.h"
#include "Debug/PTBLogChannels.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformTime.h"

// 테스트용: StartSync 후 N ms 동안 Wwise 재생 위치 조회를 실패로 취급한다.
// 캐시 상태와 상관없이 "음원이 늦게 시작되는 첫 실행" 상황을 강제로 재현하는 용도.
static TAutoConsoleVariable<float> CVarPTBSyncDebugBlockPositionMs(
	TEXT("ptb.Sync.DebugBlockPositionMs"),
	0.0f,
	TEXT("Treat Wwise playback position as unavailable for N ms after StartSync (test only)."),
	ECVF_Cheat);

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
		if (ChartData.TimeSignatureNumerator > 0)
		{
			ResolvedBeatsPerBar = ChartData.TimeSignatureNumerator;
		}

		const int32 TimeSignatureEventCount = GetTimeSignatureEventCount(ChartData);
		if (TimeSignatureEventCount <= 0)
		{
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

	const float PrevPlaybackMs = SyncData.CurrentPlaybackMs;
	float PlaybackMs = PrevPlaybackMs;
	const float DeltaMs = DeltaTime * PTBWwiseRhythmSyncInternal::MillisecondsPerSecond;
	bool bUsedWwisePosition = false;

	if (WwiseManager && CurrentPlayingId != 0)
	{
		const double SinceStartMs = (FPlatformTime::Seconds() - SyncStartRealSeconds) * 1000.0;
		const bool bDebugBlocked = SinceStartMs < CVarPTBSyncDebugBlockPositionMs.GetValueOnGameThread();

		float WwisePlaybackMs = 0.0f;
		if (!bDebugBlocked && WwiseManager->TryGetPlaybackPositionMs(CurrentPlayingId, WwisePlaybackMs))
		{
			if (!bHasWwisePosition)
			{
				bHasWwisePosition = true;
				UE_LOG(LogWwise, Log, TEXT("[SyncCheck] First Wwise position after %.0fms (pos=%.0fms, fallbackSoFar=%.0fms)"),
					SinceStartMs, WwisePlaybackMs, FallbackAdvancedMs);
			}
			PlaybackMs = WwisePlaybackMs;
			bUsedWwisePosition = true;
		}
		else if (!bHasWwisePosition && SinceStartMs < MaxWaitForFirstWwisePositionMs)
		{
			// 음원이 실제로 시작되기 전: 게임 시계를 앞서 보내지 않고 0에 고정
			PlaybackMs = 0.0f;
		}
		else if (!bHasWwisePosition)
		{
			// 대기 시간 초과: Wwise 위치를 못 받는 이벤트로 보고 기존 방식(시작 후 경과 시간)으로 진행
			PlaybackMs = static_cast<float>(SinceStartMs);
			FallbackAdvancedMs = PlaybackMs;
		}
		else
		{
			PlaybackMs += DeltaMs;
			FallbackAdvancedMs += DeltaMs;
		}
	}
	else
	{
		PlaybackMs += DeltaMs;
	}

	SyncData.CurrentPlaybackMs = FMath::Max(0.0f, PlaybackMs);
	// 일시정지(위치 변화 없음)는 제외하고, 실제 프레임 시간 대비 재생 위치가 튄 정도를 기록
	if (bUsedWwisePosition && !FMath::IsNearlyEqual(SyncData.CurrentPlaybackMs, PrevPlaybackMs))
	{
		MaxPlaybackJumpMs = FMath::Max(MaxPlaybackJumpMs, FMath::Abs(SyncData.CurrentPlaybackMs - PrevPlaybackMs - DeltaMs));
	}

	if (!bUseVisualSmoothing)
	{
		SmoothedPlaybackMs = SyncData.CurrentPlaybackMs;
	}
	else
	{
		const float SmoothDeltaMs = SyncData.CurrentPlaybackMs - SmoothedPlaybackMs;
		const float AbsSmoothDeltaMs = FMath::Abs(SmoothDeltaMs);
		if (AbsSmoothDeltaMs <= VisualSmoothingSnapThresholdMs
			|| (VisualSmoothingHardSnapThresholdMs > 0.0f && AbsSmoothDeltaMs >= VisualSmoothingHardSnapThresholdMs))
		{
			SmoothedPlaybackMs = SyncData.CurrentPlaybackMs;
		}
		else
		{
			const float MaxStepMs = FMath::Max(0.0f, MaxVisualSmoothingStepMs);
			SmoothedPlaybackMs = MaxStepMs > 0.0f
				? SmoothedPlaybackMs + FMath::Clamp(SmoothDeltaMs, -MaxStepMs, MaxStepMs)
				: SyncData.CurrentPlaybackMs;
		}
	}

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
	SmoothedPlaybackMs = SyncData.CurrentPlaybackMs;
	SyncData.CurrentBeat = GetChartBeat();
	SyncData.bIsPlaying = CurrentPlayingId != 0 || SyncData.BPM > 0.0f;

	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;

	bHasWwisePosition = false;
	SyncStartRealSeconds = FPlatformTime::Seconds();
	FallbackAdvancedMs = 0.0f;
	MaxPlaybackJumpMs = 0.0f;
}

void UPTBWwiseRhythmSyncComponent::LogSyncSummary(const TCHAR* Reason) const
{
	if (CurrentPlayingId == 0)
	{
		return;
	}

	UE_LOG(LogWwise, Log, TEXT("[SyncCheck] Summary(%s): GotWwisePosition=%d FallbackAdvanced=%.0fms MaxJump=%.0fms"),
		Reason,
		bHasWwisePosition ? 1 : 0,
		FallbackAdvancedMs,
		MaxPlaybackJumpMs);
}

void UPTBWwiseRhythmSyncComponent::StopSync()
{
	LogSyncSummary(TEXT("Stop"));
	CurrentPlayingId = 0;
	ActiveChartData = FPTBChartData();
	ChartOffsetMs = 0.0f;
	SyncData = FPTBWwiseSyncData();
	SmoothedPlaybackMs = 0.0f;

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
	return SmoothedPlaybackMs - ChartOffsetMs + VisualOffsetMs;
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
