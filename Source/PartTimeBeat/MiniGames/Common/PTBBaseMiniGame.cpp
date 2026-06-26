#include "MiniGames/Common/PTBBaseMiniGame.h"

#include "AkComponent.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Core/PTBGameModeBase.h"
#include "Debug/PTBLogChannels.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "MiniGames/Common/UI/PTBMiniGameLoadingWidget.h"
#include "Components/InputComponent.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"

namespace PTBBaseMiniGameInternal
{
	constexpr float RequiredHoldRatio = 0.75f;

	float ResolveLatestNoteTimeMs(const UPTBRhythmChartAsset* InChartAsset)
	{
		if (!InChartAsset || InChartAsset->NoteEvents.IsEmpty())
		{
			return 0.0f;
		}

		float LatestTimeMs = 0.0f;
		for (const FPTBNoteEvent& Note : InChartAsset->NoteEvents)
		{
			LatestTimeMs = FMath::Max(LatestTimeMs, Note.TimeMs);
			if (Note.bIsLongNote)
			{
				LatestTimeMs = FMath::Max(LatestTimeMs, Note.ReleaseTimeMs);
			}
		}

		return LatestTimeMs;
	}

	void RemoveResolvedNote(TArray<FPTBNoteEvent>& ActiveNotes, const FPTBJudgementResult& Result)
	{
		if (Result.NoteId == 0)
		{
			return;
		}

		for (int32 Index = 0; Index < ActiveNotes.Num(); ++Index)
		{
			if (ActiveNotes[Index].NoteId == Result.NoteId)
			{
				ActiveNotes.RemoveAt(Index);
				return;
			}
		}
	}

