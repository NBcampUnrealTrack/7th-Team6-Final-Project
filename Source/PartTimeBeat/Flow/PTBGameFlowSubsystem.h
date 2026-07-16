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
	/** 상태 전환 단일 진입점 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void SetFlowState(EGameFlowState NewState);

	/** 프로필 선택 → ModeSelect */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	bool SelectProfile(const FString& ProfileId);

	/** Single → MiniGameSelect / Multi → MultiLobby */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void SelectPlayMode(EPTBPlayMode InMode);

	/** 해금 확인 후 DifficultySelect */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	bool SelectMiniGame(FName InId);

	/** 튜토리얼 체크 → InGame 또는 Tutorial */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void SelectDifficulty(EPTBDifficulty InDiff);

	/** PendingSessionRequest 생성 후 GameMode 전달 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void StartGameplay();

	/** 진행도 적용 → Result 화면 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void FinishGameplay(const FPTBRoundResult& Result);

	/** 진행도와 보상 요약 적용 → Result 화면 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void FinishGameplayWithReward(const FPTBRoundResult& Result, const FPTBRewardSummary& Reward);

	/** 결과 화면에서 표시할 마지막 라운드 결과 저장 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void StoreRoundResult(const FPTBRoundResult& Result, const FPTBRewardSummary& Reward);

	/** 저장된 마지막 라운드 결과 초기화 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void ClearRoundResult();

	/** 결과 화면에서 표시할 라운드 결과가 있는지 여부 */
	UFUNCTION(BlueprintPure, Category = "PTB|Flow")
	bool HasRoundResult() const;

	/** 결과 화면에서 표시할 마지막 라운드 결과 */
	UFUNCTION(BlueprintPure, Category = "PTB|Flow")
	FPTBRoundResult GetLastRoundResult() const;

	/** 결과 화면에서 표시할 마지막 보상 요약 */
	UFUNCTION(BlueprintPure, Category = "PTB|Flow")
	FPTBRewardSummary GetLastRewardSummary() const;

	/** 결과 후 미니게임 선택 화면으로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void ReturnToMiniGameSelect();

	/** 메인 타이틀 화면으로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void GoToMainMenu();

	/** 결과 화면에서 마지막 플레이를 다시 시작 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	bool RetryLastGame();

	/** 현재 라운드의 재시작 대상을 저장 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void CacheRetryTarget(const FPTBGameSessionRequest& SessionRequest, FName LevelName);

	/** 저장된 재시작 대상을 초기화 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void ClearRetryTarget();

	/** 결과 화면에서 재시작 가능 여부 */
	UFUNCTION(BlueprintPure, Category = "PTB|Flow")
	bool HasRetryTarget() const;

	/** 설정 화면 진입 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void OpenSettings();

	/** 설정 화면 이탈, 이전 상태로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void CloseSettings();

	/** 현재 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	EGameFlowState CurrentFlowState;
	/** 직전 상태(뒤로가기용) */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	EGameFlowState PreviousFlowState;
	/** 현재 선택 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FName SelectedMiniGameId;
	/** 현재 선택 난이도 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	EPTBDifficulty SelectedDifficulty;
	/** GameMode로 넘길 요청 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FPTBGameSessionRequest PendingSessionRequest;
	/** 결과 화면에서 다시 진입할 마지막 미니게임 맵 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FName LastPlayedMiniGameLevelName;
	/** 결과 화면에서 다시 사용할 마지막 세션 요청 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FPTBGameSessionRequest LastSessionRequest;
	/** 결과 화면에서 표시할 마지막 라운드 결과 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FPTBRoundResult LastRoundResult;
	/** 결과 화면에서 표시할 마지막 보상 요약 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FPTBRewardSummary LastRewardSummary;
	/** 표시 가능한 마지막 라운드 결과 보유 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	bool bHasRoundResult = false;
	/** 미니게임 선택 화면 레벨 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FName MiniGameSelectLevelName = TEXT("L_GameMap");
	/** 메인 타이틀 화면 레벨 이름 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Flow")
	FName MainMenuLevelName = TEXT("L_MainTitle");
};
