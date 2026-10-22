#include "MiniGames/Common/PTBBaseMiniGame.h"

#include "AkComponent.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Core/PTBGameInstance.h"
#include "Core/PTBGameModeBase.h"
#include "Debug/PTBLogChannels.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "MiniGames/Common/PTBMiniGamePreloadAssetSet.h"
#include "MiniGames/Common/UI/PTBMiniGameLoadingWidget.h"
#include "Components/InputComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/DataTable.h"
#include "Profile/PTBProfileSubsystem.h"
#include "UI/PTBSceneWidgetBase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Flow/PTBGameFlowSubsystem.h"

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

	const bool bIsCuePreRollActive = IsCuePreRollActive();
	if ((!bIsRoundActive && !bIsCuePreRollActive) || !JudgementSystem)
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

	if (bIsCuePreRollActive)
	{
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
	PreloadedAssets.Reset();
	EmptyInputActionLockUntilTimeMs.Reset();
	if (PreloadHandle.IsValid())
	{
		PreloadHandle->CancelHandle();
		PreloadHandle.Reset();
	}
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
	GetWorldTimerManager().ClearTimer(CuePreRollTimerHandle);
	GetWorldTimerManager().ClearTimer(OutroTimerHandle);
	CloseActiveSceneWidget();
	ScenePhase = EPTBMiniGameScenePhase::None;
	bTutorialOnlyMode = Context.SessionRequest.bTutorialOnly;
	// 튜토리얼만 보기는 본 적 있어도 재생
	bForceScenePlayback = Context.SessionRequest.bForceScenePlayback || bTutorialOnlyMode;

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

	RequestPreloadAssets();
}

void APTBBaseMiniGame::CompleteMiniGameInitialization()
{
	if (!ChartAsset)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] MiniGame initialization failed. ChartAsset is not ready. MiniGameId=%s"), *GetNameSafe(this), *MiniGameId.ToString());
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
		const float InputCompensationMs = FMath::Abs(ResolveInputOffsetMs(GameContext));
		RhythmConductor->SetArmLeadTimeMs(JudgementSystem->HitWindowMissMs + InputCompensationMs);
	}

	ApplyRuleSet();
	PreloadAudioAssets();
	BuildRuntimeState();

	bIsInitialized = true;
	bIsReadyToStart = true;

	// 시작 스토리 → 튜토리얼 → HandleReadyToStart(Press Any Key) 순으로 진행
	BeginIntroScenes();
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

void APTBBaseMiniGame::RequestPreloadAssets()
{
	TArray<FSoftObjectPath> AssetPaths;
	GatherPreloadAssetPaths(AssetPaths);

	if (AssetPaths.IsEmpty())
	{
		CompleteMiniGameInitialization();
		return;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	PreloadHandle = StreamableManager.RequestAsyncLoad(
		AssetPaths,
		FStreamableDelegate::CreateUObject(this, &APTBBaseMiniGame::HandlePreloadAssetsLoaded, AssetPaths));

	if (!PreloadHandle.IsValid())
	{
		UE_LOG(LogRhythm, Error, TEXT("[%s] Preload request failed. AssetCount=%d"), *GetNameSafe(this), AssetPaths.Num());
		HideLoadingWidget();
	}
}

void APTBBaseMiniGame::GatherPreloadAssetPaths(TArray<FSoftObjectPath>& OutAssetPaths) const
{
	if (!RuleSet || !RuleSet->PreloadAssetSet)
	{
		return;
	}

	for (const TSoftObjectPtr<UObject>& Asset : RuleSet->PreloadAssetSet->Assets)
	{
		const FSoftObjectPath AssetPath = Asset.ToSoftObjectPath();
		if (!AssetPath.IsValid())
		{
			continue;
		}

		OutAssetPaths.AddUnique(AssetPath);
	}
}

bool APTBBaseMiniGame::VerifyPreloadedAssets(const TArray<FSoftObjectPath>& AssetPaths)
{
	PreloadedAssets.Reset();

	bool bAllLoaded = true;
	for (const FSoftObjectPath& AssetPath : AssetPaths)
	{
		UObject* LoadedAsset = AssetPath.ResolveObject();
		if (!LoadedAsset)
		{
			bAllLoaded = false;
			UE_LOG(LogRhythm, Error, TEXT("[%s] Preload asset missing: %s"), *GetNameSafe(this), *AssetPath.ToString());
			continue;
		}

		PreloadedAssets.Add(LoadedAsset);
	}

	return bAllLoaded;
}

void APTBBaseMiniGame::HandlePreloadAssetsLoaded(TArray<FSoftObjectPath> AssetPaths)
{
	PreloadHandle.Reset();

	if (!VerifyPreloadedAssets(AssetPaths))
	{
		HideLoadingWidget();
		return;
	}

	UE_LOG(LogRhythm, Log, TEXT("[%s] Preload assets ready. Count=%d"), *GetNameSafe(this), PreloadedAssets.Num());
	CompleteMiniGameInitialization();
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
		const bool bPrepared = AudioManager->PrepareEvent(GameContext.ChartData.WwiseEventName);
		if (bPrepared)
		{
			UE_LOG(LogWwise, Log, TEXT("[%s] Wwise BGM event ready: %s"),
				*GetNameSafe(this),
				*GameContext.ChartData.WwiseEventName.ToString());
		}
		else
		{
			UE_LOG(LogWwise, Warning, TEXT("[%s] Wwise BGM event prepare failed: %s"),
				*GetNameSafe(this),
				*GameContext.ChartData.WwiseEventName.ToString());
		}
	}
}

void APTBBaseMiniGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Owner 관계만으로는 자동 파괴되지 않으므로 등록된 스폰 Actor를 직접 정리
	for (const TObjectPtr<AActor>& Actor : SpawnedRoundActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedRoundActors.Reset();

	CloseActiveSceneWidget();

	Super::EndPlay(EndPlayReason);
}

void APTBBaseMiniGame::RegisterSpawnedRoundActor(AActor* Actor)
{
	if (Actor)
	{
		SpawnedRoundActors.AddUnique(Actor);
	}
}

void APTBBaseMiniGame::RemoveExistingWidgetsOfClass(TSubclassOf<UUserWidget> WidgetClass, UUserWidget* WidgetToKeep) const
{
	if (!WidgetClass)
	{
		return;
	}

	TArray<UUserWidget*> ExistingWidgets;
	UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), ExistingWidgets, WidgetClass, false);
	for (UUserWidget* ExistingWidget : ExistingWidgets)
	{
		if (ExistingWidget && ExistingWidget != WidgetToKeep && ExistingWidget->IsInViewport())
		{
			ExistingWidget->RemoveFromParent();
		}
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
		GetWorldTimerManager().SetTimer(IntroTimerHandle, this, &APTBBaseMiniGame::BeginCuePreRollSequence, IntroDelaySeconds, false);
		return;
	}

	BeginCuePreRollSequence();
}

void APTBBaseMiniGame::BeginCuePreRollSequence()
{
	GetWorldTimerManager().ClearTimer(IntroTimerHandle);
	if (!bStartSequenceActive || bIsRoundActive || bFinishSequenceActive)
	{
		return;
	}

	OnMiniGameIntroFinished.Broadcast();
	ReceiveIntroFinished();

	const float CuePreRollTimeMs = ResolveCuePreRollTimeMs();
	if (CuePreRollTimeMs > 0.0f)
	{
		if (RhythmConductor)
		{
			RhythmConductor->StartCuePreRoll(ChartAsset.Get(), CuePreRollTimeMs);
		}

		bInputLocked = false;
		GetWorldTimerManager().SetTimer(CuePreRollTimerHandle, this, &APTBBaseMiniGame::BeginGameplaySequence, CuePreRollTimeMs / 1000.0f, false);
		return;
	}

	BeginGameplaySequence();
}