	FPTBJudgementResult MakeEarlyReleaseResult(const FPTBNoteEvent& Note, float ReleaseTimeMs, float RequiredHoldUntilTimeMs)
	{
		FPTBJudgementResult Result;
		Result.NoteId = Note.NoteId;
		Result.ActionType = Note.ActionType;
		Result.InputActionType = Note.ActionType;
		Result.JudgementType = EPTBJudgementType::Miss;
		Result.Reason = EPTBJudgementReason::EarlyRelease;
		Result.ChartTimeMs = RequiredHoldUntilTimeMs;
		Result.InputTimeMs = ReleaseTimeMs;
		Result.DeltaMs = ReleaseTimeMs - RequiredHoldUntilTimeMs;
		Result.ScoreDelta = 0;
		Result.bBreaksCombo = true;
		return Result;
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
	LoadingWidgetClass = nullptr;
	LoadingWidgetInstance = nullptr;
	bIsInitialized = false;
	bIsReadyToStart = false;
	bIsRoundActive = false;
	bStartSequenceActive = false;
	bFinishSequenceActive = false;
	bInputLocked = true;
	bPendingRoundFinish = false;
	bPendingRoundFailed = false;
	bAllNotesDispatched = false;
	ActiveBGMPlayingId = 0;
	PendingEndReason = EPTBRoundEndReason::Completed;
}

void APTBBaseMiniGame::BeginPlay()
{
	Super::BeginPlay();

	// GameMode에 자신을 등록하고 결과 델리게이트를 바인딩.
	// StartGameFlow 경유 스폰과 레벨 직접 배치 두 경우를 모두 처리.
	if (APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>())
	{
		if (!GM->ActiveMiniGame)
		{
			GM->ActiveMiniGame = this;
			OnMiniGameStarted.AddUniqueDynamic(GM, &APTBGameModeBase::HandleMiniGameStarted);
			OnMiniGameFinished.AddUniqueDynamic(GM, &APTBGameModeBase::HandleMiniGameFinished);
		}
		else if (GM->ActiveMiniGame == this)
		{
			OnMiniGameStarted.AddUniqueDynamic(GM, &APTBGameModeBase::HandleMiniGameStarted);
			OnMiniGameFinished.AddUniqueDynamic(GM, &APTBGameModeBase::HandleMiniGameFinished);
		}
	}

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

	const float CurrentInputTimeMs = GetCurrentInputJudgeTimeMs();
	JudgementSystem->ForceMissExpiredNotes(CurrentInputTimeMs);
	ResolveSatisfiedHoldInputs(CurrentInputTimeMs);

	if (bPendingRoundFailed)
	{
		FinishMiniGame(EPTBRoundEndReason::Failed);
		return;
	}

	const bool bHasPendingNotes = !JudgementSystem->PendingNotes.IsEmpty();
	const bool bHasActiveHolds = !ActiveHoldStates.IsEmpty();
	if (bPendingRoundFinish && !bHasPendingNotes && !bHasActiveHolds)
	{
		FinishMiniGame(EPTBRoundEndReason::Completed);
		return;
	}

	if (ActiveBGMPlayingId == 0
		&& bAllNotesDispatched
		&& !bHasPendingNotes
		&& !bHasActiveHolds
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
	ActiveHoldStates.Reset();
	EmptyInputActionLockUntilTimeMs.Reset();
	RoundResult = FPTBRoundResult();
	bPendingRoundFinish = false;
	bPendingRoundFailed = false;
	bAllNotesDispatched = false;
	ActiveBGMPlayingId = 0;
	bIsInitialized = false;
	bIsReadyToStart = false;
	bIsRoundActive = false;
	bStartSequenceActive = false;
	bFinishSequenceActive = false;
	bInputLocked = true;
	PendingEndReason = EPTBRoundEndReason::Completed;
	GetWorldTimerManager().ClearTimer(IntroTimerHandle);
	GetWorldTimerManager().ClearTimer(OutroTimerHandle);

	HandleLoadingStarted();

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

	if (RuleSet)
	{
		AudioEventSet = RuleSet->AudioEventSet;
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

			HideLoadingWidget();
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
		HideLoadingWidget();
		return;
	}

	if (JudgementSystem)
	{
		JudgementSystem->Initialize(GameContext.ChartData, 0.0f);
	}

	if (JudgementSystem && RuleSet && RuleSet->bUseJudgementWindowOverride)
	{
		JudgementSystem->HitWindowHighPerfectMs = RuleSet->HitWindowHighPerfectMsOverride;
		JudgementSystem->HitWindowPerfectMs = RuleSet->HitWindowPerfectMsOverride;
		JudgementSystem->HitWindowGoodMs = RuleSet->HitWindowGoodMsOverride;
		JudgementSystem->HitWindowMissMs = RuleSet->HitWindowMissMsOverride;
	}

	if (JudgementSystem && RuleSet)
	{
		JudgementSystem->bMissNoteOnWrongInput = RuleSet->bMissNoteOnWrongInput;
		JudgementSystem->bSupportsSimultaneousInputs = RuleSet->bSupportsSimultaneousInputs;
		JudgementSystem->SimultaneousNoteToleranceMs = RuleSet->SimultaneousNoteToleranceMs;
	}

	if (RhythmConductor && JudgementSystem)
	{
		const float InputCompensationMs = FMath::Abs(ResolveInputOffsetMs(Context));
		RhythmConductor->SetArmLeadTimeMs(JudgementSystem->HitWindowMissMs + InputCompensationMs);
	}

	ApplyRuleSet();
	PreloadAudioAssets();
	BuildRuntimeState();

	bIsInitialized = true;
	bIsReadyToStart = true;
	HandleReadyToStart();
}

void APTBBaseMiniGame::PreloadAssets()
{
	if (!ChartAsset && RuleSet)
	{
		ChartAsset = RuleSet->ResolveChartAsset(GameContext.SessionRequest.Difficulty);
	}

	if (ChartAsset)
	{
		const bool bShouldLoadSourceJson = !ChartAsset->SourceJsonFilePath.IsEmpty();

		if (bShouldLoadSourceJson)
		{
			TArray<FText> LoadErrors;
			if (!ChartAsset->LoadFromSourceJson(LoadErrors))
			{
				UE_LOG(LogRhythm, Error, TEXT("[%s] Failed to load chart asset source json: %s"), *GetNameSafe(this), *GetNameSafe(ChartAsset.Get()));

				for (const FText& LoadError : LoadErrors)
				{
					UE_LOG(LogRhythm, Error, TEXT("[%s] Chart asset load error: %s"), *GetNameSafe(this), *LoadError.ToString());
				}
			}
		}

		return;
	}

	UE_LOG(LogRhythm, Warning, TEXT("[%s] ChartAsset is required. Configure RuleSet.ChartAssetsByDifficulty or ChartAsset."), *GetNameSafe(this));
}

void APTBBaseMiniGame::BuildRuntimeState()
{
}

void APTBBaseMiniGame::PreloadAudioAssets()
{
	if (!AudioManager)
	{
		return;
	}

	if (!GameContext.ChartData.WwiseEventName.IsNone())
	{
		UE_LOG(LogWwise, Log, TEXT("[%s] Wwise BGM event ready: %s"),
			*GetNameSafe(this),
			*GameContext.ChartData.WwiseEventName.ToString());
	}
}

void APTBBaseMiniGame::ApplyRuleSet()
{
	if (!RuleSet)
	{
		return;
	}

	if (MiniGameId.IsNone())
	{
		MiniGameId = RuleSet->MiniGameId;
	}

	if (MiniGameCode.IsNone())
	{
		MiniGameCode = RuleSet->MiniGameCode;
	}

	if (DisplayName.IsEmpty())
	{
		DisplayName = RuleSet->DisplayName;
	}

	if (RhythmConductor)
	{
		RhythmConductor->SetLookAheadBeats(RuleSet->LookAheadBeats);
		RhythmConductor->SetCueLeadTimeMode(RuleSet->CueLeadTimeMode);
		RhythmConductor->SetCueLeadTimeMs(RuleSet->CueLeadTimeMs);

		if (RuleSet->bUseArmLeadTimeOverride)
		{
			RhythmConductor->SetArmLeadTimeMs(RuleSet->ArmLeadTimeMsOverride);
		}
	}
}

void APTBBaseMiniGame::HandleLoadingStarted()
{
	ShowLoadingWidget();

	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->SetLoadingState();
	}
}

void APTBBaseMiniGame::HandleReadyToStart()
{
	if (LoadingWidgetInstance)
	{
		ApplyGameAndUIInputMode();
		LoadingWidgetInstance->SetReadyToStartState();
	}

	if (APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		EnableInput(PlayerController);
		if (InputComponent)
		{
			InputComponent->BindKey(EKeys::AnyKey, IE_Pressed, this, &APTBBaseMiniGame::HandleStartInput);
		}
	}

	OnMiniGameReadyToStart.Broadcast();
}

void APTBBaseMiniGame::ShowLoadingWidget()
{
	if (LoadingWidgetInstance || !LoadingWidgetClass)
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	LoadingWidgetInstance = CreateWidget<UPTBMiniGameLoadingWidget>(PlayerController, LoadingWidgetClass);
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->InitializeLoadingWidget(this);
		LoadingWidgetInstance->AddToViewport();
	}
}

