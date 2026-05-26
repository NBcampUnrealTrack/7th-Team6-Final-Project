#include "MiniGames/Common/PTBBaseMiniGame.h"

#include "AkComponent.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Debug/PTBLogChannels.h"
#include "Misc/Paths.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"

namespace PTBBaseMiniGameInternal
{
	float ResolveLatestNoteTimeMs(const UPTBRhythmChartAsset* InChartAsset)
	{
		if (!InChartAsset || InChartAsset->NoteEvents.IsEmpty())
		{
			return 0.0f;
		}

		return InChartAsset->NoteEvents.Last().TimeMs;
	}

	void RemoveResolvedNote(TArray<FPTBNoteEvent>& ActiveNotes, const FPTBJudgementResult& Result)
	{
		if (Result.NoteId == 0)
		{
			return;
		}

		const int32 FoundIndex = ActiveNotes.IndexOfByPredicate([&Result](const FPTBNoteEvent& Note)
		{
			if (Note.NoteId == Result.NoteId)
			{
				return true;
			}

			return false;
		});

		if (FoundIndex != INDEX_NONE)
		{
			ActiveNotes.RemoveAt(FoundIndex);
		}
	}
}

APTBBaseMiniGame::APTBBaseMiniGame()
{
 	PrimaryActorTick.bCanEverTick = true;

	AkComponent = CreateDefaultSubobject<UAkComponent>(TEXT("AkComponent"));
	SetRootComponent(AkComponent);
	RhythmConductor = CreateDefaultSubobject<UPTBRhythmConductorComponent>(TEXT("RhythmConductor"));
	RhythmSyncComponent = CreateDefaultSubobject<UPTBWwiseRhythmSyncComponent>(TEXT("RhythmSyncComponent"));
	JudgementSystem = CreateDefaultSubobject<UPTBJudgementSystem>(TEXT("JudgementSystem"));

	MiniGameId = NAME_None;
	MiniGameCode = NAME_None;
	RuleSet = nullptr;
	AudioEventSet = nullptr;
	AudioManager = nullptr;
	ChartAsset = nullptr;
	ScoreCalculator = nullptr;
	bIsInitialized = false;
	bIsRoundActive = false;
	bInputLocked = true;
	bPendingRoundFinish = false;
	bAllNotesDispatched = false;
	ActiveBGMPlayingId = 0;
}

void APTBBaseMiniGame::BeginPlay()
{
	Super::BeginPlay();

	if (!AudioManager)
	{
		AudioManager = NewObject<UPTBWwiseAudioManager>(this, TEXT("MiniGameAudioManager"));
	}

	if (!ScoreCalculator)
	{
		ScoreCalculator = NewObject<UPTBScoreCalculator>(this);
	}

	if (AudioManager)
	{
		AudioManager->SetMainAkComponent(AkComponent);
		AudioManager->OnBGMFinished.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleBGMFinished);
	}

	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->SetWwiseManager(AudioManager.Get());
	}

	if (RhythmConductor)
	{
		if (RhythmSyncComponent)
		{
			RhythmConductor->SetRhythmSyncComponent(RhythmSyncComponent);
		}

		RhythmConductor->OnNoteArm.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleNoteArm);
		RhythmConductor->OnNoteEvent.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleChartEvent);
		RhythmConductor->OnNoteCue.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleNoteCue);
		RhythmConductor->OnAllNotesPassed.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleAllNotesPassed);
	}

	if (JudgementSystem)
	{
		JudgementSystem->OnJudgementResult.AddUniqueDynamic(this, &APTBBaseMiniGame::HandleJudgementResult);
	}
}

void APTBBaseMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRoundActive || !JudgementSystem)
	{
		return;
	}

	JudgementSystem->ForceMissExpiredNotes(GetCurrentInputJudgeTimeMs());

	const bool bHasPendingNotes = !JudgementSystem->PendingNotes.IsEmpty();
	if (bPendingRoundFinish && !bHasPendingNotes)
	{
		FinishMiniGame(EPTBRoundEndReason::Completed);
		return;
	}

	if (ActiveBGMPlayingId == 0
		&& bAllNotesDispatched
		&& !bHasPendingNotes
		&& GetCurrentChartTimeMs() >= GetRoundEndChartTimeMs())
	{
		FinishMiniGame(EPTBRoundEndReason::Completed);
	}
}

void APTBBaseMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
	GameContext = Context;
	MiniGameId = Context.SessionRequest.MiniGameId;
	MiniGameCode = Context.SessionRequest.MiniGameCode;
	ActiveNoteQueue.Reset();
	RoundResult = FPTBRoundResult();
	bPendingRoundFinish = false;
	bAllNotesDispatched = false;
	ActiveBGMPlayingId = 0;
	bIsInitialized = false;
	bIsRoundActive = false;
	bInputLocked = true;

	if (!AudioManager)
	{
		AudioManager = NewObject<UPTBWwiseAudioManager>(this, TEXT("MiniGameAudioManager"));
	}

	if (!ScoreCalculator)
	{
		ScoreCalculator = NewObject<UPTBScoreCalculator>(this);
	}

	if (ScoreCalculator)
	{
		ScoreCalculator->Reset();
	}

	if (AudioManager)
	{
		AudioManager->SetMainAkComponent(AkComponent);
		AudioManager->ApplySettings(Context.UserSettings);

		if (AudioEventSet)
		{
			AudioManager->ApplyEventMapAsset(AudioEventSet);
		}
	}

	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->SetWwiseManager(AudioManager.Get());
		RhythmSyncComponent->SetUserOffsets(
			ResolveInputOffsetMs(Context),
			ResolveVisualOffsetMs(Context),
			ResolveSoundOffsetMs(Context));
	}

	PreloadAssets();

	if (ChartAsset)
	{
		TArray<FText> ChartErrors;
		if (!ChartAsset->ValidateChart(ChartErrors))
		{
			for (const FText& ChartError : ChartErrors)
			{
				UE_LOG(LogRhythm, Error, TEXT("[%s] Invalid chart: %s"), *GetNameSafe(this), *ChartError.ToString());
			}

			ChartAsset = nullptr;
		}
		else
		{
			GameContext.ChartData = ChartAsset->ChartData;
		}
	}

	if (!ChartAsset)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] ChartAsset is not ready. MiniGameId=%s"), *GetNameSafe(this), *MiniGameId.ToString());
	}

	if (JudgementSystem)
	{
		JudgementSystem->Initialize(GameContext.ChartData, 0.0f);
	}

	if (RhythmConductor && JudgementSystem)
	{
		const float InputCompensationMs = FMath::Abs(ResolveInputOffsetMs(Context));
		RhythmConductor->SetArmLeadTimeMs(JudgementSystem->HitWindowMissMs + InputCompensationMs);
	}

	BuildRuntimeState();

	bIsInitialized = true;
}

void APTBBaseMiniGame::PreloadAssets()
{
	if (ChartAsset || ChartJsonFilePath.IsEmpty())
	{
		return;
	}

	FString ResolvedPath = ChartJsonFilePath;
	if (FPaths::IsRelative(ResolvedPath))
	{
		ResolvedPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), ResolvedPath));
	}

	UPTBRhythmChartAsset* LoadedChartAsset = NewObject<UPTBRhythmChartAsset>(this, NAME_None, RF_Transient);
	TArray<FText> LoadErrors;
	if (!LoadedChartAsset->LoadFromJson(ResolvedPath, LoadErrors))
	{
		UE_LOG(LogRhythm, Error, TEXT("[%s] Failed to load chart json: %s"), *GetNameSafe(this), *ResolvedPath);

		for (const FText& LoadError : LoadErrors)
		{
			UE_LOG(LogRhythm, Error, TEXT("[%s] Chart load error: %s"), *GetNameSafe(this), *LoadError.ToString());
		}

		return;
	}

	ChartAsset = LoadedChartAsset;
	UE_LOG(LogRhythm, Log, TEXT("[%s] Loaded chart json: %s"), *GetNameSafe(this), *ResolvedPath);
}

