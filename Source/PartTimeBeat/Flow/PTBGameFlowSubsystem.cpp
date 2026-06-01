#include "Flow/PTBGameFlowSubsystem.h"

#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Profile/PTBProfileSubsystem.h"

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
		PTB_VERBOSE(LogPTBFlow, TEXT("[PTBFlow] 중복 흐름 상태 무시: %s"),
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
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] SetFlowState: PTBGameInstance를 사용할 수 없습니다."));
	}
	
	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 흐름 상태 변경: %s -> %s"),
		*GetFlowStateName(OldState),
		*GetFlowStateName(NewState));
}

bool UPTBGameFlowSubsystem::SelectProfile(const FString& ProfileId) 
{
	if (ProfileId.IsEmpty())
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] SelectProfile 실패: ProfileId가 비어 있습니다."));
		return false;
	}
	
	// TODO(Refactor): UPTBGameInstance 접근 방식이 정리되면
	// GetGameInstance<UPTBGameInstance>() 또는 공용 헬퍼로 교체 필요
	UGameInstance* GI = GetGameInstance();
	UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GI);
	
	if (!PTBGI)
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] SelectProfile 실패: PTBGameInstance를 사용할 수 없습니다."));
		return false;
	}
	
	// PTBProfileSubsystem을 통해 활성 프로필 설정
	UPTBProfileSubsystem* PS = PTBGI->GetSubsystem<UPTBProfileSubsystem>();
	if (!PS)
	{
		PTB_WARNING(LogPTBFlow, TEXT("[PTBFlow] SelectProfile 실패: ProfileSubsystem 없음"));
		return false;
	}

	FGuid ProfileGuid;
	if (!FGuid::Parse(ProfileId, ProfileGuid) || !PS->SetActiveProfile(ProfileGuid))
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] SelectProfile 실패: ProfileId=%s"), *ProfileId);
		return false;
	}

	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 프로필 선택: %s"), *ProfileId);

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
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] SelectPlayMode: PTBGameInstance를 사용할 수 없습니다. 로컬 흐름만 계속 진행합니다."));
	}

	PTB_RECORD(LogPTBFlow, 
		TEXT("[PTBFlow] 플레이 모드 선택: %s"), *GetPlayModeName(InMode));

	if (InMode == EPTBPlayMode::Single)
	{
		SetFlowState(EGameFlowState::MiniGameSelect);
		return;
	}

	if (InMode == EPTBPlayMode::Multiplayer)
	{
		// TODO(Multiplayer): SessionSubsystem/LobbyManager 연동 후 구현
		PTB_WARNING(LogPTBFlow, TEXT("[PTBFlow] 멀티플레이 흐름은 아직 구현되지 않았습니다."));
		SetFlowState(EGameFlowState::MultiLobby);
		return;
	}

	PTB_WARNING(LogPTBFlow, 
		TEXT("[PTBFlow] SelectPlayMode 실패: 지원하지 않는 플레이 모드입니다."));
}

bool UPTBGameFlowSubsystem::SelectMiniGame(FName InId) 
{
	if (InId.IsNone())
	{
		PTB_WARNING(LogPTBFlow, TEXT("[PTBFlow] SelectMiniGame 실패: MiniGameId가 None입니다."));
		return false;
	}

	// TODO(Progression): StageUnlockManager::CanPlayMiniGame(InId)로 해금 여부 검사
	SelectedMiniGameId = InId;

	PTB_RECORD(LogPTBFlow, 
		TEXT("[PTBFlow] 미니게임 선택: %s"), *SelectedMiniGameId.ToString());

	SetFlowState(EGameFlowState::DifficultySelect);
	return true;
}

void UPTBGameFlowSubsystem::SelectDifficulty(EPTBDifficulty InDiff)
{
	if (SelectedMiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] SelectDifficulty 실패: 미니게임이 선택되지 않았습니다."));
		return;
	}

	SelectedDifficulty = InDiff;

	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 난이도 선택: %s"),
		*GetDifficultyName(SelectedDifficulty));

	// TODO(Tutorial): TutorialManager::ShouldShowTutorial() 연동 후 첫 플레이면 Tutorial 상태로 전환
	StartGameplay();
}

