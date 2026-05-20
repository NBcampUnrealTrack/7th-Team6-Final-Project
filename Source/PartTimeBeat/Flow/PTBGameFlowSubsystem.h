#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core/PTBStructEnums.h"
#include "PTBGameFlowSubsystem.generated.h"

UCLASS()
class PARTTIMEBEAT_API UPTBGameFlowSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	//	상태 전환 단일 진입점
	void SetFlowState(EGameFlowState NewState);
	//	프로필 선택 → ModeSelect
	bool SelectProfile(const FString& ProfileId);
	//	Single → MiniGameSelect / Multi → MultiLobby
	void SelectPlayMode(EPTBPlayMode InMode);
	//	해금 확인 후 DifficultySelect
	bool SelectMiniGame(FName InId);
	//	튜토리얼 체크 → InGame 또는 Tutorial
	void SelectDifficulty(EPTBDifficulty InDiff);
	//	PendingSessionRequest 생성 후 GameMode 전달
	void StartGameplay();
	//	진행도 적용 → Result 화면
	void FinishGameplay(const FPTBRoundResult& Result);
	//	결과 후 복귀
	void ReturnToMiniGameSelect();
	// 설정 진입
	void OpenSettings();
	// 설정 이탈
	void CloseSettings();


	/**현재 상태 */
	EGameFlowState CurrentFlowState;
	/**직전 상태(뒤로가기용) */
	EGameFlowState PreviousFlowState;
	/** 현재 선택 미니게임 */
	FName SelectedMiniGameId;
	/** 현재 선택 난이도 */
	EPTBDifficulty SelectedDifficulty;
	/** GameMode로 넘길 요청 */
	FPTBGameSessionRequest PendingSessionRequest;
};