void APTBBaseMiniGame::BuildRuntimeState()
{
}

void APTBBaseMiniGame::StartMiniGame()
{
	if (!bIsInitialized || !ChartAsset)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] StartMiniGame aborted. Initialized=%d ChartAsset=%s"), *GetNameSafe(this), bIsInitialized ? 1 : 0, *GetNameSafe(ChartAsset));
		return;
	}

	if (AudioManager)
	{
		AudioManager->SetMainAkComponent(AkComponent);
		if (AudioEventSet)
		{
			AudioManager->ApplyEventMapAsset(AudioEventSet);
		}

		if (!GameContext.ChartData.WwiseBankName.IsNone())
		{
			const bool bLoadedBank = AudioManager->LoadSoundBank(GameContext.ChartData.WwiseBankName);
			UE_LOG(LogWwise, Log, TEXT("[%s] LoadSoundBank %s -> %s"), *GetNameSafe(this), *GameContext.ChartData.WwiseBankName.ToString(), bLoadedBank ? TEXT("Success") : TEXT("Failed"));
		}
	}

	int32 PlayingId = 0;
	if (AudioManager && !GameContext.ChartData.WwiseEventName.IsNone())
	{
		PlayingId = AudioManager->PostBGMEvent(GameContext.ChartData.WwiseEventName);
		UE_LOG(LogWwise, Log, TEXT("[%s] PostBGMEvent %s -> PlayingId=%d"), *GetNameSafe(this), *GameContext.ChartData.WwiseEventName.ToString(), PlayingId);
	}
	else
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Starting without Wwise BGM event. Fallback timing will be used."), *GetNameSafe(this));
	}

	ActiveBGMPlayingId = PlayingId;
	bPendingRoundFinish = false;
	bAllNotesDispatched = ChartAsset->NoteEvents.IsEmpty();

	if (RhythmSyncComponent)
	{
		RhythmSyncComponent->SetWwiseManager(AudioManager.Get());
	}

	if (RhythmConductor)
	{
		RhythmConductor->StartConductor(ChartAsset.Get(), PlayingId, AudioManager.Get());
	}

	bIsRoundActive = true;
	bInputLocked = false;

	UE_LOG(LogRhythm, Log, TEXT("[%s] MiniGame started. Notes=%d BPM=%.2f OffsetMs=%.2f"), *GetNameSafe(this), ChartAsset->NoteEvents.Num(), GameContext.ChartData.BPM, GameContext.ChartData.OffsetMs);
}

FPTBRoundResult APTBBaseMiniGame::FinishMiniGame(EPTBRoundEndReason Reason)
{
	if (!bIsRoundActive && RoundResult.MiniGameId == MiniGameId)
	{
		return RoundResult;
	}

	bIsRoundActive = false;
	bInputLocked = true;
	bPendingRoundFinish = false;

	if (JudgementSystem)
	{
		JudgementSystem->ForceMissExpiredNotes(GetRoundEndChartTimeMs());
	}

	if (RhythmConductor)
	{
		RhythmConductor->StopConductor();
	}

	if (AudioManager && ActiveBGMPlayingId != 0 && AudioManager->IsEventPlaying(ActiveBGMPlayingId))
	{
		AudioManager->StopBGM(0.0f);
	}

	const FString ProfileIdString = GameContext.SessionRequest.ProfileId.ToString(EGuidFormats::DigitsWithHyphens);
	const FPTBMiniGameResultPayload Payload = BuildResultPayload();

	RoundResult = ScoreCalculator
		? ScoreCalculator->BuildRoundResult(ProfileIdString, MiniGameId, GameContext.SessionRequest.Difficulty, Payload)
		: FPTBRoundResult();
	RoundResult.PlayMode = GameContext.SessionRequest.PlayMode;

	ActiveBGMPlayingId = 0;

	UE_LOG(LogRhythm, Log, TEXT("[%s] MiniGame finished. Reason=%d Score=%d HP=%d P=%d G=%d M=%d"),
		*GetNameSafe(this),
		static_cast<int32>(Reason),
		RoundResult.Score,
		RoundResult.HighPerfectCount,
		RoundResult.PerfectCount,
		RoundResult.GoodCount,
		RoundResult.MissCount);

	return RoundResult;
}

