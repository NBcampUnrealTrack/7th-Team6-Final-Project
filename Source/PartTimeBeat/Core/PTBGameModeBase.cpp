#include "Core/PTBGameModeBase.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "Profile/PTBProfileSubsystem.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Components/MeshComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void APTBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	if (UGameViewportClient* VC = GetWorld()->GetGameViewport())
	{
		VC->SetIgnoreInput(false);
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		return;
	}

	UPTBGameFlowSubsystem* FlowSubsystem = GameInstance->GetSubsystem<UPTBGameFlowSubsystem>();
	if (FlowSubsystem && FlowSubsystem->CurrentFlowState == EGameFlowState::InGame)
	{
		const FPTBGameSessionRequest& PendingRequest = FlowSubsystem->PendingSessionRequest;
		if (!PendingRequest.MiniGameId.IsNone())
		{
			StartGameFlow(PendingRequest);
			return;
		}
	}

	// HandleMiniGameStarted가 건 GameOnly 모드가 OpenLevel 후에도 남아있어 허브 UI가 클릭을 못 받던 문제 방지
	if (FlowSubsystem && FlowSubsystem->CurrentFlowState == EGameFlowState::MiniGameSelect)
	{
		if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
	}

	// 허브 맵이면 플레이어 위치를 주기적으로 기록 (미니게임 복귀·재실행 시 그 자리에서 시작)
	if (IsHubLevel())
	{
		StartHubPositionTracking();
	}

	UWorld* World = GetWorld();
	if (!World || World->WorldType != EWorldType::PIE)
	{
		return;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	bool bCanTryDirectPIEMiniGame = false;

	// 1차: 키 이름만으로 매칭 (클래스 로드 없음)
	for (const TPair<FName, TSoftClassPtr<APTBBaseMiniGame>>& ClassEntry : MiniGameClassMap)
	{
		if (CurrentLevelName.Contains(ClassEntry.Key.ToString(), ESearchCase::IgnoreCase))
		{
			bCanTryDirectPIEMiniGame = true;
			break;
		}
	}

	// 2차: RuleSet Id/Code로 매칭 (PIE 전용이므로 동기 로드 허용)
	if (!bCanTryDirectPIEMiniGame)
	{
		for (const TPair<FName, TSoftClassPtr<APTBBaseMiniGame>>& ClassEntry : MiniGameClassMap)
		{
			const TSubclassOf<APTBBaseMiniGame> CandidateClass = ClassEntry.Value.LoadSynchronous();
			if (DoesLevelMatchMiniGame(CurrentLevelName, ClassEntry.Key, CandidateClass))
			{
				bCanTryDirectPIEMiniGame = true;
				break;
			}
		}
	}

	if (!bCanTryDirectPIEMiniGame)
	{
		return;
	}

	FTimerDelegate DeferredDirectPIEStart;
	DeferredDirectPIEStart.BindWeakLambda(this, [this]()
		{
			TryStartDirectPIEMiniGame();
		});
	World->GetTimerManager().SetTimerForNextTick(DeferredDirectPIEStart);
}

bool APTBGameModeBase::DoesLevelMatchMiniGame(const FString& LevelName, FName Key, TSubclassOf<APTBBaseMiniGame> Cls) const
{
	if (LevelName.Contains(Key.ToString(), ESearchCase::IgnoreCase))
	{
		return true;
	}

	const APTBBaseMiniGame* CDO = Cls ? Cls->GetDefaultObject<APTBBaseMiniGame>() : nullptr;
	if (!CDO || !CDO->RuleSet)
	{
		return false;
	}

	return LevelName.Contains(CDO->RuleSet->MiniGameId.ToString(), ESearchCase::IgnoreCase)
		|| LevelName.Contains(CDO->RuleSet->MiniGameCode.ToString(), ESearchCase::IgnoreCase);
}

