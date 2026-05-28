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

	/** 결과 후 미니게임 선택 화면으로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Flow")
	void ReturnToMiniGameSelect();

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
};