void UPTBGameFlowSubsystem::StartGameplay() 
{
	if (SelectedMiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] StartGameplay 실패: 미니게임이 선택되지 않았습니다."));
		return;
	}

	ClearRoundResult();

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
		if (UPTBProfileSubsystem* PS = PTBGI->GetSubsystem<UPTBProfileSubsystem>())
		{
			bool bHasActive = false;
			const FPTBProfileData ActiveProfile = PS->GetActiveProfile(bHasActive);
			if (bHasActive)
				PendingSessionRequest.ProfileId = ActiveProfile.ProfileId;
		}
	}
	else
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] StartGameplay: PTBGameInstance를 사용할 수 없습니다. ProfileId는 기본값으로 유지됩니다."));
	}

	if (PendingSessionRequest.PlayMode == EPTBPlayMode::Multiplayer)
	{
		// TODO(Multiplayer): Multiplayer 로직은 추후 구현
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] StartGameplay 차단: 멀티플레이는 아직 구현되지 않았습니다."));
		return;
	}

	PTB_RECORD(LogPTBFlow,
		TEXT("[PTBFlow] 게임플레이 요청 생성. 프로필=%s 미니게임=%s 난이도=%s 플레이모드=%s 시드=%d"),
		*PendingSessionRequest.ProfileId.ToString(),
		*PendingSessionRequest.MiniGameId.ToString(),
		*GetDifficultyName(PendingSessionRequest.Difficulty),
		*GetPlayModeName(PendingSessionRequest.PlayMode),
		PendingSessionRequest.RandomSeed);

	// TODO(Integration): GameMode 라운드 시작 API가 안정화되면
	// APTBGameModeBase::StartGameFlow(PendingSessionRequest)를 호출
	SetFlowState(EGameFlowState::InGame);
}

void UPTBGameFlowSubsystem::FinishGameplay(const FPTBRoundResult& Result)
{
	StoreRoundResult(Result, FPTBRewardSummary());

	if (Result.MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] FinishGameplay: MiniGameId가 비어 있는 결과를 받았습니다."));
	}

	PTB_RECORD(LogPTBFlow,
		TEXT("[PTBFlow] 게임플레이 종료. 프로필=%s 미니게임=%s 점수=%d 별=%d 등급=%d"),
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

void UPTBGameFlowSubsystem::StoreRoundResult(const FPTBRoundResult& Result, const FPTBRewardSummary& Reward)
{
	LastRoundResult = Result;
	LastRewardSummary = Reward;
	bHasRoundResult = true;
}

void UPTBGameFlowSubsystem::ClearRoundResult()
{
	LastRoundResult = FPTBRoundResult();
	LastRewardSummary = FPTBRewardSummary();
	bHasRoundResult = false;
}

bool UPTBGameFlowSubsystem::HasRoundResult() const
{
	return bHasRoundResult;
}

FPTBRoundResult UPTBGameFlowSubsystem::GetLastRoundResult() const
{
	return LastRoundResult;
}

FPTBRewardSummary UPTBGameFlowSubsystem::GetLastRewardSummary() const
{
	return LastRewardSummary;
}

void UPTBGameFlowSubsystem::ReturnToMiniGameSelect() 
{
	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 미니게임 선택 화면으로 돌아갑니다."));

	ClearRetryTarget();

	SetFlowState(EGameFlowState::MiniGameSelect);

	if (MiniGameSelectLevelName.IsNone())
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] ReturnToMiniGameSelect 실패: 미니게임 선택 화면 맵 이름이 없습니다."));
		return;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (CurrentLevelName.Equals(MiniGameSelectLevelName.ToString(), ESearchCase::CaseSensitive))
	{
		PTB_VERBOSE(LogPTBFlow,
			TEXT("[PTBFlow] ReturnToMiniGameSelect 무시: 이미 미니게임 선택 화면 맵입니다."));
		return;
	}

	UGameplayStatics::OpenLevel(this, MiniGameSelectLevelName);
}