bool APTBGameModeBase::TryStartDirectPIEMiniGame()
{
	UWorld* World = GetWorld();
	if (!World || World->WorldType != EWorldType::PIE)
	{
		return false;
	}

	TArray<AActor*> MiniGameActors;
	UGameplayStatics::GetAllActorsOfClass(this, APTBBaseMiniGame::StaticClass(), MiniGameActors);

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	APTBBaseMiniGame* DirectMiniGame = nullptr;
	bool bSpawnedDirectMiniGame = false;
	for (AActor* Actor : MiniGameActors)
	{
		APTBBaseMiniGame* Candidate = Cast<APTBBaseMiniGame>(Actor);
		if (Candidate)
		{
			PTB_VERBOSE(LogPTBMiniGames, TEXT("Direct PIE MiniGame candidate [%s] Class=%s AutoStart=%d RuleSet=%s"),
				*GetNameSafe(Candidate),
				*GetNameSafe(Candidate->GetClass()),
				Candidate->bAutoStartWhenOpenedDirectlyInPIE ? 1 : 0,
				*GetNameSafe(Candidate->RuleSet));
		}

		if (Candidate && Candidate->bAutoStartWhenOpenedDirectlyInPIE)
		{
			DirectMiniGame = Candidate;
			break;
		}
	}

	if (!DirectMiniGame)
	{
		TSubclassOf<APTBBaseMiniGame> DirectMiniGameClass = nullptr;
		for (const TPair<FName, TSoftClassPtr<APTBBaseMiniGame>>& ClassEntry : MiniGameClassMap)
		{
			// PIE 전용 경로이므로 동기 로드 허용
			const TSubclassOf<APTBBaseMiniGame> CandidateClass = ClassEntry.Value.LoadSynchronous();
			const APTBBaseMiniGame* CandidateCDO = CandidateClass
				? CandidateClass->GetDefaultObject<APTBBaseMiniGame>()
				: nullptr;

			if (CandidateCDO)
			{
				PTB_VERBOSE(LogPTBMiniGames, TEXT("Direct PIE MiniGame class candidate Id=%s Class=%s AutoStart=%d RuleSet=%s"),
					*ClassEntry.Key.ToString(),
					*GetNameSafe(CandidateClass),
					CandidateCDO->bAutoStartWhenOpenedDirectlyInPIE ? 1 : 0,
					*GetNameSafe(CandidateCDO->RuleSet));
			}

			if (CandidateCDO && CandidateCDO->bAutoStartWhenOpenedDirectlyInPIE)
			{
				if (!DoesLevelMatchMiniGame(CurrentLevelName, ClassEntry.Key, CandidateClass))
				{
					PTB_VERBOSE(LogPTBMiniGames, TEXT("Direct PIE MiniGame class skipped: current level [%s] does not match Id=%s Class=%s."),
						*CurrentLevelName,
						*ClassEntry.Key.ToString(),
						*GetNameSafe(CandidateClass));
					continue;
				}

				DirectMiniGameClass = CandidateClass;
				break;
			}
		}

		if (DirectMiniGameClass)
		{
			DirectMiniGame = SpawnMiniGame(DirectMiniGameClass);
			bSpawnedDirectMiniGame = DirectMiniGame != nullptr;
			if (!DirectMiniGame)
			{
				PTB_WARNING(LogPTBMiniGames, TEXT("Direct PIE MiniGame start failed: could not spawn [%s]."),
					*GetNameSafe(DirectMiniGameClass));
				return false;
			}
		}
		else
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("Direct PIE MiniGame start skipped: no placed actor or mapped class has bAutoStartWhenOpenedDirectlyInPIE enabled. PlacedFound=%d ClassMapEntries=%d"),
				MiniGameActors.Num(),
				MiniGameClassMap.Num());
			return false;
		}
	}

	if (!DirectMiniGame->RuleSet)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("Direct PIE MiniGame start failed: RuleSet is missing on [%s]."), *GetNameSafe(DirectMiniGame));
		return false;
	}

	if (DirectMiniGame->RuleSet->MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("Direct PIE MiniGame start failed: RuleSet.MiniGameId is None on [%s]."), *GetNameSafe(DirectMiniGame));
		return false;
	}

	CurrentRequest = FPTBGameSessionRequest();
	CurrentRequest.MiniGameId = DirectMiniGame->RuleSet->MiniGameId;
	CurrentRequest.MiniGameCode = DirectMiniGame->RuleSet->MiniGameCode.IsNone()
		? CurrentRequest.MiniGameId
		: DirectMiniGame->RuleSet->MiniGameCode;
	CurrentRequest.Difficulty = DirectMiniGame->DirectPIEDifficulty;
	CurrentRequest.PlayMode = DirectMiniGame->DirectPIEPlayMode;
	CurrentRequest.RandomSeed = FMath::Rand();
	CurrentRequest.ExpectedPlayerCount = 1;

	bIsGameActive = false;
	bIsPaused = false;

	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	UPTBGameFlowSubsystem* FlowSubsystem = GI ? GI->GetSubsystem<UPTBGameFlowSubsystem>() : nullptr;
	if (GI)
	{
		GI->CurrentPlayMode = CurrentRequest.PlayMode;

		if (UPTBProfileSubsystem* ProfileSubsystem = GI->GetSubsystem<UPTBProfileSubsystem>())
		{
			if (ProfileSubsystem->ActivateDevelopmentProfileForPIE())
			{
				bool bHasDevelopmentProfile = false;
				const FPTBProfileData DevelopmentProfile = ProfileSubsystem->GetActiveProfile(bHasDevelopmentProfile);
				if (bHasDevelopmentProfile)
				{
					CurrentRequest.ProfileId = DevelopmentProfile.ProfileId;
					PTB_RECORD(LogPTBMiniGames, TEXT("Direct PIE MiniGame start: using development profile [%s]."), *CurrentRequest.ProfileId.ToString());
				}
			}
		}
	}

	if (FlowSubsystem)
	{
		FlowSubsystem->ClearRetryTarget();
		FlowSubsystem->SelectedMiniGameId = CurrentRequest.MiniGameId;
		FlowSubsystem->SelectedDifficulty = CurrentRequest.Difficulty;
		FlowSubsystem->PendingSessionRequest = CurrentRequest;
		FlowSubsystem->SetFlowState(EGameFlowState::InGame);
	}

	ActiveMiniGame = DirectMiniGame;
	ActiveMiniGame->OnMiniGameStarted.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
	ActiveMiniGame->OnMiniGameFinished.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);

	FPTBMiniGameContext Context;
	Context.SessionRequest = CurrentRequest;
	if (GI)
	{
		Context.UserSettings = GI->CachedSettings;
	}
	Context.LocalPlayerIndex = 0;

	ActiveMiniGame->InitializeMiniGame(Context);

	if (FlowSubsystem && !CurrentLevelName.IsEmpty())
	{
		FlowSubsystem->CacheRetryTarget(CurrentRequest, FName(*CurrentLevelName));
	}

	PTB_RECORD(LogPTBMiniGames, TEXT("Direct PIE MiniGame initialized [%s] from %s mini-game [%s]. Waiting for start input."),
		*CurrentRequest.MiniGameId.ToString(),
		bSpawnedDirectMiniGame ? TEXT("spawned") : TEXT("placed"),
		*GetNameSafe(ActiveMiniGame));

	return true;
}

