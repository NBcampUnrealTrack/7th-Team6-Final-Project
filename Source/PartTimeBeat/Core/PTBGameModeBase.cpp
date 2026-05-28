#include "Core/PTBGameModeBase.h"
#include "Core/PTBGameInstance.h"
#include "Profile/PTBProfileSubsystem.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

void APTBGameModeBase::StartGameFlow(const FPTBGameSessionRequest& Request)
{
	CurrentRequest = Request;
	bIsGameActive = false;
	bIsPaused = false;

	// 1) 미니게임 클래스 해석
	TSubclassOf<APTBBaseMiniGame> Cls = ResolveMiniGameClass(Request.MiniGameId);
	if (!Cls)
	{
		UE_LOG(LogTemp, Error, TEXT("StartGameFlow: MiniGame class not found for [%s]"),
			*Request.MiniGameId.ToString());
		return;
	}

	// 2) 기존 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	// 3) 스폰
	ActiveMiniGame = SpawnMiniGame(Cls);
	if (!ActiveMiniGame)
	{
		UE_LOG(LogTemp, Error, TEXT("StartGameFlow: SpawnMiniGame failed"));
		return;
	}

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

	// 5) 카운트다운 후 BeginRound
	if (CountdownSeconds <= 0) { CountdownSeconds = 3; }

	FTimerHandle CountdownHandle;
	GetWorldTimerManager().SetTimer(
		CountdownHandle,
		this,
		&APTBGameModeBase::BeginRound,
		static_cast<float>(CountdownSeconds),
		false // 반복 안 함
	);

	UE_LOG(LogTemp, Log, TEXT("StartGameFlow: countdown %d sec → [%s]"),
		CountdownSeconds, *Request.MiniGameId.ToString());
}

TSubclassOf<APTBBaseMiniGame> APTBGameModeBase::ResolveMiniGameClass(FName Id) const
{
	if (const TSubclassOf<APTBBaseMiniGame>* Found = MiniGameClassMap.Find(Id))
	{
		return *Found;
	}

	UE_LOG(LogTemp, Warning, TEXT("ResolveMiniGameClass: No mapping for [%s]"), *Id.ToString());
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
		UE_LOG(LogTemp, Log, TEXT("SpawnMiniGame: [%s] spawned"), *Cls->GetName());
	}

	return Spawned;
}

void APTBGameModeBase::BeginRound()
{
	if (!ActiveMiniGame)
	{
		UE_LOG(LogTemp, Error, TEXT("BeginRound: No active mini-game"));
		return;
	}

	bIsGameActive = true;
	bIsPaused = false;

	// BGM + Conductor 시작, 입력 허용
	ActiveMiniGame->StartMiniGame();

	// FlowState 갱신
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentFlowState = EGameFlowState::InGame;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::InGame);
	}

	OnGameStarted.Broadcast();

	UE_LOG(LogTemp, Log, TEXT("BeginRound: Game started"));
}

void APTBGameModeBase::PauseGame()
{
if (bIsPaused) { return; }

	bIsPaused = true;

	// 미니게임 일시정지 (Conductor + 입력 잠금)
	if (ActiveMiniGame)
	{
		ActiveMiniGame->PauseMiniGame();
	}

	// 엔진 레벨 일시정지 (선택사항 — UI 애니메이션은 살려야 할 수 있음)
	// UGameplayStatics::SetGamePaused(GetWorld(), true);

	// FlowState 갱신
	UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance());
	if (GI)
	{
		GI->CurrentFlowState = EGameFlowState::Paused;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::Paused);
	}

	OnGamePaused.Broadcast();

	ShowPauseMenu();

	UE_LOG(LogTemp, Log, TEXT("PauseGame"));
}

void APTBGameModeBase::ResumeGame()
{
	if (!bIsPaused) { return; }

	bIsPaused = false;

	HidePauseMenu();

	// UGameplayStatics::SetGamePaused(GetWorld(), false);

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

	UE_LOG(LogTemp, Log, TEXT("ResumeGame"));
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
				Update.Score       = Result.Score;
				Update.EarnedStars = Result.StarCount;
				Update.EarnedMoney = Result.EarnedMoney;
				PS->ApplyRoundResultToActive(Update);

				// 보상 요약 생성 (결과 화면용)
				bool bHasActive = false;
				const FPTBProfileData UpdatedProfile = PS->GetActiveProfile(bHasActive);

				FPTBRewardSummary Reward;
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

		// 결과 화면으로 전환
		GI->CurrentFlowState = EGameFlowState::Result;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::Result);
	}

	OnGameEnded.Broadcast(Result);

	UE_LOG(LogTemp, Log, TEXT("SubmitRoundResult: Score=%d Grade=%d Stars=%d"),
		Result.Score, static_cast<int32>(Result.Grade), Result.StarCount);
}

void APTBGameModeBase::RetryGame_Implementation()
{
	HidePauseMenu();

	// 기존 미니게임 정리
	if (ActiveMiniGame)
	{
		ActiveMiniGame->Destroy();
		ActiveMiniGame = nullptr;
	}

	bIsGameActive = false;
	bIsPaused = false;

	// 동일 요청으로 재시작
	StartGameFlow(CurrentRequest);

	UE_LOG(LogTemp, Log, TEXT("RetryGame: Restarting [%s]"),
		*CurrentRequest.MiniGameId.ToString());
}

void APTBGameModeBase::ExitToMenu_Implementation()
{
	HidePauseMenu();

	// 미니게임 정리
	if (ActiveMiniGame)
	{
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
		GI->CurrentFlowState = EGameFlowState::MiniGameSelect;
		GI->OnFlowStateChanged.Broadcast(EGameFlowState::MiniGameSelect);
	}

	// 레벨 전환 (맵 이름은 프로젝트에 맞게 수정)
	// UGameplayStatics::OpenLevel(this, FName("MainMenuMap"));

	UE_LOG(LogTemp, Log, TEXT("ExitToMenu"));
}