void APTBBaseMiniGame::BeginGameplaySequence()
{
	GetWorldTimerManager().ClearTimer(IntroTimerHandle);
	GetWorldTimerManager().ClearTimer(CuePreRollTimerHandle);
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
	GetWorldTimerManager().ClearTimer(CuePreRollTimerHandle);
	ApplyGameAndUIInputMode();

	// Aborted(재시작 등 중도 종료)에서는 판정 대기 중이던 노트를 Miss로 흘려보내지 않는다.
	// 예전에 여기서 흘려보냈다가 Retry 중 판정 델리게이트가 처음으로 실행되면서
	// 몇몇 미니게임 Blueprint(중단 상황을 가정 안 함)가 터지는 문제가 있었다.
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

	// 튜토리얼 연습은 아웃트로/결과 없이 위젯으로 결과만 전달
	if (bTutorialPracticeActive)
	{
		CompleteTutorialPractice(Reason);
		return RoundResult;
	}

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

	// 결과 직전 스토리가 있으면 재생 후 NotifySceneFinished에서 결과 전달
	if (PendingEndReason != EPTBRoundEndReason::Aborted
		&& TryPlayScene(EPTBMiniGameScenePhase::OutroStory))
	{
		return;
	}

	DispatchMiniGameFinished();
}

void APTBBaseMiniGame::DispatchMiniGameFinished()
{
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
	GetWorldTimerManager().PauseTimer(CuePreRollTimerHandle);
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
	GetWorldTimerManager().UnPauseTimer(CuePreRollTimerHandle);
	GetWorldTimerManager().UnPauseTimer(OutroTimerHandle);
	if (RhythmConductor)
	{
		RhythmConductor->ResumeConductor();
	}

	if (AudioManager)
	{
		AudioManager->ResumeBGM();
	}

	bInputLocked = !(bIsRoundActive || IsCuePreRollActive());
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

	// 튜토리얼 연습: 맞추든 못 맞추든 노트 타이밍마다 소리
	if (bTutorialPracticeActive && PracticeNoteSound)
	{
		UGameplayStatics::PlaySound2D(this, PracticeNoteSound);
	}

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
	// 씬 재생 중에는 리듬 키를 씬 위젯으로 전달 (튜토리얼 입력, 대사 넘기기)
	if (ActiveSceneWidget
		&& !bTutorialPracticeActive
		&& ScenePhase != EPTBMiniGameScenePhase::None
		&& ScenePhase != EPTBMiniGameScenePhase::Ready)
	{
		ActiveSceneWidget->HandleSceneAction(Action);
		return;
	}

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

void APTBBaseMiniGame::ReceiveIntroFinished_Implementation()
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

void APTBBaseMiniGame::ReceiveScenePhaseChanged_Implementation(EPTBMiniGameScenePhase Phase)
{
}

// ── 스토리/튜토리얼 씬 ─────────────────────────────────────────────

void APTBBaseMiniGame::BeginIntroScenes()
{
	ScenePhase = EPTBMiniGameScenePhase::None;
	AdvanceIntroScenes();
}

void APTBBaseMiniGame::AdvanceIntroScenes()
{
	// 튜토리얼만 보기: 스토리 건너뛰고 튜토리얼 → 끝나면 미니게임 선택 화면
	if (bTutorialOnlyMode)
	{
		if (ScenePhase == EPTBMiniGameScenePhase::None && TryPlayScene(EPTBMiniGameScenePhase::Tutorial))
		{
			return;
		}
		ExitTutorialOnly();
		return;
	}

	if (ScenePhase == EPTBMiniGameScenePhase::None
		&& TryPlayScene(EPTBMiniGameScenePhase::IntroStory))
	{
		return;
	}

	if ((ScenePhase == EPTBMiniGameScenePhase::None || ScenePhase == EPTBMiniGameScenePhase::IntroStory)
		&& TryPlayScene(EPTBMiniGameScenePhase::Tutorial))
	{
		return;
	}

	SetScenePhase(EPTBMiniGameScenePhase::Ready);
	SetLoadingWidgetHidden(false);
	HandleReadyToStart();
}

bool APTBBaseMiniGame::TryPlayScene(EPTBMiniGameScenePhase Phase)
{
	const UPTBMiniGameIntroData* SceneData = ResolveSceneData(Phase);
	if (!SceneData || !ShouldPlayScene(Phase))
	{
		return false;
	}

	const TSubclassOf<UPTBSceneWidgetBase> WidgetClass = ResolveSceneWidgetClass(Phase);
	if (!WidgetClass)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Scene skipped: widget class is not set on GameInstance. Phase=%d"),
			*GetNameSafe(this), static_cast<int32>(Phase));
		return false;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return false;
	}

	UPTBSceneWidgetBase* SceneWidget = CreateWidget<UPTBSceneWidgetBase>(PlayerController, WidgetClass);
	if (!SceneWidget)
	{
		return false;
	}

	CloseActiveSceneWidget();
	ActiveSceneWidget = SceneWidget;

	SetLoadingWidgetHidden(true);
	SceneWidget->AddToViewport(10);

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(SceneWidget->TakeWidget());
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);

	// SetupScene 안에서 바로 FinishScene이 불려도 흐름이 꼬이지 않도록 단계 먼저 갱신
	SetScenePhase(Phase);
	SceneWidget->SetupScene(this, Phase, SceneData->GetTable(Phase));

	UE_LOG(LogRhythm, Log, TEXT("[%s] Scene started. Phase=%d Data=%s Force=%d"),
		*GetNameSafe(this), static_cast<int32>(Phase), *GetNameSafe(SceneData), bForceScenePlayback ? 1 : 0);
	return true;
}