void APTBGameModeBase::StartGameFlow(const FPTBGameSessionRequest& Request)
{
	CurrentRequest = Request;
	bIsGameActive = false;
	bIsPaused = false;

	UGameInstance* GameInstance = GetGameInstance();
	UPTBGameFlowSubsystem* FlowSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPTBGameFlowSubsystem>()
		: nullptr;

	if (FlowSubsystem)
	{
		FlowSubsystem->ClearRetryTarget();
		FlowSubsystem->PendingSessionRequest = CurrentRequest;
	}

	// 1) 미니게임 클래스 해석 (Soft 참조)
	const TSoftClassPtr<APTBBaseMiniGame> SoftClass = ResolveMiniGameClass(CurrentRequest.MiniGameId);
	if (SoftClass.IsNull())
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: MiniGame class not found for [%s]"),
			*CurrentRequest.MiniGameId.ToString());
		return;
	}

	// 2) 기존 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->OnMiniGameStarted.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
		ActiveMiniGame->OnMiniGameFinished.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	// 로드 + 텍스처 스트리밍 동안 화면을 가림 (ContinueStartGameFlow에서 페이드인)
	HoldScreenBlack();

	// 진행 중인 이전 로드가 있으면 취소
	const int32 Serial = ++StartFlowSerial;
	if (MiniGameLoadHandle.IsValid())
	{
		MiniGameLoadHandle->CancelHandle();
		MiniGameLoadHandle.Reset();
	}

	// 이미 로드돼 있으면 (Retry 등) 바로 진행
	if (UClass* LoadedClass = SoftClass.Get())
	{
		ContinueStartGameFlow(LoadedClass);
		return;
	}

	// 3) 선택된 미니게임만 비동기 로드
	PTB_VERBOSE(LogPTBMiniGames, TEXT("StartGameFlow: async loading [%s]"), *SoftClass.ToString());

	MiniGameLoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(
		SoftClass.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, SoftClass, Serial]()
			{
				// 로드 중 더 새로운 요청이 들어왔으면 무시
				if (Serial != StartFlowSerial)
				{
					return;
				}

				UClass* LoadedClass = SoftClass.Get();
				if (!LoadedClass)
				{
					PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: async load failed for [%s]"), *SoftClass.ToString());
					FadeInScreen(0.f);
					return;
				}

				ContinueStartGameFlow(LoadedClass);
			}));

	if (!MiniGameLoadHandle.IsValid())
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: RequestAsyncLoad failed for [%s]"), *SoftClass.ToString());
		FadeInScreen(0.f);
	}
}

