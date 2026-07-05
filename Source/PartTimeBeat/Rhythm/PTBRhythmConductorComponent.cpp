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

	int32 ResolveBeatsPerBarForBeat(float Beat, const FPTBChartData& ChartData, int32 FallbackBeatsPerBar)
	{
		const int32 EventCount = FMath::Min3(
			ChartData.TimeSignatureChangeBeats.Num(),
			ChartData.TimeSignatureChangeNumerators.Num(),
			ChartData.TimeSignatureChangeDenominators.Num());

		int32 ResolvedBeatsPerBar = FMath::Max(MinBeatsPerBar, FallbackBeatsPerBar);
		if (ChartData.TimeSignatureNumerator > 0)
		{
			ResolvedBeatsPerBar = ChartData.TimeSignatureNumerator;
		}

		for (int32 Index = 0; Index < EventCount; ++Index)
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
	CuePreRollTimeMs = 0.0f;
	CuePreRollElapsedMs = 0.0f;
	bIsPlaying = false;
	bIsPaused = false;
	bCuePreRollOnly = false;
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

	if (bCuePreRollOnly)
	{
		if (!ChartAsset)
		{
			return;
		}

		CuePreRollElapsedMs = FMath::Min(
			FMath::Max(0.0f, CuePreRollTimeMs),
			CuePreRollElapsedMs + DeltaTime * PTBRhythmConductorInternal::MillisecondsPerSecond);

		const float VisualChartTimeMs = CuePreRollElapsedMs - FMath::Max(0.0f, CuePreRollTimeMs);
		CurrentTimeMs = VisualChartTimeMs + ChartOffsetMs;
		CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(VisualChartTimeMs, ChartData.BPM);
		const float CueBeat = CurrentBeat;

		const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;
		const float CueLeadBeats = FMath::Max(0.0f, LookAheadBeats);
		const float EffectiveCueLeadTimeMs = FMath::Max(0.0f, CueLeadTimeMs);
		const float EffectiveArmLeadTimeMs = FMath::Max(0.0f, ArmLeadTimeMs);

		while (Notes.IsValidIndex(NextArmIndex) && Notes[NextArmIndex].TimeMs - EffectiveArmLeadTimeMs <= VisualChartTimeMs)
		{
			OnNoteArm.Broadcast(Notes[NextArmIndex]);
			++NextArmIndex;
		}

		while (Notes.IsValidIndex(NextCueIndex)
			&& ((CueLeadTimeMode == EPTBCueLeadTimeMode::MS
				&& Notes[NextCueIndex].TimeMs - EffectiveCueLeadTimeMs <= VisualChartTimeMs)
				|| (CueLeadTimeMode == EPTBCueLeadTimeMode::Beat
					&& Notes[NextCueIndex].BeatTime - CueLeadBeats <= CueBeat)))
		{
			OnNoteCue.Broadcast(Notes[NextCueIndex]);
			++NextCueIndex;
		}

		return;
	}

	float ChartTimeMs = 0.0f;
	float VisualChartTimeMs = 0.0f;
	float CueBeat = 0.0f;

	if (RhythmSyncComponent)
	{
		CurrentTimeMs = RhythmSyncComponent->GetRawPlaybackTimeMs();
		CurrentBeat = RhythmSyncComponent->GetChartBeat();
		ChartTimeMs = RhythmSyncComponent->GetChartTimeMs();
		VisualChartTimeMs = RhythmSyncComponent->GetVisualChartTimeMs();
		CueBeat = RhythmSyncComponent->GetVisualBeat();
	}
	else if (AudioManager && WwisePlayingId != 0)
	{
		CurrentTimeMs = AudioManager->GetPlaybackPositionMs(WwisePlayingId);
		ChartTimeMs = CurrentTimeMs - ChartOffsetMs;
		VisualChartTimeMs = ChartTimeMs;
		CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(ChartTimeMs, ChartData.BPM);
		CueBeat = CurrentBeat;
	}
	else
	{
		CurrentTimeMs += DeltaTime * PTBRhythmConductorInternal::MillisecondsPerSecond;
		ChartTimeMs = CurrentTimeMs - ChartOffsetMs;
		VisualChartTimeMs = ChartTimeMs;
		CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(ChartTimeMs, ChartData.BPM);
		CueBeat = CurrentBeat;
	}

	const int32 BeatTickIndex = FMath::FloorToInt(CurrentBeat);
	if (BeatTickIndex > LastBeatTickIndex)
	{
		LastBeatTickIndex = BeatTickIndex;
		OnBeatTick.Broadcast(CurrentBeat);
	}

	const int32 SafeBeatsPerBar = RhythmSyncComponent
		? RhythmSyncComponent->GetCurrentBeatsPerBar()
		: PTBRhythmConductorInternal::ResolveBeatsPerBarForBeat(CurrentBeat, ChartData, BeatsPerBar);
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
	const float EffectiveCueLeadTimeMs = FMath::Max(0.0f, CueLeadTimeMs);
	const float EffectiveArmLeadTimeMs = FMath::Max(0.0f, ArmLeadTimeMs);

	while (Notes.IsValidIndex(NextArmIndex) && Notes[NextArmIndex].TimeMs - EffectiveArmLeadTimeMs <= ChartTimeMs)
	{
		OnNoteArm.Broadcast(Notes[NextArmIndex]);
		++NextArmIndex;
	}

	while (Notes.IsValidIndex(NextCueIndex)
		&& ((CueLeadTimeMode == EPTBCueLeadTimeMode::MS
			&& Notes[NextCueIndex].TimeMs - EffectiveCueLeadTimeMs <= VisualChartTimeMs)
			|| (CueLeadTimeMode == EPTBCueLeadTimeMode::Beat
				&& Notes[NextCueIndex].BeatTime - CueLeadBeats <= CueBeat)))
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
	if (bCuePreRollOnly && ChartAsset == InChartAsset && ChartAsset)
	{
		const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;
		const float CueLeadBeats = FMath::Max(0.0f, LookAheadBeats);
		const float EffectiveCueLeadTimeMs = FMath::Max(0.0f, CueLeadTimeMs);
		const float VisualChartTimeMs = 0.0f;
		const float CueBeat = 0.0f;

		while (Notes.IsValidIndex(NextCueIndex)
			&& ((CueLeadTimeMode == EPTBCueLeadTimeMode::MS
				&& Notes[NextCueIndex].TimeMs - EffectiveCueLeadTimeMs <= VisualChartTimeMs)
				|| (CueLeadTimeMode == EPTBCueLeadTimeMode::Beat
					&& Notes[NextCueIndex].BeatTime - CueLeadBeats <= CueBeat)))
		{
			OnNoteCue.Broadcast(Notes[NextCueIndex]);
			++NextCueIndex;
		}
	}

	const int32 InitialCueIndex = bCuePreRollOnly ? NextCueIndex : 0;
	const int32 InitialArmIndex = bCuePreRollOnly ? NextArmIndex : 0;

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
	NextCueIndex = InitialCueIndex;
	NextArmIndex = InitialArmIndex;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = ChartData.OffsetMs;
	CuePreRollTimeMs = 0.0f;
	CuePreRollElapsedMs = 0.0f;
	if (ChartData.TimeSignatureNumerator > 0)
	{
		BeatsPerBar = FMath::Max(PTBRhythmConductorInternal::MinBeatsPerBar, ChartData.TimeSignatureNumerator);
	}
	bIsPlaying = InChartAsset != nullptr;
	bIsPaused = false;
	bCuePreRollOnly = false;
	bAllNotesPassed = false;

	if (RhythmSyncComponent && bIsPlaying)
	{
		if (AudioManager)
		{
			RhythmSyncComponent->SetWwiseManager(AudioManager.Get());
		}

		RhythmSyncComponent->SetBeatsPerBar(BeatsPerBar);
		RhythmSyncComponent->StartSync(WwisePlayingId, ChartData);
	}

	if (bIsPlaying && ChartAsset)
	{
		const float StartChartTimeMs = RhythmSyncComponent
			? RhythmSyncComponent->GetChartTimeMs()
			: CurrentTimeMs - ChartOffsetMs;
		const float StartVisualChartTimeMs = RhythmSyncComponent
			? RhythmSyncComponent->GetVisualChartTimeMs()
			: StartChartTimeMs;
		const float StartCueBeat = RhythmSyncComponent
			? RhythmSyncComponent->GetVisualBeat()
			: PTBRhythmConductorInternal::CalculateBeat(StartVisualChartTimeMs, ChartData.BPM);
		const TArray<FPTBNoteEvent>& Notes = ChartAsset->NoteEvents;
		const float CueLeadBeats = FMath::Max(0.0f, LookAheadBeats);
		const float EffectiveCueLeadTimeMs = FMath::Max(0.0f, CueLeadTimeMs);
		const float EffectiveArmLeadTimeMs = FMath::Max(0.0f, ArmLeadTimeMs);

		while (Notes.IsValidIndex(NextArmIndex) && Notes[NextArmIndex].TimeMs - EffectiveArmLeadTimeMs <= StartChartTimeMs)
		{
			OnNoteArm.Broadcast(Notes[NextArmIndex]);
			++NextArmIndex;
		}

		while (Notes.IsValidIndex(NextCueIndex)
			&& ((CueLeadTimeMode == EPTBCueLeadTimeMode::MS
				&& Notes[NextCueIndex].TimeMs - EffectiveCueLeadTimeMs <= StartVisualChartTimeMs)
				|| (CueLeadTimeMode == EPTBCueLeadTimeMode::Beat
					&& Notes[NextCueIndex].BeatTime - CueLeadBeats <= StartCueBeat)))
		{
			OnNoteCue.Broadcast(Notes[NextCueIndex]);
			++NextCueIndex;
		}

		while (Notes.IsValidIndex(NextNoteIndex) && Notes[NextNoteIndex].TimeMs <= StartChartTimeMs)
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
}

void UPTBRhythmConductorComponent::StartCuePreRoll(UPTBRhythmChartAsset* InChartAsset, float PreRollTimeMs)
{
	ChartAsset = InChartAsset;
	AudioManager = nullptr;
	ChartData = InChartAsset ? InChartAsset->ChartData : FPTBChartData();
	WwisePlayingId = 0;
	NextNoteIndex = 0;
	NextCueIndex = 0;
	NextArmIndex = 0;
	LastBeatTickIndex = -1;
	LastBarTickIndex = -1;
	ChartOffsetMs = ChartData.OffsetMs;
	CuePreRollTimeMs = FMath::Max(0.0f, PreRollTimeMs);
	CuePreRollElapsedMs = 0.0f;
	if (ChartData.TimeSignatureNumerator > 0)
	{
		BeatsPerBar = FMath::Max(PTBRhythmConductorInternal::MinBeatsPerBar, ChartData.TimeSignatureNumerator);
	}
	CurrentTimeMs = ChartOffsetMs - CuePreRollTimeMs;
	CurrentBeat = PTBRhythmConductorInternal::CalculateBeat(-CuePreRollTimeMs, ChartData.BPM);
	bIsPlaying = InChartAsset != nullptr && CuePreRollTimeMs > 0.0f;
	bIsPaused = false;
	bCuePreRollOnly = bIsPlaying;
	bAllNotesPassed = false;

	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->StopSync();
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
	CuePreRollTimeMs = 0.0f;
	CuePreRollElapsedMs = 0.0f;
	bIsPlaying = false;
	bIsPaused = false;
	bCuePreRollOnly = false;
	bAllNotesPassed = false;
}

float UPTBRhythmConductorComponent::GetCurrentMusicTimeMs() const
{
	return CurrentTimeMs;
}

float UPTBRhythmConductorComponent::GetCurrentChartTimeMs() const
{
	return CurrentTimeMs - ChartOffsetMs;
}

float UPTBRhythmConductorComponent::GetCurrentBeat() const
{
	return CurrentBeat;
}

bool UPTBRhythmConductorComponent::IsCuePreRollOnly() const
{
	return bCuePreRollOnly;
}

void UPTBRhythmConductorComponent::SetArmLeadTimeMs(float InArmLeadTimeMs)
{
	ArmLeadTimeMs = FMath::Max(0.0f, InArmLeadTimeMs);
}

void UPTBRhythmConductorComponent::SetLookAheadBeats(float InLookAheadBeats)
{
	LookAheadBeats = FMath::Max(0.0f, InLookAheadBeats);
}

void UPTBRhythmConductorComponent::SetCueLeadTimeMode(EPTBCueLeadTimeMode InCueLeadTimeMode)
{
	CueLeadTimeMode = InCueLeadTimeMode;
}

void UPTBRhythmConductorComponent::SetCueLeadTimeMs(float InCueLeadTimeMs)
{
	CueLeadTimeMs = FMath::Max(0.0f, InCueLeadTimeMs);
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