bool APTBBaseMiniGame::ShouldPlayScene(EPTBMiniGameScenePhase Phase) const
{
	if (GameContext.SessionRequest.PlayMode == EPTBPlayMode::Multiplayer)
	{
		return false;
	}

	const UPTBMiniGameIntroData* SceneData = ResolveSceneData(Phase);
	if (!SceneData)
	{
		return false;
	}

	// 결과 직전 스토리: 실패/중도 종료면 재생 안 함 (설정 시)
	if (Phase == EPTBMiniGameScenePhase::OutroStory && SceneData->bOutroRequiresClear)
	{
		const bool bCleared = PendingEndReason == EPTBRoundEndReason::Completed
			&& RoundResult.Grade != EPTBGradeType::Fail;
		if (!bCleared)
		{
			return false;
		}
	}

	if (bForceScenePlayback)
	{
		return true;
	}

	const UGameInstance* GI = GetGameInstance();
	const UPTBProfileSubsystem* ProfileSubsystem = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	if (!ProfileSubsystem)
	{
		return true;
	}

	const FName SceneKey = UPTBMiniGameIntroData::MakeSceneKey(ResolveSceneMiniGameId(), Phase);
	return !ProfileSubsystem->HasSeenScene(SceneKey, SceneData->GetVersion(Phase));
}

const UPTBMiniGameIntroData* APTBBaseMiniGame::ResolveSceneData(EPTBMiniGameScenePhase Phase) const
{
	if (IntroData && IntroData->HasContent(Phase))
	{
		return IntroData;
	}

	// 결과 직전 스토리는 없는 게임이 정상이므로 공용 템플릿으로 폴백하지 않음
	if (Phase == EPTBMiniGameScenePhase::OutroStory)
	{
		return nullptr;
	}

	const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance());
	const UPTBMiniGameIntroData* DefaultData = PTBGI ? PTBGI->DefaultIntroData.Get() : nullptr;
	return DefaultData && DefaultData->HasContent(Phase) ? DefaultData : nullptr;
}

TSubclassOf<UPTBSceneWidgetBase> APTBBaseMiniGame::ResolveSceneWidgetClass(EPTBMiniGameScenePhase Phase) const
{
	const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance());
	if (!PTBGI)
	{
		return nullptr;
	}

	return Phase == EPTBMiniGameScenePhase::Tutorial
		? PTBGI->TutorialWidgetClass
		: PTBGI->StoryWidgetClass;
}

FName APTBBaseMiniGame::ResolveSceneMiniGameId() const
{
	if (!MiniGameId.IsNone())
	{
		return MiniGameId;
	}

	if (RuleSet && !RuleSet->MiniGameId.IsNone())
	{
		return RuleSet->MiniGameId;
	}

	return GetClass()->GetFName();
}