void APTBGameModeBase::HoldScreenBlack()
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (PC && PC->PlayerCameraManager)
	{
		PC->PlayerCameraManager->SetManualCameraFade(1.f, FLinearColor::Black, false);
	}
}

void APTBGameModeBase::FadeInScreen(float Duration)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	if (Duration <= 0.f)
	{
		PC->PlayerCameraManager->StopCameraFade();
		return;
	}

	PC->PlayerCameraManager->StartCameraFade(1.f, 0.f, Duration, FLinearColor::Black, false, false);
}

void APTBGameModeBase::ContinueStartGameFlow(TSubclassOf<APTBBaseMiniGame> Cls)
{
	// 4) 스폰
	ActiveMiniGame = SpawnMiniGame(Cls);
	if (!ActiveMiniGame)
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: SpawnMiniGame failed"));
		FadeInScreen(0.f);
		return;
	}

	ActiveMiniGame->OnMiniGameStarted.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
	ActiveMiniGame->OnMiniGameFinished.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);

	// 5) 미니게임 컨텍스트 구성 & 주입
	FPTBMiniGameContext Context;
	Context.SessionRequest = CurrentRequest;
	// Context.ChartData  → 채보 로딩 시스템에서 채워야 함 (DataTable / Asset 등)
	// Context.UserSettings → GameInstance에서 가져오기
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		Context.UserSettings = GI->CachedSettings;
	}
	Context.LocalPlayerIndex = 0;

	ActiveMiniGame->InitializeMiniGame(Context);

	// 6) Init에서 생성된 메시까지 포함해 텍스처를 고해상도 mip까지 미리 스트리밍
	TArray<UMeshComponent*> MeshComps;
	ActiveMiniGame->GetComponents<UMeshComponent>(MeshComps);
	for (UMeshComponent* Mesh : MeshComps)
	{
		Mesh->PrestreamTextures(2.0f, true);
	}

	// 7) 검은 화면에서 페이드인
	FadeInScreen(0.8f);

	UGameInstance* GameInstance = GetGameInstance();
	UPTBGameFlowSubsystem* FlowSubsystem = GameInstance
		? GameInstance->GetSubsystem<UPTBGameFlowSubsystem>()
		: nullptr;

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (FlowSubsystem && !CurrentLevelName.IsEmpty())
	{
		FlowSubsystem->CacheRetryTarget(CurrentRequest, FName(*CurrentLevelName));
	}

	PTB_RECORD(LogPTBMiniGames, TEXT("StartGameFlow: initialized [%s]"),
		*CurrentRequest.MiniGameId.ToString());
}

