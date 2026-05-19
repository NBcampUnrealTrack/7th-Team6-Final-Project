#include "Core/PTBGameFlowSubsystem.h"

#include "Core/PTBGameInstance.h"
#include "Debug/PTBLogChannels.h"

namespace
{
	FString GetFlowStateName(EGameFlowState State)
	{
		const UEnum* FlowStateEnum = StaticEnum<EGameFlowState>();
		return FlowStateEnum
			? FlowStateEnum->GetNameStringByValue(static_cast<int64>(State))
			: TEXT("Unknown");
	}
	
	FString GetPlayModeName(EPTBPlayMode Mode)
	{
		const UEnum* PlayModeEnum = StaticEnum<EPTBPlayMode>();
		return PlayModeEnum
			? PlayModeEnum->GetNameStringByValue(static_cast<int64>(Mode))
			: TEXT("Unknown");
	}

	FString GetDifficultyName(EPTBDifficulty Difficulty)
	{
		const UEnum* DifficultyEnum = StaticEnum<EPTBDifficulty>();
		return DifficultyEnum
			? DifficultyEnum->GetNameStringByValue(static_cast<int64>(Difficulty))
			: TEXT("Unknown");
	}
}

void UPTBGameFlowSubsystem::SetFlowState(EGameFlowState NewState)
{
	if (CurrentFlowState == NewState)
	{
		UE_LOG(LogFlow, Verbose, TEXT("[PTBFlow] Ignore duplicated flow state: %s"),
					*GetFlowStateName(NewState));
		return;
	}
	
	const EGameFlowState OldState = CurrentFlowState;

	PreviousFlowState = CurrentFlowState;
	CurrentFlowState = NewState;

	
	// TODO(Refactor): UPTBGameInstance 접근 방식이 정리되면
	// GetGameInstance<UPTBGameInstance>() 또는 공용 헬퍼로 교체 필요
	UGameInstance* GI = GetGameInstance();
	UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI);
	
	if (PTBGI)
	{
		PTBGI->CurrentFlowState = NewState;
		PTBGI->OnFlowStateChanged.Broadcast(NewState);
	}
	else
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SetFlowState: PTBGameInstance is not available."));
	}
	
	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Flow state changed: %s -> %s"),
		*GetFlowStateName(OldState),
		*GetFlowStateName(NewState));
}

bool UPTBGameFlowSubsystem::SelectProfile(const FString& ProfileId) 
{
	if (ProfileId.IsEmpty())
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SelectProfile failed: ProfileId is empty."));
		return false;
	}
	
	// TODO(Refactor): UPTBGameInstance 접근 방식이 정리되면
	// GetGameInstance<UPTBGameInstance>() 또는 공용 헬퍼로 교체 필요
	UGameInstance* GI = GetGameInstance();
	UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI);
	
	if (!PTBGI)
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SelectProfile failed: PTBGameInstance is not available."));
		return false;
	}
	
	// TODO(Integration): ProfileSubsystem이 안정화되면 프로필 유효성 검사를 그쪽으로 위임
	if (!PTBGI->LoadProfile(ProfileId))
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SelectProfile failed: ProfileId=%s"), *ProfileId);
		return false;
	}

	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Profile selected: %s"), *ProfileId);

	SetFlowState(EGameFlowState::ModeSelect);
	return true;
}

void UPTBGameFlowSubsystem::SelectPlayMode(EPTBPlayMode InMode) 
{
	// TODO(Refactor): UPTBGameInstance 접근 방식이 정리되면
	// GetGameInstance<UPTBGameInstance>() 또는 공용 헬퍼로 교체 필요
	UGameInstance* GI = GetGameInstance();
	UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI);

	if (PTBGI)
	{
		PTBGI->CurrentPlayMode = InMode;
	}
	else
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SelectPlayMode: PTBGameInstance is not available. Continue with local flow only."));
	}

	UE_LOG(LogFlow, Log, 
		TEXT("[PTBFlow] Play mode selected: %s"), *GetPlayModeName(InMode));

	if (InMode == EPTBPlayMode::Single)
	{
		SetFlowState(EGameFlowState::MiniGameSelect);
		return;
	}

	if (InMode == EPTBPlayMode::Multiplayer)
	{
		// TODO(Multiplayer): SessionSubsystem/LobbyManager 연동 후 구현
		UE_LOG(LogFlow, Warning, TEXT("[PTBFlow] Multiplayer flow is not implemented yet."));
		SetFlowState(EGameFlowState::MultiLobby);
		return;
	}

	UE_LOG(LogFlow, Warning, 
		TEXT("[PTBFlow] SelectPlayMode failed: Unsupported play mode."));
}

bool UPTBGameFlowSubsystem::SelectMiniGame(FName InId) 
{
	if (InId.IsNone())
	{
		UE_LOG(LogFlow, Warning, TEXT("[PTBFlow] SelectMiniGame failed: MiniGameId is None."));
		return false;
	}

	// TODO(Progression): StageUnlockManager::CanPlayMiniGame(InId)로 해금 여부 검사
	SelectedMiniGameId = InId;

	UE_LOG(LogFlow, Log, 
		TEXT("[PTBFlow] MiniGame selected: %s"), *SelectedMiniGameId.ToString());

	SetFlowState(EGameFlowState::DifficultySelect);
	return true;
}

