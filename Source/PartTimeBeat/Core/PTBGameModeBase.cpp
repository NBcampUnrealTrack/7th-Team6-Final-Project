#include "Core/PTBGameModeBase.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Flow/PTBGameFlowSubsystem.h"
#include "Profile/PTBProfileSubsystem.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void APTBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

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

	TryStartDirectPIEMiniGame();
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

	APTBBaseMiniGame* DirectMiniGame = nullptr;
	for (AActor* Actor : MiniGameActors)
	{
		APTBBaseMiniGame* Candidate = Cast<APTBBaseMiniGame>(Actor);
		if (Candidate && Candidate->bAutoStartWhenOpenedDirectlyInPIE)
		{
			DirectMiniGame = Candidate;
			break;
		}
	}

	if (!DirectMiniGame)
	{
		return false;
	}

	if (!DirectMiniGame->RuleSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("Direct PIE MiniGame start failed: RuleSet is missing on [%s]."), *GetNameSafe(DirectMiniGame));
		return false;
	}

	if (DirectMiniGame->RuleSet->MiniGameId.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Direct PIE MiniGame start failed: RuleSet.MiniGameId is None on [%s]."), *GetNameSafe(DirectMiniGame));
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
			bool bHasActiveProfile = false;
			const FPTBProfileData ActiveProfile = ProfileSubsystem->GetActiveProfile(bHasActiveProfile);
			if (bHasActiveProfile)
			{
				CurrentRequest.ProfileId = ActiveProfile.ProfileId;
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

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (FlowSubsystem && !CurrentLevelName.IsEmpty())
	{
		FlowSubsystem->CacheRetryTarget(CurrentRequest, FName(*CurrentLevelName));
	}

	UE_LOG(LogTemp, Log, TEXT("Direct PIE MiniGame initialized [%s] from placed actor [%s]."),
		*CurrentRequest.MiniGameId.ToString(),
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
	}

	// 1) 미니게임 클래스 해석
	TSubclassOf<APTBBaseMiniGame> Cls = ResolveMiniGameClass(Request.MiniGameId);
	if (!Cls)
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: MiniGame class not found for [%s]"),
			*Request.MiniGameId.ToString());
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

	// 3) 스폰
	ActiveMiniGame = SpawnMiniGame(Cls);
	if (!ActiveMiniGame)
	{
		PTB_ERROR(LogPTBMiniGames, TEXT("StartGameFlow: SpawnMiniGame failed"));
		return;
	}

	ActiveMiniGame->OnMiniGameStarted.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameStarted);
	ActiveMiniGame->OnMiniGameFinished.AddUniqueDynamic(this, &APTBGameModeBase::HandleMiniGameFinished);

	// 4) 미니게임 컨텍스트 구성 & 주입
	FPTBMiniGameContext Context;
	Context.SessionRequest = Request;
	// Context.ChartData  → 채보 로딩 시스템에서 채워야 함 (DataTable / Asset 등)
	// Context.UserSettings → GameInstance에서 가져오기
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		Context.UserSettings = GI->CachedSettings;
	}
	Context.LocalPlayerIndex = 0;

	ActiveMiniGame->InitializeMiniGame(Context);

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (FlowSubsystem && !CurrentLevelName.IsEmpty())
	{
		FlowSubsystem->CacheRetryTarget(CurrentRequest, FName(*CurrentLevelName));
	}

	PTB_RECORD(LogPTBMiniGames, TEXT("StartGameFlow: initialized [%s]"),
		*Request.MiniGameId.ToString());
}

TSubclassOf<APTBBaseMiniGame> APTBGameModeBase::ResolveMiniGameClass(FName Id) const
{
	if (const TSubclassOf<APTBBaseMiniGame>* Found = MiniGameClassMap.Find(Id))
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
				Update.MiniGameId  = Result.MiniGameId;
				Update.Difficulty  = Result.Difficulty;
				Update.Score       = Result.Score;
				Update.EarnedStars = Result.StarCount;
				Update.EarnedMoney = Result.EarnedMoney;
				PS->ApplyRoundResultToActive(Update);

				// 보상 요약 생성 (결과 화면용)
				bool bHasActive = false;
				const FPTBProfileData UpdatedProfile = PS->GetActiveProfile(bHasActive);
				Reward.EarnedMoney = Result.EarnedMoney;
				Reward.EarnedStars = Result.StarCount;
				Reward.TotalMoney  = bHasActive ? UpdatedProfile.TotalEarnedMoney : Result.EarnedMoney;
				Reward.TotalStars  = 0;
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

	// 동일 요청으로 재시작
	StartGameFlow(CurrentRequest);

}

void APTBGameModeBase::ExitToMenu_Implementation()
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