void APTBBaseMiniGame::HideLoadingWidget()
{
	if (!LoadingWidgetInstance)
	{
		return;
	}

	LoadingWidgetInstance->RemoveFromParent();
	LoadingWidgetInstance = nullptr;
}

void APTBBaseMiniGame::ApplyGameOnlyInputMode()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameOnly InputMode;
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(false);
}


void APTBBaseMiniGame::ApplyGameAndUIInputMode()
{
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	if (LoadingWidgetInstance)
	{
		InputMode.SetWidgetToFocus(LoadingWidgetInstance->TakeWidget());
	}
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}

void APTBBaseMiniGame::StartMiniGame()
{
	if (bStartSequenceActive || bIsRoundActive || bFinishSequenceActive)
	{
		return;
	}

	if (!bIsInitialized || !bIsReadyToStart || !ChartAsset)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] StartMiniGame aborted. Initialized=%d Ready=%d ChartAsset=%s"),
			*GetNameSafe(this),
			bIsInitialized ? 1 : 0,
			bIsReadyToStart ? 1 : 0,
			*GetNameSafe(ChartAsset));
		return;
	}

	HideLoadingWidget();
	DisableInput(GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr);
	ApplyGameOnlyInputMode();
	bStartSequenceActive = true;
	bInputLocked = true;
	OnMiniGameStarted.Broadcast();
	ReceiveIntroStarted();

	const float IntroDelaySeconds = RuleSet && RuleSet->bUseIntroTime
		? FMath::Max(0.0f, RuleSet->IntroTimeMs) / 1000.0f
		: 0.0f;
	if (IntroDelaySeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(IntroTimerHandle, this, &APTBBaseMiniGame::BeginGameplaySequence, IntroDelaySeconds, false);
		return;
	}

	BeginGameplaySequence();
}