void APTBBaseMiniGame::SetScenePhase(EPTBMiniGameScenePhase NewPhase)
{
	if (ScenePhase == NewPhase)
	{
		return;
	}

	ScenePhase = NewPhase;
	OnScenePhaseChanged.Broadcast(NewPhase);
	ReceiveScenePhaseChanged(NewPhase);

	// 튜토리얼 시작: 레벨 BP가 카메라/HUD를 미리 준비할 수 있도록
	if (NewPhase == EPTBMiniGameScenePhase::Tutorial)
	{
		if (APTBGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APTBGameModeBase>() : nullptr)
		{
			GM->OnTutorialStarted.Broadcast();
		}
	}
}

void APTBBaseMiniGame::NotifySceneFinished(bool bSkipped)
{
	const EPTBMiniGameScenePhase FinishedPhase = ScenePhase;
	if (FinishedPhase == EPTBMiniGameScenePhase::None || FinishedPhase == EPTBMiniGameScenePhase::Ready)
	{
		return;
	}

	// 연습 도중 튜토리얼 스킵: 연습 중단 후 본게임 채보 복구
	if (bTutorialPracticeActive)
	{
		FinishMiniGame(EPTBRoundEndReason::Aborted);
	}

	// 스킵해도 '봤음'으로 기록. 다시 보려면 다시보기(bForceScenePlayback)로
	if (const UPTBMiniGameIntroData* SceneData = ResolveSceneData(FinishedPhase))
	{
		UGameInstance* GI = GetGameInstance();
		if (UPTBProfileSubsystem* ProfileSubsystem = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr)
		{
			const FName SceneMiniGameId = ResolveSceneMiniGameId();
			const FName SceneKey = UPTBMiniGameIntroData::MakeSceneKey(SceneMiniGameId, FinishedPhase);
			ProfileSubsystem->MarkSceneSeen(SceneKey, SceneData->GetVersion(FinishedPhase));

			// 기존 진행도 API와도 동기화 (IsFirstTimeProfile 등에서 사용)
			if (FinishedPhase == EPTBMiniGameScenePhase::Tutorial)
			{
				ProfileSubsystem->MarkTutorialComplete(SceneMiniGameId);
			}
			else
			{
				ProfileSubsystem->MarkStoryViewed(SceneKey);
			}
		}
	}

	UE_LOG(LogRhythm, Log, TEXT("[%s] Scene finished. Phase=%d Skipped=%d"),
		*GetNameSafe(this), static_cast<int32>(FinishedPhase), bSkipped ? 1 : 0);

	CloseActiveSceneWidget();

	if (FinishedPhase == EPTBMiniGameScenePhase::OutroStory)
	{
		SetScenePhase(EPTBMiniGameScenePhase::None);
		ApplyGameAndUIInputMode();
		DispatchMiniGameFinished();
		return;
	}

	AdvanceIntroScenes();
}

// ── 튜토리얼 연습 ──────────────────────────────────────────────────