void UPTBGameFlowSubsystem::SelectDifficulty(EPTBDifficulty InDiff)
{
	if (SelectedMiniGameId.IsNone())
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] SelectDifficulty failed: MiniGame is not selected."));
		return;
	}

	SelectedDifficulty = InDiff;

	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Difficulty selected: %s"),
		*GetDifficultyName(SelectedDifficulty));

	// TODO(Tutorial): TutorialManager::ShouldShowTutorial() 연동 후 첫 플레이면 Tutorial 상태로 전환
	StartGameplay();
}

void UPTBGameFlowSubsystem::StartGameplay() 
{
	if (SelectedMiniGameId.IsNone())
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] StartGameplay failed: MiniGame is not selected."));
		return;
	}

	// TODO(Refactor): UPTBGameInstance 접근 방식이 정리되면
	// GetGameInstance<UPTBGameInstance>() 또는 공용 헬퍼로 교체 필요
	UGameInstance* GI = GetGameInstance();
	UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI);

	PendingSessionRequest = FPTBGameSessionRequest();
	PendingSessionRequest.MiniGameId = SelectedMiniGameId;
	PendingSessionRequest.MiniGameCode = SelectedMiniGameId;
	PendingSessionRequest.Difficulty = SelectedDifficulty;
	PendingSessionRequest.PlayMode = PTBGI ? PTBGI->CurrentPlayMode : EPTBPlayMode::Single;
	PendingSessionRequest.RandomSeed = FMath::Rand();
	PendingSessionRequest.ExpectedPlayerCount = 1;

	if (PTBGI)
	{
		PendingSessionRequest.ProfileId = PTBGI->ActiveProfile.ProfileId;
	}
	else
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] StartGameplay: PTBGameInstance is not available. ProfileId remains default."));
	}

	if (PendingSessionRequest.PlayMode == EPTBPlayMode::Multiplayer)
	{
		// TODO(Multiplayer): Multiplayer 로직은 추후 구현
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] StartGameplay blocked: Multiplayer is not implemented yet."));
		return;
	}

	UE_LOG(LogFlow, Log,
		TEXT("[PTBFlow] Gameplay request created. Profile=%s MiniGame=%s Difficulty=%s PlayMode=%s Seed=%d"),
		*PendingSessionRequest.ProfileId.ToString(),
		*PendingSessionRequest.MiniGameId.ToString(),
		*GetDifficultyName(PendingSessionRequest.Difficulty),
		*GetPlayModeName(PendingSessionRequest.PlayMode),
		PendingSessionRequest.RandomSeed);

	// TODO(Integration): GameMode 라운드 시작 API가 안정화되면
	// APTBGameModeBase::StartGameFlow(PendingSessionRequest)를 호출
	SetFlowState(EGameFlowState::InGame);
}

void UPTBGameFlowSubsystem::FinishGameplay(const FPTBRoundResult & Result)
{
	if (Result.MiniGameId.IsNone())
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] FinishGameplay received result with empty MiniGameId."));
	}

	UE_LOG(LogFlow, Log,
		TEXT("[PTBFlow] Gameplay finished. Profile=%s MiniGame=%s Score=%d Stars=%d Grade=%d"),
		*Result.ProfileId.ToString(),
		*Result.MiniGameId.ToString(),
		Result.Score,
		Result.StarCount,
		static_cast<uint8>(Result.Grade));

	// TODO(Progression): StageUnlockManager::ApplyRoundResult(Result)로 보상/해금 반영
	// TODO(Story): StoryManager::CheckUnlockConditions(Result)로 결과 후 스토리 여부 판단
	// TODO(Save): 저장 정책 확정 후 GameInstance::AutoSave() 호출

	SetFlowState(EGameFlowState::Result);
}

void UPTBGameFlowSubsystem::ReturnToMiniGameSelect() 
{
	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Return to MiniGameSelect."));

	SetFlowState(EGameFlowState::MiniGameSelect);
}

void UPTBGameFlowSubsystem::OpenSettings() 
{
	if (CurrentFlowState == EGameFlowState::Settings)
	{
		UE_LOG(LogFlow, Verbose, TEXT("[PTBFlow] OpenSettings ignored: already in Settings."));
		return;
	}

	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Open Settings from %s."),
		*GetFlowStateName(CurrentFlowState));

	SetFlowState(EGameFlowState::Settings);
}

void UPTBGameFlowSubsystem::CloseSettings() 
{
	if (CurrentFlowState != EGameFlowState::Settings)
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] CloseSettings ignored: current state is %s."),
			*GetFlowStateName(CurrentFlowState));
		return;
	}

	if (PreviousFlowState == EGameFlowState::Settings)
	{
		UE_LOG(LogFlow, Warning, 
			TEXT("[PTBFlow] CloseSettings fallback: previous state is also Settings."));
		SetFlowState(EGameFlowState::MainMenu);
		return;
	}
	
	UE_LOG(LogFlow, Log, TEXT("[PTBFlow] Close Settings. Return to %s."),
		*GetFlowStateName(PreviousFlowState));

	SetFlowState(PreviousFlowState);
}