void APTBBaseMiniGame::BeginGameplaySequence()
{
	GetWorldTimerManager().ClearTimer(IntroTimerHandle);
	if (!bStartSequenceActive || bIsRoundActive || bFinishSequenceActive)
	{
		return;
	}

	if (AudioManager)
	{
		AudioManager->SetMainAkComponent(AkComponent);
		if (AudioEventSet)
		{
			AudioManager->ApplyEventMapAsset(AudioEventSet);
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
	bPendingRoundFailed = false;
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
	bStartSequenceActive = false;
	bInputLocked = false;
	OnMiniGameGameplayStarted.Broadcast();
	ReceiveGameplayStarted();

	UE_LOG(LogRhythm, Log, TEXT("[%s] MiniGame started. Notes=%d BPM=%.2f OffsetMs=%.2f"), *GetNameSafe(this), ChartAsset->NoteEvents.Num(), GameContext.ChartData.BPM, GameContext.ChartData.OffsetMs);
}

void APTBBaseMiniGame::RequestStartMiniGame()
{
	if (!bIsReadyToStart)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Start request ignored. MiniGame is not ready."), *GetNameSafe(this));
		return;
	}

	StartMiniGame();
}

void APTBBaseMiniGame::HandleStartInput()
{
	RequestStartMiniGame();
}

FPTBRoundResult APTBBaseMiniGame::FinishMiniGame(EPTBRoundEndReason Reason)
{
	if (bFinishSequenceActive || (!bIsRoundActive && !bStartSequenceActive && RoundResult.MiniGameId == MiniGameId))
	{
		return RoundResult;
	}

	bFinishSequenceActive = true;
	PendingEndReason = Reason;
	bIsRoundActive = false;
	bStartSequenceActive = false;
	bIsReadyToStart = false;
	bInputLocked = true;
	bPendingRoundFinish = false;
	bPendingRoundFailed = false;
	GetWorldTimerManager().ClearTimer(IntroTimerHandle);
	ApplyGameAndUIInputMode();

	if (Reason != EPTBRoundEndReason::Aborted && JudgementSystem)
	{
		JudgementSystem->ForceMissExpiredNotes(GetRoundEndChartTimeMs());
	}

	if (Reason != EPTBRoundEndReason::Aborted)
	{
		const float CurrentInputTimeMs = GetCurrentInputJudgeTimeMs();
		for (int32 Index = ActiveHoldStates.Num() - 1; Index >= 0; --Index)
		{
			if (CurrentInputTimeMs >= ActiveHoldStates[Index].RequiredHoldUntilTimeMs)
			{
				ConfirmActiveHold(Index);
			}
			else
			{
				FailActiveHoldEarlyRelease(Index, CurrentInputTimeMs);
			}
		}
	}
	else
	{
		ActiveHoldStates.Reset();
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

	// 강제 종료(HP 소진 등)의 경우 점수 기반 Grade를 무시하고 Fail로 덮어씀
	if (Reason == EPTBRoundEndReason::Failed)
	{
		RoundResult.Grade = EPTBGradeType::Fail;
	}

	ActiveBGMPlayingId = 0;

	UE_LOG(LogRhythm, Log, TEXT("[%s] MiniGame finished. Reason=%d Score=%d HP=%d P=%d G=%d M=%d"),
		*GetNameSafe(this),
		static_cast<int32>(Reason),
		RoundResult.Score,
		RoundResult.HighPerfectCount,
		RoundResult.PerfectCount,
		RoundResult.GoodCount,
		RoundResult.MissCount);

	BeginOutroSequence(Reason);

	return RoundResult;
}

void APTBBaseMiniGame::BeginOutroSequence(EPTBRoundEndReason Reason)
{
	if (Reason == EPTBRoundEndReason::Aborted)
	{
		CompleteFinishSequence();
		return;
	}

	OnMiniGameOutroStarted.Broadcast(RoundResult, Reason);
	ReceiveOutroStarted(RoundResult, Reason);

	const float OutroDelaySeconds = RuleSet && RuleSet->bUseOutroTime
		? FMath::Max(0.0f, RuleSet->OutroTimeMs) / 1000.0f
		: 0.0f;
	if (OutroDelaySeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(OutroTimerHandle, this, &APTBBaseMiniGame::CompleteFinishSequence, OutroDelaySeconds, false);
		return;
	}

	CompleteFinishSequence();
}

void APTBBaseMiniGame::CompleteFinishSequence()
{
	GetWorldTimerManager().ClearTimer(OutroTimerHandle);

	if (PendingEndReason != EPTBRoundEndReason::Aborted)
	{
		OnMiniGameOutroFinished.Broadcast(RoundResult, PendingEndReason);
		ReceiveOutroFinished(RoundResult, PendingEndReason);
	}

	bFinishSequenceActive = false;
	OnMiniGameFinished.Broadcast(RoundResult);
}

void APTBBaseMiniGame::PauseMiniGame()
{
	if (!bIsRoundActive && !bStartSequenceActive && !bFinishSequenceActive)
	{
		return;
	}

	bInputLocked = true;
	ApplyGameAndUIInputMode();
	GetWorldTimerManager().PauseTimer(IntroTimerHandle);
	GetWorldTimerManager().PauseTimer(OutroTimerHandle);
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
	if (!bIsRoundActive && !bStartSequenceActive && !bFinishSequenceActive)
	{
		return;
	}

	GetWorldTimerManager().UnPauseTimer(IntroTimerHandle);
	GetWorldTimerManager().UnPauseTimer(OutroTimerHandle);
	if (RhythmConductor)
	{
		RhythmConductor->ResumeConductor();
	}

	if (AudioManager)
	{
		AudioManager->ResumeBGM();
	}

	bInputLocked = !bIsRoundActive;
	ApplyGameOnlyInputMode();
}

void APTBBaseMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	UE_LOG(LogRhythm, Verbose, TEXT("[%s] NoteEvent NoteId=%d Action=%d TimeMs=%.2f Beat=%.2f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		Note.TimeMs,
		Note.BeatTime);

	OnMiniGameChartNote.Broadcast(Note);
	ReceiveChartNote(Note);
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

	OnMiniGameNoteArmed.Broadcast(Note);
	ReceiveNoteArmed(Note);
}

void APTBBaseMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	UE_LOG(LogRhythm, Verbose, TEXT("[%s] NoteCue NoteId=%d Action=%d CueBeat=%.2f TimeMs=%.2f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		Note.BeatTime,
		Note.TimeMs);

	OnMiniGameNoteCue.Broadcast(Note);
	ReceiveNoteCue(Note);
}

void APTBBaseMiniGame::HandleRhythmInput(EPTBActionType Action, float TimeMs)
{
	if (!CanAcceptInput())
	{
		return;
	}

	if (RuleSet && !RuleSet->SupportsAction(Action))
	{
		return;
	}

	const float ResolvedTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();
	if (const float* LockUntilTimeMs = EmptyInputActionLockUntilTimeMs.Find(Action))
	{
		if (ResolvedTimeMs < *LockUntilTimeMs)
		{
			UE_LOG(LogRhythm, Verbose, TEXT("[%s] Input action locked. Action=%d TimeMs=%.2f UnlockMs=%.2f"),
				*GetNameSafe(this),
				static_cast<int32>(Action),
				ResolvedTimeMs,
				*LockUntilTimeMs);
			return;
		}

		EmptyInputActionLockUntilTimeMs.Remove(Action);
	}

	FPTBNoteEvent TargetNote;
	if (JudgementSystem
		&& JudgementSystem->FindBestPendingNote(Action, ResolvedTimeMs, TargetNote)
		&& TargetNote.NoteType == EPTBNoteType::Hold)
	{
		EvaluateHoldInput(Action, ResolvedTimeMs);
		return;
	}

	EvaluateInput(Action, ResolvedTimeMs);
}

void APTBBaseMiniGame::HandleRhythmInputReleased(EPTBActionType Action, float TimeMs)
{
	if (!CanAcceptInput())
	{
		return;
	}

	if (RuleSet && !RuleSet->SupportsAction(Action))
	{
		return;
	}

	const float ResolvedTimeMs = TimeMs >= 0.0f ? TimeMs : GetCurrentInputJudgeTimeMs();
	int32 TargetHoldIndex = INDEX_NONE;
	float EarliestRequiredTimeMs = 0.0f;

	for (int32 Index = 0; Index < ActiveHoldStates.Num(); ++Index)
	{
		const FPTBActiveHoldState& HoldState = ActiveHoldStates[Index];
		if (HoldState.Note.ActionType != Action)
		{
			continue;
		}

		if (TargetHoldIndex == INDEX_NONE || HoldState.RequiredHoldUntilTimeMs < EarliestRequiredTimeMs)
		{
			TargetHoldIndex = Index;
			EarliestRequiredTimeMs = HoldState.RequiredHoldUntilTimeMs;
		}
	}

	if (TargetHoldIndex == INDEX_NONE)
	{
		return;
	}

	if (ResolvedTimeMs >= ActiveHoldStates[TargetHoldIndex].RequiredHoldUntilTimeMs)
	{
		ConfirmActiveHold(TargetHoldIndex);
		return;
	}

	FailActiveHoldEarlyRelease(TargetHoldIndex, ResolvedTimeMs);
}

FPTBJudgementResult APTBBaseMiniGame::EvaluateInput(EPTBActionType Action, float TimeMs)
{
	if (!JudgementSystem)
	{
		return FPTBJudgementResult();
	}

	const FPTBJudgementResult Result = JudgementSystem->EvaluateInput(Action, TimeMs);
	if (Result.Reason == EPTBJudgementReason::EmptyInput && RuleSet && RuleSet->ShouldLockActionOnEmptyInput())
	{
		EmptyInputActionLockUntilTimeMs.Add(Action, TimeMs + RuleSet->EmptyInputActionLockMs);
	}

	UE_LOG(LogRhythm, Verbose, TEXT("[%s] EvaluateInput Action=%d TimeMs=%.2f -> Judgement=%d ChartMs=%.2f InputMs=%.2f DeltaMs=%.2f"),
		*GetNameSafe(this),
		static_cast<int32>(Action),
		TimeMs,
		static_cast<int32>(Result.JudgementType),
		Result.ChartTimeMs,
		Result.InputTimeMs,
		Result.DeltaMs);
	return Result;
}

FPTBJudgementResult APTBBaseMiniGame::EvaluateHoldInput(EPTBActionType Action, float TimeMs)
{
	if (!JudgementSystem)
	{
		return FPTBJudgementResult();
	}

	FPTBNoteEvent MatchedNote;
	const bool bHasMatchedNote = JudgementSystem->FindBestPendingNote(Action, TimeMs, MatchedNote);
	const FPTBJudgementResult Result = JudgementSystem->EvaluateInput(Action, TimeMs, false);
	if (Result.Reason == EPTBJudgementReason::EmptyInput && RuleSet && RuleSet->ShouldLockActionOnEmptyInput())
	{
		EmptyInputActionLockUntilTimeMs.Add(Action, TimeMs + RuleSet->EmptyInputActionLockMs);
	}

	if (!bHasMatchedNote
		|| MatchedNote.NoteType != EPTBNoteType::Hold
		|| Result.JudgementType == EPTBJudgementType::Miss
		|| Result.Reason != EPTBJudgementReason::Note)
	{
		DispatchDeferredJudgementResult(Result);
		return Result;
	}

	const float HoldDurationMs = MatchedNote.ReleaseTimeMs - MatchedNote.TimeMs;
	if (HoldDurationMs <= 0.0f)
	{
		DispatchDeferredJudgementResult(Result);
		return Result;
	}

	FPTBActiveHoldState HoldState;
	HoldState.Note = MatchedNote;
	HoldState.PendingResult = Result;
	HoldState.PressedTimeMs = TimeMs;
	HoldState.RequiredHoldUntilTimeMs = MatchedNote.TimeMs + HoldDurationMs * PTBBaseMiniGameInternal::RequiredHoldRatio;
	ActiveHoldStates.Add(HoldState);

	if (TimeMs >= HoldState.RequiredHoldUntilTimeMs)
	{
		ConfirmActiveHold(ActiveHoldStates.Num() - 1);
	}

	UE_LOG(LogRhythm, Verbose, TEXT("[%s] Hold started NoteId=%d Action=%d RequiredUntilMs=%.2f"),
		*GetNameSafe(this),
		MatchedNote.NoteId,
		static_cast<int32>(MatchedNote.ActionType),
		HoldState.RequiredHoldUntilTimeMs);

	return Result;
}

void APTBBaseMiniGame::ResolveSatisfiedHoldInputs(float CurrentTimeMs)
{
	for (int32 Index = ActiveHoldStates.Num() - 1; Index >= 0; --Index)
	{
		if (CurrentTimeMs >= ActiveHoldStates[Index].RequiredHoldUntilTimeMs)
		{
			ConfirmActiveHold(Index);
		}
	}
}

void APTBBaseMiniGame::ConfirmActiveHold(int32 HoldIndex)
{
	if (!ActiveHoldStates.IsValidIndex(HoldIndex))
	{
		return;
	}

	const FPTBJudgementResult Result = ActiveHoldStates[HoldIndex].PendingResult;
	ActiveHoldStates.RemoveAt(HoldIndex);
	DispatchDeferredJudgementResult(Result);
}

void APTBBaseMiniGame::FailActiveHoldEarlyRelease(int32 HoldIndex, float ReleaseTimeMs)
{
	if (!ActiveHoldStates.IsValidIndex(HoldIndex))
	{
		return;
	}

	const FPTBActiveHoldState HoldState = ActiveHoldStates[HoldIndex];
	ActiveHoldStates.RemoveAt(HoldIndex);
	DispatchDeferredJudgementResult(PTBBaseMiniGameInternal::MakeEarlyReleaseResult(
		HoldState.Note,
		ReleaseTimeMs,
		HoldState.RequiredHoldUntilTimeMs));
}

void APTBBaseMiniGame::DispatchDeferredJudgementResult(const FPTBJudgementResult& Result)
{
	if (JudgementSystem)
	{
		JudgementSystem->OnJudgementResult.Broadcast(Result);
		return;
	}

	HandleJudgementResult(Result);
}

void APTBBaseMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		if (!RuleSet || !RuleSet->ShouldTreatEmptyInputAsMiss())
		{
			UE_LOG(LogRhythm, Verbose, TEXT("[%s] Empty input ignored. Action=%d"), *GetNameSafe(this), static_cast<int32>(Result.ActionType));
			OnMiniGameJudgement.Broadcast(Result);
			ReceiveJudgement(Result);
			return;
		}

		Result.bBreaksCombo = true;
	}

	PTBBaseMiniGameInternal::RemoveResolvedNote(ActiveNoteQueue, Result);

	if (ScoreCalculator)
	{
		ScoreCalculator->AddJudgementScore(Result);
	}

	if (AudioManager)
	{
		const FName RuleSetSFXKey = RuleSet ? RuleSet->GetJudgementSFXKey(Result.JudgementType) : NAME_None;
		if (!RuleSetSFXKey.IsNone())
		{
			AudioManager->PostSFXEvent(RuleSetSFXKey, this);
		}
		else
		{
			AudioManager->PostJudgementEvent(Result.JudgementType, this);
		}
	}

	OnMiniGameJudgement.Broadcast(Result);
	ReceiveJudgement(Result);

	if (RuleSet && ScoreCalculator && RuleSet->ShouldFailForMissCount(GameContext.SessionRequest.Difficulty, ScoreCalculator->MissCount))
	{
		FinishMiniGame(EPTBRoundEndReason::Failed);
	}
	
	UE_LOG(LogRhythm, Log, TEXT("[%s] Judgement NoteId=%d Action=%d Type=%d ChartMs=%.2f InputMs=%.2f DeltaMs=%.2f ScoreDelta=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		Result.ChartTimeMs,
		Result.InputTimeMs,
		Result.DeltaMs,
		Result.ScoreDelta);
}

void APTBBaseMiniGame::ReceiveChartNote_Implementation(FPTBNoteEvent Note)
{
}

void APTBBaseMiniGame::ReceiveNoteArmed_Implementation(FPTBNoteEvent Note)
{
}

void APTBBaseMiniGame::ReceiveNoteCue_Implementation(FPTBNoteEvent Note)
{
}

void APTBBaseMiniGame::ReceiveJudgement_Implementation(FPTBJudgementResult Result)
{
}

void APTBBaseMiniGame::ReceiveIntroStarted_Implementation()
{
}

void APTBBaseMiniGame::ReceiveGameplayStarted_Implementation()
{
}

void APTBBaseMiniGame::ReceiveOutroStarted_Implementation(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
}

void APTBBaseMiniGame::ReceiveOutroFinished_Implementation(FPTBRoundResult Result, EPTBRoundEndReason EndReason)
{
}

FPTBMiniGameResultPayload APTBBaseMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload;
	Payload.PayloadType = MiniGameCode.IsNone() ? MiniGameId : MiniGameCode;
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