bool APTBBaseMiniGame::StartTutorialPractice(const FPTBTutorialStep& Step)
{
	if (bTutorialPracticeActive || bIsRoundActive || bStartSequenceActive || bFinishSequenceActive)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Tutorial practice ignored: round busy"), *GetNameSafe(this));
		return false;
	}

	if (!bIsInitialized || ScenePhase != EPTBMiniGameScenePhase::Tutorial)
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Tutorial practice ignored: not in tutorial phase"), *GetNameSafe(this));
		return false;
	}

	UPTBRhythmChartAsset* PracticeChart = BuildTutorialPracticeChart(Step);
	if (!PracticeChart || PracticeChart->NoteEvents.IsEmpty())
	{
		UE_LOG(LogRhythm, Warning, TEXT("[%s] Tutorial practice ignored: empty pattern/chart"), *GetNameSafe(this));
		return false;
	}

	// 본게임 채보는 연습이 끝나면 복구
	if (!SavedMainChartAsset)
	{
		SavedMainChartAsset = ChartAsset;
		SavedMainChartData = GameContext.ChartData;
	}

	TutorialPracticeChart = PracticeChart;
	ActivePracticeTag = Step.PracticeTag;
	ChartAsset = PracticeChart;
	GameContext.ChartData = PracticeChart->ChartData;
	ResetRoundStateForReplay();
	HandleRoundChartChanged();

	PracticeTotalNotes = PracticeChart->NoteEvents.Num();
	PracticeRequiredHits = Step.PracticeRequiredHits > 0
		? FMath::Min(Step.PracticeRequiredHits, PracticeTotalNotes)
		: PracticeTotalNotes;

	const UPTBMiniGameIntroData* SceneData = ResolveSceneData(EPTBMiniGameScenePhase::Tutorial);
	const UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance());
	PracticeMetronomeSFXKey = SceneData ? SceneData->TutorialMetronomeSFXKey : NAME_None;
	PracticeNoteSound = SceneData && SceneData->TutorialNoteSound
		? SceneData->TutorialNoteSound.Get()
		: (PTBGI ? PTBGI->DefaultTutorialNoteSound.Get() : nullptr);
	PracticeMetronomeSound = SceneData && SceneData->TutorialMetronomeSound
		? SceneData->TutorialMetronomeSound.Get()
		: (PTBGI ? PTBGI->DefaultTutorialMetronomeSound.Get() : nullptr);
	if (RhythmConductor && (PracticeMetronomeSound || !PracticeMetronomeSFXKey.IsNone()))
	{
		RhythmConductor->OnBeatTick.AddUniqueDynamic(this, &APTBBaseMiniGame::HandlePracticeBeatTick);
	}

	bTutorialPracticeActive = true;
	bIsReadyToStart = true;
	bStartSequenceActive = true;
	bInputLocked = true;

	UE_LOG(LogRhythm, Log, TEXT("[%s] Tutorial practice started. Notes=%d Required=%d BPM=%.1f"),
		*GetNameSafe(this), PracticeTotalNotes, PracticeRequiredHits, GameContext.ChartData.BPM);

	// BP에서 HUD/판정 UI 준비 (미니게임 BP / 레벨 BP 둘 다 지원)
	ReceiveTutorialPracticeStarted();
	if (APTBGameModeBase* GM = GetWorld() ? GetWorld()->GetAuthGameMode<APTBGameModeBase>() : nullptr)
	{
		GM->OnTutorialPracticeStarted.Broadcast();
	}

	// 인트로 연출 없이 바로 시작. BGM 이벤트가 없으므로 대체 타이밍(메트로놈)으로 진행
	// OnMiniGameStarted는 브로드캐스트하지 않음 (GameMode가 본게임 시작으로 처리하지 않도록)
	BeginCuePreRollSequence();
	return true;
}

UPTBRhythmChartAsset* APTBBaseMiniGame::BuildTutorialPracticeChart(const FPTBTutorialStep& Step)
{
	// 지정 채보가 있으면 그대로 사용 (BGM 포함)
	if (!Step.PracticeChartOverride.IsNull())
	{
		return Step.PracticeChartOverride.LoadSynchronous();
	}

	const float BPM = FMath::Max(30.0f, Step.PracticeBPM);
	const float MsPerBeat = 60000.0f / BPM;
	const float Interval = FMath::Max(0.25f, Step.PracticeBeatInterval);

	UPTBRhythmChartAsset* Chart = NewObject<UPTBRhythmChartAsset>(this, NAME_None, RF_Transient);
	Chart->ChartId = TEXT("Tutorial");
	Chart->ChartData.BPM = BPM;
	Chart->ChartData.OffsetMs = 0.0f;
	Chart->ChartData.WwiseEventName = NAME_None;
	Chart->ChartData.ChartId = Chart->ChartId;
	Chart->ChartData.MiniGameId = ResolveSceneMiniGameId();
	Chart->ChartData.Difficulty = GameContext.SessionRequest.Difficulty;

	TArray<FString> Tokens;
	Step.PracticePattern.ParseIntoArrayWS(Tokens);

	float Beat = static_cast<float>(FMath::Max(0, Step.PracticeLeadInBeats));
	// 회차마다 다른 NoteId 대역 사용 (NoteId로 중복을 막는 미니게임 대비, 본게임 채보 ID와도 안 겹치게)
	int32 NextNoteId = 100000 + (PracticeRunCounter++ % 1000) * 100 + 1;
	for (const FString& Token : Tokens)
	{
		const TCHAR Symbol = Token.IsEmpty() ? TEXT('-') : FChar::ToUpper(Token[0]);
		EPTBActionType Action = EPTBActionType::None;
		switch (Symbol)
		{
		case TEXT('A'): Action = EPTBActionType::ActionA; break;
		case TEXT('B'): Action = EPTBActionType::ActionB; break;
		case TEXT('C'): Action = EPTBActionType::ActionC; break;
		case TEXT('D'): Action = EPTBActionType::ActionD; break;
		case TEXT('E'): Action = EPTBActionType::ActionE; break;
		default: break; // '-' 등은 쉼표
		}

		if (Action != EPTBActionType::None)
		{
			FPTBNoteEvent Note;
			Note.NoteId = NextNoteId++;
			Note.BeatTime = Beat;
			Note.TimeMs = Beat * MsPerBeat;
			Note.ActionType = Action;
			Note.NoteType = EPTBNoteType::Tap;
			Note.Lane = static_cast<int32>(Action) - 1;
			Chart->NoteEvents.Add(Note);
		}

		Beat += Interval;
	}

	// 마지막 노트 뒤 2박 여유
	Chart->ChartData.SongLengthMs = (Beat + 2.0f) * MsPerBeat;
	return Chart;
}