void APTBBaseMiniGame::PauseMiniGame()
{
	if (!bIsRoundActive)
	{
		return;
	}

	bInputLocked = true;
	if (RhythmConductor)
	{
		RhythmConductor->PauseConductor();
	}

	if (AudioManager)
	{
		AudioManager->PauseBGM();
	}
}

void APTBBaseMiniGame::ResumeMiniGame()
{
	if (!bIsRoundActive)
	{
		return;
	}

	if (RhythmConductor)
	{
		RhythmConductor->ResumeConductor();
	}

	if (AudioManager)
	{
		AudioManager->ResumeBGM();
	}

	bInputLocked = false;
}

void APTBBaseMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	UE_LOG(LogRhythm, Verbose, TEXT("[%s] NoteEvent NoteId=%d Action=%d TimeMs=%.2f Beat=%.2f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		Note.TimeMs,
		Note.BeatTime);
}

void APTBBaseMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	ActiveNoteQueue.Add(Note);

	if (JudgementSystem)
	{
		JudgementSystem->RegisterNoteEvent(Note);
	}

	UE_LOG(LogRhythm, Verbose, TEXT("[%s] NoteArm NoteId=%d Action=%d ArmLead=%.2f TimeMs=%.2f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		RhythmConductor ? RhythmConductor->ArmLeadTimeMs : 0.0f,
		Note.TimeMs);
}

void APTBBaseMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	UE_LOG(LogRhythm, Verbose, TEXT("[%s] NoteCue NoteId=%d Action=%d CueBeat=%.2f TimeMs=%.2f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		Note.BeatTime,
		Note.TimeMs);
}

void APTBBaseMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
	if (!CanAcceptInput())
	{
		return;
	}

	const float ResolvedTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();
	EvaluateInput(Action, ResolvedTimeMs);
}

FPTBJudgementResult APTBBaseMiniGame::EvaluateInput(EPTBActionType Action, float TimeMs)
{
	if (!JudgementSystem)
	{
		return FPTBJudgementResult();
	}

	const FPTBJudgementResult Result = JudgementSystem->EvaluateInput(Action, TimeMs);
	UE_LOG(LogRhythm, Verbose, TEXT("[%s] EvaluateInput Action=%d TimeMs=%.2f -> Judgement=%d Delta=%.2f"),
		*GetNameSafe(this),
		static_cast<int32>(Action),
		TimeMs,
		static_cast<int32>(Result.JudgementType),
		Result.DeltaMs);
	return Result;
}

void APTBBaseMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	PTBBaseMiniGameInternal::RemoveResolvedNote(ActiveNoteQueue, Result);

	if (ScoreCalculator)
	{
		ScoreCalculator->AddJudgementScore(Result);
	}

	if (AudioManager)
	{
		AudioManager->PostJudgementEvent(Result.JudgementType, this);
	}

	PlayJudgementFeedback(Result);
}

void APTBBaseMiniGame::PlayJudgementFeedback(const FPTBJudgementResult& Result)
{
	UE_LOG(LogRhythm, Log, TEXT("[%s] Judgement NoteId=%d Action=%d Type=%d Delta=%.2f ScoreDelta=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		Result.DeltaMs,
		Result.ScoreDelta);
}

FPTBMiniGameResultPayload APTBBaseMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload;
	Payload.PayloadType = TEXT("BaseMiniGame");
	Payload.StringValues.Add(TEXT("MiniGameCode"), MiniGameCode.ToString());
	Payload.StringValues.Add(TEXT("ChartId"), GameContext.ChartData.ChartId.ToString());
	Payload.IntValues.Add(TEXT("ActiveNotesRemaining"), ActiveNoteQueue.Num());

	if (ScoreCalculator)
	{
		Payload.IntValues.Add(TEXT("FinalCombo"), ScoreCalculator->ComboCount);
		Payload.IntValues.Add(TEXT("MaxCombo"), ScoreCalculator->MaxCombo);
	}

	return Payload;
}