TSoftClassPtr<APTBBaseMiniGame> APTBGameModeBase::ResolveMiniGameClass(FName Id) const
{
	if (const TSoftClassPtr<APTBBaseMiniGame>* Found = MiniGameClassMap.Find(Id))
	{
		return *Found;
	}

	PTB_WARNING(LogPTBMiniGames, TEXT("ResolveMiniGameClass: No mapping for [%s]"), *Id.ToString());
	return nullptr;
}

APTBBaseMiniGame* APTBGameModeBase::SpawnMiniGame(TSubclassOf<APTBBaseMiniGame> Cls)
{
	UWorld* World = GetWorld();
	if (!Cls || !World)
	{
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APTBBaseMiniGame* Spawned = World->SpawnActor<APTBBaseMiniGame>(
		Cls, FTransform::Identity, Params);

	if (Spawned)
	{
		PTB_RECORD(LogPTBMiniGames, TEXT("SpawnMiniGame: [%s] spawned"), *Cls->GetName());
	}

	return Spawned;
}

void APTBGameModeBase::HandleMiniGameStarted()
{
	if (!ActiveMiniGame)
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("HandleMiniGameStarted: No active mini-game"));
		return;
	}

	bIsGameActive = true;
	bIsPaused = false;

	// FlowState 갱신
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentFlowState = EGameFlowState::InGame;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::InGame);
	}

	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}

	OnGameStarted.Broadcast();
}

void APTBGameModeBase::PauseGame()
{
	if (!bIsGameActive || bIsPaused) { return; }

	bIsPaused = true;

	// 미니게임 일시정지 (Conductor + 입력 잠금)
	if (ActiveMiniGame)
	{
		ActiveMiniGame->PauseMiniGame();
	}

	UGameplayStatics::SetGamePaused(this, true);

	// FlowState 갱신
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentFlowState = EGameFlowState::Paused;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::Paused);
	}

	OnGamePaused.Broadcast();

	ShowPauseMenu();
}

void APTBGameModeBase::ResumeGame()
{
	if (!bIsPaused) { return; }

	UGameplayStatics::SetGamePaused(this, false);

	bIsPaused = false;

	HidePauseMenu();

	if (ActiveMiniGame)
	{
		ActiveMiniGame->ResumeMiniGame();
	}

	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentFlowState = EGameFlowState::InGame;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::InGame);
	}

	OnGameResumed.Broadcast();
}

void APTBGameModeBase::HandleMiniGameFinished(FPTBRoundResult Result)
{
	SubmitRoundResult(Result);
}

void APTBGameModeBase::ShowPauseMenu()
{
	if (!PauseMenuClass) { return; }

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) { return; }

	if (!PauseMenuInstance)
	{
		PauseMenuInstance = CreateWidget<UPTBPauseMenuWidget>(PC, PauseMenuClass);
	}

	if (PauseMenuInstance)
	{
		PauseMenuInstance->OpenMenu();
	}
}

void APTBGameModeBase::HidePauseMenu()
{
	if (PauseMenuInstance && PauseMenuInstance->IsInViewport())
	{
		PauseMenuInstance->CloseMenu();
	}
}

