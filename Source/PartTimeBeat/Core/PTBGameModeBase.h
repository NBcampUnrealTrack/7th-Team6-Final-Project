#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTBStructEnums.h"
#include "UI/PTBPauseMenuWidget.h"
#include "PTBGameModeBase.generated.h"

class APTBBaseMiniGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEnded, FPTBRoundResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRoundResultReady, FPTBRoundResult, Result, FPTBRewardSummary, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamePaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameResumed);

UCLASS()
class PARTTIMEBEAT_API APTBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/**	미니게임 ID → 클래스 매핑 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	TMap<FName, TSubclassOf<APTBBaseMiniGame>> MiniGameClassMap;
	/**	현재 실행 중인 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	APTBBaseMiniGame* ActiveMiniGame = nullptr;
	/** 현재 라운드 요청 */
	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	FPTBGameSessionRequest CurrentRequest;
	/** 게임 진행 중 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	bool bIsGameActive = false;
	/**	일시정지 상태 */
	UPROPERTY(BlueprintReadOnly, Category = "Game Flow")
	bool bIsPaused = false;

	//	라운드 준비 시작
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGameFlow(const FPTBGameSessionRequest& Request);
	//	클래스 탐색
	TSubclassOf<APTBBaseMiniGame> ResolveMiniGameClass(FName Id) const;
	//	Actor 스폰
	APTBBaseMiniGame* SpawnMiniGame(TSubclassOf<APTBBaseMiniGame> Cls);
	//게임 + Wwise 일시정지
	void PauseGame(); 
	// 게임 + Wwise 재개
	void ResumeGame();

	//	결과 제출(싱글 = 저장, 멀티 = 검증)
	void SubmitRoundResult(const FPTBRoundResult& Result);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PTB|Game")
	void RetryGame();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PTB|Game")
	void ExitToMenu();

	/** 일시정지 메뉴 위젯 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	TSubclassOf<UPTBPauseMenuWidget> PauseMenuClass;

	/** 일시정지 메뉴 표시 */
	void ShowPauseMenu();
	/** 일시정지 메뉴 숨기기 */
	void HidePauseMenu();

	// 1) 카운트다운 후 게임 시작 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameStarted OnGameStarted;

	// 2) 게임 종료 시 결과 전달과 함께 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameEnded OnGameEnded;

	/** 결과 위젯 표시용 라운드 결과와 보상 요약 전달 */
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnRoundResultReady OnRoundResultReady;

	// 3) 게임 일시정지 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePaused OnGamePaused;

	// 4) 게임 재개 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameResumed OnGameResumed;

	/** 마지막 라운드 결과 */
	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	FPTBRoundResult LastRoundResult;

	/** 마지막 보상 요약 */
	UPROPERTY(BlueprintReadOnly, Category = "Game State")
	FPTBRewardSummary LastRewardSummary;

protected:
	/** 미니게임 실제 시작 델리게이트 수신 */
	UFUNCTION()
	void HandleMiniGameStarted();

	/** 미니게임 종료 델리게이트 수신 */
	UFUNCTION()
	void HandleMiniGameFinished(FPTBRoundResult Result);
	
	/** 일시정지 메뉴 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPTBPauseMenuWidget> PauseMenuInstance;
};