void APTBBaseMiniGame::RequestWwiseEvent(FName EventKey, AActor* Target)
{
	if (!AudioManager)
	{
		return;
	}

	AudioManager->PostSFXEvent(EventKey, Target ? Target : this);
}

bool APTBBaseMiniGame::CanAcceptInput() const
{
	return bIsInitialized && bIsRoundActive && !bInputLocked && JudgementSystem != nullptr && ChartAsset != nullptr;
}

TMap<FKey, EPTBActionType> APTBBaseMiniGame::GetActionMapping() const
{
	return {
		{ EKeys::A, EPTBActionType::ActionA },
		{ EKeys::S, EPTBActionType::ActionB },
		{ EKeys::D, EPTBActionType::ActionC },
		{ EKeys::F, EPTBActionType::ActionD },
		{ EKeys::G, EPTBActionType::ActionE }
	};
}

float APTBBaseMiniGame::ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const
{
	return Context.UserSettings.JudgementOffsetMs + Context.UserSettings.InputLatencyMs;
}

float APTBBaseMiniGame::ResolveVisualOffsetMs(const FPTBMiniGameContext& Context) const
{
	return 0.0f;
}

float APTBBaseMiniGame::ResolveSoundOffsetMs(const FPTBMiniGameContext& Context) const
{
	return 0.0f;
}

float APTBBaseMiniGame::GetCurrentChartTimeMs() const
{
	if (RhythmSyncComponent)
	{
		return RhythmSyncComponent->GetChartTimeMs();
	}

	if (RhythmConductor)
	{
		return RhythmConductor->GetCurrentMusicTimeMs() - GameContext.ChartData.OffsetMs;
	}

	return 0.0f;
}

float APTBBaseMiniGame::GetCurrentInputJudgeTimeMs() const
{
	if (RhythmSyncComponent)
	{
		return RhythmSyncComponent->GetInputJudgeTimeMs();
	}

	return GetCurrentChartTimeMs() + ResolveInputOffsetMs(GameContext);
}

float APTBBaseMiniGame::GetRoundEndChartTimeMs() const
{
	const float SongLengthMs = FMath::Max(0.0f, GameContext.ChartData.SongLengthMs);
	const float LatestNoteTimeMs = PTBBaseMiniGameInternal::ResolveLatestNoteTimeMs(ChartAsset.Get());
	const float MissGraceMs = JudgementSystem ? JudgementSystem->HitWindowMissMs : 0.0f;
	return FMath::Max(SongLengthMs, LatestNoteTimeMs + MissGraceMs);
}

void APTBBaseMiniGame::HandleBGMFinished(int32 PlayingId)
{
	if (!bIsRoundActive || (ActiveBGMPlayingId != 0 && ActiveBGMPlayingId != PlayingId))
	{
		return;
	}

	ActiveBGMPlayingId = 0;

	if (JudgementSystem)
	{
		JudgementSystem->ForceMissExpiredNotes(GetRoundEndChartTimeMs());
	}

	bPendingRoundFinish = JudgementSystem && !JudgementSystem->PendingNotes.IsEmpty();
	if (!bPendingRoundFinish)
	{
		FinishMiniGame(EPTBRoundEndReason::Completed);
	}
}

void APTBBaseMiniGame::HandleAllNotesPassed()
{
	bAllNotesDispatched = true;

	if (ActiveBGMPlayingId == 0 && (!JudgementSystem || JudgementSystem->PendingNotes.IsEmpty()))
	{
		FinishMiniGame(EPTBRoundEndReason::Completed);
	}
}