void APTBGameModeBase::SubmitRoundResult(const FPTBRoundResult& Result)
{
	bIsGameActive = false;

	FPTBRewardSummary Reward;

	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		// 싱글 모드: 점수/별 갱신 + 저장 (PTBProfileSubsystem 위임)
		if (GI->CurrentPlayMode == EPTBPlayMode::Single)
		{
			if (UPTBProfileSubsystem* PS = GI->GetSubsystem<UPTBProfileSubsystem>())
			{
				// 진행도 반영 (내부에서 RequestSave 자동 호출)
				FPTBProfileProgressUpdate Update;
				Update.MiniGameId = Result.MiniGameId;
				Update.Difficulty = Result.Difficulty;
				Update.Score = Result.Score;
				Update.EarnedStars = Result.StarCount;
				Update.EarnedMoney = Result.EarnedMoney;
				PS->ApplyRoundResultToActive(Update);

				// 보상 요약 생성 (결과 화면용)
				bool bHasActive = false;
				const FPTBProfileData UpdatedProfile = PS->GetActiveProfile(bHasActive);
				Reward.EarnedMoney = Result.EarnedMoney;
				Reward.EarnedStars = Result.StarCount;
				Reward.TotalMoney = bHasActive ? UpdatedProfile.TotalEarnedMoney : Result.EarnedMoney;
				Reward.TotalStars = 0;
				if (bHasActive)
				{
					for (auto& Pair : UpdatedProfile.EarnedStarsByMiniGame)
						Reward.TotalStars += Pair.Value;
				}
			}
		}
		// 멀티 모드: 서버 검증 로직
		// else { ... }

		GI->AutoSave();
	}

	LastRoundResult = Result;
	LastRewardSummary = Reward;

	if (GI)
	{
		GI->LastRoundResult = Result;
		GI->LastRewardSummary = Reward;

		if (UPTBGameFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			FlowSubsystem->FinishGameplayWithReward(Result, Reward);
		}
		else
		{
			GI->CurrentFlowState = EGameFlowState::Result;
			GI->OnFlowStateChanged.Broadcast(EGameFlowState::Result);
		}
	}

	OnRoundResultReady.Broadcast(Result, Reward);
	OnGameEnded.Broadcast(Result);

	PTB_RECORD(LogPTBMiniGames, TEXT("SubmitRoundResult: Score=%d Grade=%d Stars=%d"),
		Result.Score, static_cast<int32>(Result.Grade), Result.StarCount);

	if (!ResultLevelName.IsNone())
	{
		UGameplayStatics::OpenLevel(this, ResultLevelName);
	}
}

void APTBGameModeBase::RetryGame_Implementation()
{
	UGameplayStatics::SetGamePaused(this, false);

	HidePauseMenu();

	// 기존 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->OnMiniGameStarted.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
		ActiveMiniGame->OnMiniGameFinished.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	bIsGameActive = false;
	bIsPaused = false;

	// 동일 요청으로 재시작 (클래스가 이미 로드돼 있으므로 즉시 진행)
	StartGameFlow(CurrentRequest);

}

void APTBGameModeBase::ExitToMenu_Implementation()
{
	UGameplayStatics::SetGamePaused(this, false);

	HidePauseMenu();

	// 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->OnMiniGameStarted.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
		ActiveMiniGame->OnMiniGameFinished.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);
		if (bIsGameActive)
		{
			ActiveMiniGame->FinishMiniGame(EPTBRoundEndReason::Aborted);
		}
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	bIsGameActive = false;
	bIsPaused = false;

	// FlowState 복원
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		if (UPTBGameFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			FlowSubsystem->ReturnToMiniGameSelect();
			return;
		}

		GI->CurrentFlowState = EGameFlowState::MiniGameSelect;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::MiniGameSelect);
	}

}

void APTBGameModeBase::GoToMainMenu_Implementation()
{
	HidePauseMenu();

	// 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->OnMiniGameStarted.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
		ActiveMiniGame->OnMiniGameFinished.RemoveDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);
		if (bIsGameActive)
		{
			ActiveMiniGame->FinishMiniGame(EPTBRoundEndReason::Aborted);
		}
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	bIsGameActive = false;
	bIsPaused = false;

	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		if (UPTBGameFlowSubsystem* FlowSubsystem = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			FlowSubsystem->GoToMainMenu();
			return;
		}

		GI->CurrentFlowState = EGameFlowState::MainMenu;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::MainMenu);
	}
}
// ── 허브 복귀 위치 ──────────────────────────────────────────────

bool APTBGameModeBase::IsHubLevel() const
{
	FName HubLevelName = TEXT("L_GameMap");
	if (const UGameInstance* GI = GetGameInstance())
	{
		if (const UPTBGameFlowSubsystem* Flow = GI->GetSubsystem<UPTBGameFlowSubsystem>())
		{
			if (!Flow->MiniGameSelectLevelName.IsNone())
			{
				HubLevelName = Flow->MiniGameSelectLevelName;
			}
		}
	}

	return UGameplayStatics::GetCurrentLevelName(this, true).Equals(HubLevelName.ToString(), ESearchCase::IgnoreCase);
}