void APTBBaseMiniGame::CompleteTutorialPractice(EPTBRoundEndReason Reason)
{
	if (RhythmConductor)
	{
		RhythmConductor->OnBeatTick.RemoveDynamic(this, &APTBBaseMiniGame::HandlePracticeBeatTick);
	}

	const int32 HitCount = RoundResult.HighPerfectCount + RoundResult.PerfectCount + RoundResult.GoodCount;
	const int32 TotalCount = PracticeTotalNotes;
	const int32 RequiredCount = PracticeRequiredHits;
	const bool bPassed = Reason != EPTBRoundEndReason::Aborted && HitCount >= RequiredCount;

	// 본게임 채보 복구 + 상태 초기화 (점수/결과에 연습이 남지 않도록)
	if (SavedMainChartAsset)
	{
		ChartAsset = SavedMainChartAsset;
		GameContext.ChartData = SavedMainChartData;
		SavedMainChartAsset = nullptr;
	}
	TutorialPracticeChart = nullptr;
	ActivePracticeTag = NAME_None;
	ResetRoundStateForReplay();
	HandleRoundChartChanged();

	bTutorialPracticeActive = false;
	bFinishSequenceActive = false;
	bIsReadyToStart = true;
	bInputLocked = true;

	UE_LOG(LogRhythm, Log, TEXT("[%s] Tutorial practice finished. Reason=%d Hits=%d/%d Required=%d Passed=%d"),
		*GetNameSafe(this), static_cast<int32>(Reason), HitCount, TotalCount, RequiredCount, bPassed ? 1 : 0);

	// FinishMiniGame이 바꾼 입력 모드를 튜토리얼 위젯 포커스로 되돌림
	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PlayerController && ActiveSceneWidget)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ActiveSceneWidget->TakeWidget());
		InputMode.SetHideCursorDuringCapture(false);
		PlayerController->SetInputMode(InputMode);
		PlayerController->SetShowMouseCursor(true);
	}

	ReceiveTutorialPracticeFinished(bPassed);

	if (Reason != EPTBRoundEndReason::Aborted && ActiveSceneWidget)
	{
		ActiveSceneWidget->HandlePracticeFinished(bPassed, HitCount, RequiredCount, TotalCount);
	}
}

bool APTBBaseMiniGame::HandleSceneConfirmInput()
{
	if (!ActiveSceneWidget
		|| bTutorialPracticeActive
		|| ScenePhase == EPTBMiniGameScenePhase::None
		|| ScenePhase == EPTBMiniGameScenePhase::Ready)
	{
		return false;
	}

	return ActiveSceneWidget->HandleConfirmInput();
}

void APTBBaseMiniGame::ReceiveTutorialPracticeStarted_Implementation()
{
}

void APTBBaseMiniGame::ReceiveTutorialPracticeFinished_Implementation(bool bPassed)
{
}

