#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBBaseMiniGame.generated.h"

class UPTBMiniGameRuleSet;
class UPTBWwiseEventMapAsset;
class UPTBWwiseAudioManager;
class UPTBWwiseRhythmSyncComponent;
class UPTBRhythmChartAsset;
class UPTBRhythmConductorComponent;
class UPTBJudgementSystem;
class UPTBScoreCalculator;
class UPTBMiniGameLoadingWidget;
class UAkComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBOnMiniGameFinished, FPTBRoundResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBOnMiniGameReadyToStart);

/**
 * 공통 리듬 라운드의 초기화, 입력 판정, 점수 계산, 오디오 요청을 담당하는 미니게임 베이스 Actor입니다.
 *
 * 개별 미니게임은 채보 이벤트와 판정 피드백만 각자의 연출 방식으로 해석합니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBBaseMiniGame : public AActor
{
	GENERATED_BODY()
	
public:
	APTBBaseMiniGame();

	/** 난이도별 규칙 DataAsset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame")
	TObjectPtr<UPTBMiniGameRuleSet> RuleSet;

	/** Wwise 이벤트 매핑 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Audio")
	TObjectPtr<UPTBWwiseEventMapAsset> AudioEventSet;

	/** 재생할 채보 Asset */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Rhythm")
	TObjectPtr<UPTBRhythmChartAsset> ChartAsset;

	/** 임시 외부 JSON 채보 파일 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Rhythm")
	FString ChartJsonFilePath;

	/** 로딩 화면 Widget 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	TSubclassOf<UPTBMiniGameLoadingWidget> LoadingWidgetClass;

	/** 미니게임 종료 결과 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameFinished OnMiniGameFinished;

	/** 미니게임 시작 준비 완료 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameReadyToStart OnMiniGameReadyToStart;

	/** 컨텍스트 주입 */
	virtual void InitializeMiniGame(const FPTBMiniGameContext& Context);

	/** BGM + Conductor 시작, 입력 허용 */
	virtual void StartMiniGame();

	/** 준비 완료 상태에서 미니게임 시작 요청 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void RequestStartMiniGame();

	/** Press Any Key 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void HandleStartInput();

	/** 종료, 결과 생성 */
	virtual FPTBRoundResult FinishMiniGame(EPTBRoundEndReason Reason);

	/** 일시정지 */
	virtual void PauseMiniGame();

	/** 재개 */
	virtual void ResumeMiniGame();

	/** 입력을 판정으로 전달 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void HandleRhythmInput(EPTBActionType Action, float TimeMs = -1.0f);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 고유 ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame")
	FName MiniGameId;

	/** 2글자 코드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame")
	FName MiniGameCode;

	/** UI 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame")
	FText DisplayName;

	/** Wwise 런타임 매니저 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Audio")
	TObjectPtr<UPTBWwiseAudioManager> AudioManager;

	/** 미니게임 전용 AkComponent */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TObjectPtr<UAkComponent> AkComponent;

	/** Beat 발행 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm")
	TObjectPtr<UPTBRhythmConductorComponent> RhythmConductor;

	/** Wwise 재생 시간 기반 동기화 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm")
	TObjectPtr<UPTBWwiseRhythmSyncComponent> RhythmSyncComponent;

	/** 판정 계산 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm")
	TObjectPtr<UPTBJudgementSystem> JudgementSystem;

	/** 점수 계산 */
	UPROPERTY()
	TObjectPtr<UPTBScoreCalculator> ScoreCalculator;

	/** 라운드 컨텍스트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FPTBMiniGameContext GameContext;

	/** 처리 중인 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm")
	TArray<FPTBNoteEvent> ActiveNoteQueue;

	/** 종료 결과 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FPTBRoundResult RoundResult;

	/** 로딩 화면 Widget 인스턴스 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UPTBMiniGameLoadingWidget> LoadingWidgetInstance;

	/** 초기화 완료 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bIsInitialized = false;

	/** 시작 준비 완료 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bIsReadyToStart = false;

	/** 라운드 진행 중 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bIsRoundActive = false;

	/** 입력 잠금 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame")
	bool bInputLocked = true;

	/** 라운드 완료 예약 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bPendingRoundFinish = false;

	/** 모든 노트 발행 완료 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bAllNotesDispatched = false;

	/** 현재 재생 중인 BGM PlayingId */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	int32 ActiveBGMPlayingId = 0;

	/** 헛입력으로 잠긴 Action별 해제 시간(ms) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	TMap<EPTBActionType, float> EmptyInputActionLockUntilTimeMs;

	/** 라운드 시작 전 에셋 준비(하위 override 권장) */
	virtual void PreloadAssets();

	/** 오브젝트 / 상태 구성(하위 override) */
	virtual void BuildRuntimeState();

	/** RuleSet 공통 규칙 적용 */
	virtual void ApplyRuleSet();

	/** 로딩 시작 처리(하위 override) */
	virtual void HandleLoadingStarted();

	/** 시작 준비 완료 처리(하위 override) */
	virtual void HandleReadyToStart();

	/** 로딩 Widget 표시 */
	virtual void ShowLoadingWidget();

	/** 로딩 Widget 제거 */
	virtual void HideLoadingWidget();

	/** 채보 이벤트 처리(하위 override) */
	UFUNCTION()
	virtual void HandleChartEvent(FPTBNoteEvent Note);

	/** 판정 등록 가능 상태 진입(하위 override) */
	UFUNCTION()
	virtual void HandleNoteArm(FPTBNoteEvent Note);

	/** 선행 비주얼 큐(하위 override) */
	UFUNCTION()
	virtual void HandleNoteCue(FPTBNoteEvent Note);

	/** 단일 판정 */
	virtual FPTBJudgementResult EvaluateInput(EPTBActionType Action, float TimeMs);

	/** 점수 / HUD / SFX 반영 */
	UFUNCTION()
	virtual void HandleJudgementResult(FPTBJudgementResult Result);

	/** 미니게임 전용 피드백(하위 override) */
	virtual void PlayJudgementFeedback(const FPTBJudgementResult& Result);

	/** 전용 결과(하위 override) */
	virtual FPTBMiniGameResultPayload BuildResultPayload() const;

	/** AudioManager에 이벤트 요청 */
	void RequestWwiseEvent(FName EventKey, AActor* Target);

	/** 입력 가능 여부 */
	bool CanAcceptInput() const;

	/** 키 - 액션 매핑(하위 override) */
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const;

	/** 입력 판정 오프셋 계산 */
	virtual float ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const;

	/** 화면 표시 오프셋 계산 */
	virtual float ResolveVisualOffsetMs(const FPTBMiniGameContext& Context) const;

	/** 소리 출력 오프셋 계산 */
	virtual float ResolveSoundOffsetMs(const FPTBMiniGameContext& Context) const;

	/** 현재 판정 기준 차트 시간(ms) */
	float GetCurrentChartTimeMs() const;

	/** 현재 입력 판정 기준 시간(ms) */
	float GetCurrentInputJudgeTimeMs() const;

	/** 라운드 종료 기준 차트 시간(ms) */
	float GetRoundEndChartTimeMs() const;

	UFUNCTION()
	void HandleBGMFinished(int32 PlayingId);

	UFUNCTION()
	void HandleAllNotesPassed();

};
