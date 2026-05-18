#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTBStructEnums.h"
#include "PTBGameModeBase.generated.h"

class APTBBaseMiniGame;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameEnded, FPTBRoundResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGamePaused);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameResumed);

UCLASS()
class PARTTIMEBEAT_API APTBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/**	미니게임 ID → 클래스 매핑 */
	TMap<FName, TSubclassOf<APTBBaseMiniGame>> MiniGameClassMap;
	/**	현재 실행 중인 미니게임 */
	APTBBaseMiniGame* ActiveMiniGame;
	/** 현재 라운드 요청 */
	FPTBGameSessionRequest CurrentRequest;
	/** 시작 카운트다운(기본 3) */
	int32 CountdownSeconds;
	/** 게임 진행 중 여부 */
	bool bIsGameActive;
	/**	일시정지 상태 */
	bool bIsPaused;

	//	라운드 준비 시작
	void StartGameFlow(const FPTBGameSessionRequest& Request);
	//	클래스 탐색
	TSubclassOf<APTBBaseMiniGame> ResolveMiniGameClass(FName Id) const;
	//	Actor 스폰
	APTBBaseMiniGame* SpawnMiniGame(TSubclassOf<APTBBaseMiniGame> Cls);
	//	카운트다운 → BGM → Conductor → StartMiniGame
	void BeginRound();
	//게임 + Wwise 일시정지
	void PauseGame(); 
	// 게임 + Wwise 재개
	void ResumeGame();
	//	결과 제출(싱글 = 저장, 멀티 = 검증)
	void SubmitRoundResult(const FPTBRoundResult& Result);
	//	현재 라운드 재시작
	void RetryGame();
	//	미니게임 선택으로 복귀
	void ExitToMenu();

	// 1) 카운트다운 후 게임 시작 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameStarted OnGameStarted;

	// 2) 게임 종료 시 결과 전달과 함께 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameEnded OnGameEnded;

	// 3) 게임 일시정지 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGamePaused OnGamePaused;

	// 4) 게임 재개 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Game State")
	FOnGameResumed OnGameResumed;
};