void APTBBaseMiniGame::ResetRoundStateForReplay()
{
	if (JudgementSystem)
	{
		JudgementSystem->Initialize(GameContext.ChartData, 0.0f);
	}
	if (ScoreCalculator)
	{
		ScoreCalculator->Reset();
	}

	RoundResult = FPTBRoundResult();
	ActiveNoteQueue.Reset();
	ActiveHoldStates.Reset();
	EmptyInputActionLockUntilTimeMs.Reset();
	bPendingRoundFinish = false;
	bPendingRoundFailed = false;
	bAllNotesDispatched = false;
	ActiveBGMPlayingId = 0;
	PendingEndReason = EPTBRoundEndReason::Completed;
}

void APTBBaseMiniGame::ExitTutorialOnly()
{
	SetScenePhase(EPTBMiniGameScenePhase::None);
	CloseActiveSceneWidget();

	UE_LOG(LogRhythm, Log, TEXT("[%s] Tutorial-only finished. Returning to mini game select."), *GetNameSafe(this));

	UGameInstance* GI = GetGameInstance();
	if (UPTBGameFlowSubsystem* Flow = GI ? GI->GetSubsystem<UPTBGameFlowSubsystem>() : nullptr)
	{
		Flow->ReturnToMiniGameSelect();
	}
}

void APTBBaseMiniGame::HandleRoundChartChanged()
{
}

void APTBBaseMiniGame::HandlePracticeBeatTick(float Beat)
{
	if (!bTutorialPracticeActive)
	{
		return;
	}

	if (PracticeMetronomeSound)
	{
		UGameplayStatics::PlaySound2D(this, PracticeMetronomeSound);
	}
	else if (!PracticeMetronomeSFXKey.IsNone())
	{
		RequestWwiseEvent(PracticeMetronomeSFXKey, this);
	}
}

void APTBBaseMiniGame::CloseActiveSceneWidget()
{
	if (ActiveSceneWidget)
	{
		ActiveSceneWidget->RemoveFromParent();
		ActiveSceneWidget = nullptr;
	}
}

void APTBBaseMiniGame::SetLoadingWidgetHidden(bool bShouldHide)
{
	if (LoadingWidgetInstance)
	{
		LoadingWidgetInstance->SetVisibility(bShouldHide ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
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
	return bIsInitialized
		&& (bIsRoundActive || IsCuePreRollActive())
		&& !bInputLocked
		&& JudgementSystem != nullptr
		&& ChartAsset != nullptr;
}

bool APTBBaseMiniGame::IsCuePreRollActive() const
{
	return bStartSequenceActive && RhythmConductor && RhythmConductor->IsCuePreRollOnly();
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

float APTBBaseMiniGame::ResolveCuePreRollTimeMs() const
{
	if (!RuleSet || !RuleSet->bUseCuePreRoll)
	{
		return 0.0f;
	}

	if (RuleSet->CuePreRollMode == EPTBCueLeadTimeMode::MS)
	{
		return FMath::Max(0.0f, RuleSet->CuePreRollTimeMs);
	}

	const float BPM = GameContext.ChartData.BPM > 0.0f
		? GameContext.ChartData.BPM
		: (ChartAsset ? ChartAsset->ChartData.BPM : 0.0f);
	if (BPM <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Max(0.0f, RuleSet->CuePreRollBeats) * 60000.0f / BPM;
}

float APTBBaseMiniGame::GetCurrentChartTimeMs() const
{
	if (RhythmConductor && RhythmConductor->IsCuePreRollOnly())
	{
		return RhythmConductor->GetCurrentChartTimeMs();
	}

	if (RhythmSyncComponent)
	{
		return RhythmSyncComponent->GetChartTimeMs();
	}

	if (RhythmConductor)
	{
		return RhythmConductor->GetCurrentChartTimeMs();
	}

	return 0.0f;
}

float APTBBaseMiniGame::GetCurrentInputJudgeTimeMs() const
{
	if (IsCuePreRollActive())
	{
		return GetCurrentChartTimeMs() + ResolveInputOffsetMs(GameContext);
	}

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