APawn* APTBGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	bSpawnedAtHubReturnTransform = false;

	if (NewPlayer && NewPlayer->IsPlayerController() && IsHubLevel())
	{
		const UGameInstance* GI = GetGameInstance();
		const UPTBProfileSubsystem* PS = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;

		FTransform SavedTransform;
		if (PS && PS->GetHubReturnTransform(SavedTransform))
		{
			// 캐릭터는 Yaw만 유지 (기울어진 채로 스폰 방지)
			const FRotator SavedRotation(0.f, SavedTransform.Rotator().Yaw, 0.f);
			SavedTransform.SetRotation(SavedRotation.Quaternion());
			SavedTransform.SetScale3D(FVector::OneVector);

			// SpawnDefaultPawnAtTransform은 겹침 시 위치를 보정해서 스폰함
			if (APawn* Pawn = SpawnDefaultPawnAtTransform(NewPlayer, SavedTransform))
			{
				bSpawnedAtHubReturnTransform = true;
				HubReturnControlRotation = SavedRotation;
				PTB_RECORD(LogPTBFlow, TEXT("[PTBHub] 저장된 위치에서 스폰: %s"),
					*SavedTransform.GetLocation().ToString());
				return Pawn;
			}

			PTB_WARNING(LogPTBFlow, TEXT("[PTBHub] 저장 위치 스폰 실패, PlayerStart 사용"));
		}
	}

	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

void APTBGameModeBase::FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation)
{
	if (bSpawnedAtHubReturnTransform)
	{
		bSpawnedAtHubReturnTransform = false;
		Super::FinishRestartPlayer(NewPlayer, HubReturnControlRotation);
		return;
	}

	Super::FinishRestartPlayer(NewPlayer, StartRotation);
}

void APTBGameModeBase::StartHubPositionTracking()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FTimerManager& TM = World->GetTimerManager();

	if (HubPositionCacheInterval > 0.f)
	{
		TM.SetTimer(HubPositionCacheTimer,
			FTimerDelegate::CreateWeakLambda(this, [this]() { CacheHubPlayerTransform(false); }),
			HubPositionCacheInterval, true);
	}

	if (HubPositionDiskSaveInterval > 0.f)
	{
		TM.SetTimer(HubPositionDiskSaveTimer, this, &APTBGameModeBase::SaveHubPlayerTransformToDisk,
			HubPositionDiskSaveInterval, true);
	}
}

bool APTBGameModeBase::CacheHubPlayerTransform(bool bSaveToDisk)
{
	UGameInstance* GI = GetGameInstance();
	UPTBProfileSubsystem* PS = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	if (!PS)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const APlayerController* PC = World ? World->GetFirstPlayerController() : nullptr;
	const APawn* Pawn = PC ? PC->GetPawn() : nullptr;

	bool bCached = false;
	if (IsValid(Pawn))
	{
		// 점프·낙하 중 위치는 저장하지 않음 (공중 스폰·추락 방지)
		const ACharacter* Character = Cast<ACharacter>(Pawn);
		const bool bOnGround = !Character || !Character->GetCharacterMovement()
			|| Character->GetCharacterMovement()->IsMovingOnGround();

		if (bOnGround)
		{
			FTransform Transform = Pawn->GetActorTransform();
			Transform.SetRotation(FRotator(0.f, PC->GetControlRotation().Yaw, 0.f).Quaternion());
			PS->SetHubReturnTransform(Transform, false);
			bCached = true;
		}
	}

	// 이번에 기록 못 했어도 직전에 기록한 위치는 디스크에 저장
	if (bSaveToDisk)
	{
		PS->RequestSave();
	}

	return bCached;
}

void APTBGameModeBase::SaveHubPlayerTransformToDisk()
{
	CacheHubPlayerTransform(true);
}

void APTBGameModeBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 미니게임 진입(레벨 이동), 메인 메뉴 이동, 게임 종료 모두 여기로 들어옴
	if (IsHubLevel())
	{
		CacheHubPlayerTransform(true);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HubPositionCacheTimer);
		World->GetTimerManager().ClearTimer(HubPositionDiskSaveTimer);
	}

	Super::EndPlay(EndPlayReason);
}