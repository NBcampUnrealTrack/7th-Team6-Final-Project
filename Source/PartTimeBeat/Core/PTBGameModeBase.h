#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PTBStructEnums.h"
#include "UI/PTBPauseMenuWidget.h"
#include "PTBGameModeBase.generated.h"

class APTBBaseMiniGame;
struct FStreamableHandle;

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
	/**
	 * 미니게임 ID → 클래스 매핑
	 * Soft 참조: 맵 로드 시 모든 미니게임이 같이 로드되지 않도록 하고,
	 * 실제로 시작할 미니게임만 StartGameFlow에서 비동기 로드한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	TMap<FName, TSoftClassPtr<APTBBaseMiniGame>> MiniGameClassMap;
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

	//	라운드 준비 시작 (미니게임 클래스 비동기 로드 후 스폰)
	UFUNCTION(BlueprintCallable, Category = "Game Flow")
	void StartGameFlow(const FPTBGameSessionRequest& Request);
	//	클래스 탐색 (Soft 참조 반환, 로드하지 않음)
	TSoftClassPtr<APTBBaseMiniGame> ResolveMiniGameClass(FName Id) const;
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

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "PTB|Game")
	void GoToMainMenu();

	/** 일시정지 메뉴 위젯 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	TSubclassOf<UPTBPauseMenuWidget> PauseMenuClass;

	/** 일시정지 메뉴 표시 */
	void ShowPauseMenu();
	/** 일시정지 메뉴 숨기기 */
	void HidePauseMenu();

	/** 미니게임 실제 시작 델리게이트 수신 */
	UFUNCTION()
	void HandleMiniGameStarted();

	/** 미니게임 종료 델리게이트 수신 */
	UFUNCTION()
	void HandleMiniGameFinished(FPTBRoundResult Result);

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

	/** 결과 화면 레벨 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Flow")
	FName ResultLevelName = TEXT("L_MiniGameResult");

	/** 허브에서 위치를 메모리에 기록하는 주기(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hub")
	float HubPositionCacheInterval = 1.0f;

	/** 허브에서 위치를 디스크에 저장하는 주기(초). 비정상 종료 대비용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Hub")
	float HubPositionDiskSaveInterval = 30.0f;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 허브 맵이면 저장된 복귀 위치에 스폰, 아니면 기본(PlayerStart) */
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	/** 저장 위치로 스폰했으면 카메라 방향도 저장된 방향으로 맞춤 */
	virtual void FinishRestartPlayer(AController* NewPlayer, const FRotator& StartRotation) override;

	/** 현재 맵이 허브(미니게임 선택) 맵인지 */
	bool IsHubLevel() const;
	/** 허브 위치 기록 타이머 시작 */
	void StartHubPositionTracking();
	/** 플레이어 위치를 활성 프로필에 기록 (공중에 있으면 건너뜀) */
	bool CacheHubPlayerTransform(bool bSaveToDisk);
	/** 디스크 저장 타이머 콜백 */
	void SaveHubPlayerTransformToDisk();

	FTimerHandle HubPositionCacheTimer;
	FTimerHandle HubPositionDiskSaveTimer;

	/** 이번 스폰이 저장 위치로 이뤄졌는지 (FinishRestartPlayer에서 회전 적용용) */
	bool bSpawnedAtHubReturnTransform = false;
	FRotator HubReturnControlRotation = FRotator::ZeroRotator;

	/** Flow 요청 없이 미니게임 맵을 직접 PIE로 연 경우 배치된 미니게임 Actor로 테스트 시작 */
	bool TryStartDirectPIEMiniGame();

	/** 클래스 로드 완료 후 스폰 ~ 초기화 진행 */
	void ContinueStartGameFlow(TSubclassOf<APTBBaseMiniGame> Cls);

	/** 미니게임 로드 동안 화면을 검은색으로 유지 */
	void HoldScreenBlack();
	/** 검은 화면에서 페이드인 (Duration <= 0이면 즉시 해제) */
	void FadeInScreen(float Duration);

	/** 현재 레벨 이름이 미니게임 키/RuleSet과 매칭되는지 (PIE 전용, 필요 시 동기 로드) */
	bool DoesLevelMatchMiniGame(const FString& LevelName, FName Key, TSubclassOf<APTBBaseMiniGame> Cls) const;

	/** 일시정지 메뉴 위젯 인스턴스 */
	UPROPERTY()
	TObjectPtr<UPTBPauseMenuWidget> PauseMenuInstance;

	/** 미니게임 클래스 비동기 로드 핸들 (유지하는 동안 로드된 클래스가 GC되지 않음) */
	TSharedPtr<FStreamableHandle> MiniGameLoadHandle;

	/** 로드 중 새 요청이 들어온 경우 이전 콜백을 무시하기 위한 일련번호 */
	int32 StartFlowSerial = 0;
};