bool UPTBGameFlowSubsystem::RetryLastGame()
{
	if (LastSessionRequest.MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] RetryLastGame 실패: 마지막 세션 요청이 없습니다."));
		return false;
	}

	if (LastPlayedMiniGameLevelName.IsNone())
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] RetryLastGame 실패: 마지막 미니게임 맵 이름이 없습니다."));
		return false;
	}

	SelectedMiniGameId = LastSessionRequest.MiniGameId;
	SelectedDifficulty = LastSessionRequest.Difficulty;
	PendingSessionRequest = LastSessionRequest;

	if (UPTBGameInstance* PTBGI = Cast<UPTBGameInstance>(GetGameInstance()))
	{
		PTBGI->CurrentPlayMode = LastSessionRequest.PlayMode;
	}

	PTB_RECORD(LogPTBFlow,
		TEXT("[PTBFlow] 마지막 게임 재시작: 미니게임=%s 맵=%s 난이도=%s"),
		*LastSessionRequest.MiniGameId.ToString(),
		*LastPlayedMiniGameLevelName.ToString(),
		*GetDifficultyName(LastSessionRequest.Difficulty));

	SetFlowState(EGameFlowState::InGame);
	UGameplayStatics::OpenLevel(this, LastPlayedMiniGameLevelName);
	return true;
}

void UPTBGameFlowSubsystem::CacheRetryTarget(const FPTBGameSessionRequest& SessionRequest, FName LevelName)
{
	if (SessionRequest.MiniGameId.IsNone())
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] CacheRetryTarget 실패: MiniGameId가 None입니다."));
		return;
	}

	if (LevelName.IsNone())
	{
		PTB_WARNING(LogPTBFlow,
			TEXT("[PTBFlow] CacheRetryTarget 실패: LevelName이 None입니다."));
		return;
	}

	LastSessionRequest = SessionRequest;
	LastPlayedMiniGameLevelName = LevelName;

	PTB_RECORD(LogPTBFlow,
		TEXT("[PTBFlow] 재시작 대상 저장: 미니게임=%s 맵=%s 난이도=%s"),
		*LastSessionRequest.MiniGameId.ToString(),
		*LastPlayedMiniGameLevelName.ToString(),
		*GetDifficultyName(LastSessionRequest.Difficulty));
}

void UPTBGameFlowSubsystem::ClearRetryTarget()
{
	LastSessionRequest = FPTBGameSessionRequest();
	LastPlayedMiniGameLevelName = NAME_None;

	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 재시작 대상 초기화"));
}

bool UPTBGameFlowSubsystem::HasRetryTarget() const
{
	return !LastSessionRequest.MiniGameId.IsNone() && !LastPlayedMiniGameLevelName.IsNone();
}

void UPTBGameFlowSubsystem::OpenSettings() 
{
	if (CurrentFlowState == EGameFlowState::Settings)
	{
		PTB_VERBOSE(LogPTBFlow, TEXT("[PTBFlow] OpenSettings 무시: 이미 설정 화면입니다."));
		return;
	}

	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 설정 화면 열기. 이전 상태=%s"),
		*GetFlowStateName(CurrentFlowState));

	SetFlowState(EGameFlowState::Settings);
}

void UPTBGameFlowSubsystem::CloseSettings() 
{
	if (CurrentFlowState != EGameFlowState::Settings)
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] CloseSettings 무시: 현재 상태=%s"),
			*GetFlowStateName(CurrentFlowState));
		return;
	}

	if (PreviousFlowState == EGameFlowState::Settings)
	{
		PTB_WARNING(LogPTBFlow, 
			TEXT("[PTBFlow] CloseSettings 대체 처리: 이전 상태도 설정 화면입니다."));
		SetFlowState(EGameFlowState::MainMenu);
		return;
	}
	
	PTB_RECORD(LogPTBFlow, TEXT("[PTBFlow] 설정 화면 닫기. 복귀 상태=%s"),
		*GetFlowStateName(PreviousFlowState));

	SetFlowState(PreviousFlowState);
}
