#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "TimerManager.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBOnMiniGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBMiniGameNoteDelegate, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBMiniGameJudgementDelegate, FPTBJudgementResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBMiniGameOutroDelegate, FPTBRoundResult, Result, EPTBRoundEndReason, EndReason);

struct FPTBActiveHoldState
{
	FPTBNoteEvent Note;
	FPTBJudgementResult PendingResult;
	float PressedTimeMs = 0.0f;
	float RequiredHoldUntilTimeMs = 0.0f;
};

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

	/** RuleSet에서 적용되는 Wwise 이벤트 매핑 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio", AdvancedDisplay)
	TObjectPtr<UPTBWwiseEventMapAsset> AudioEventSet;

	/** RuleSet 난이도 설정에서 선택된 채보 Asset */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm", AdvancedDisplay)
	TObjectPtr<UPTBRhythmChartAsset> ChartAsset;

	/** Deprecated: ChartAsset.SourceJsonFilePath를 사용 */
	UPROPERTY()
	FString ChartJsonFilePath;

	/** 로딩 화면 Widget 클래스 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	TSubclassOf<UPTBMiniGameLoadingWidget> LoadingWidgetClass;

	/** 미니게임 맵을 직접 PIE로 열었을 때 이 Actor로 테스트 라운드를 자동 시작 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame|PIE Test")
	bool bAutoStartWhenOpenedDirectlyInPIE = false;

	/** 직접 PIE 테스트에 사용할 난이도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame|PIE Test")
	EPTBDifficulty DirectPIEDifficulty = EPTBDifficulty::Standard;

	/** 직접 PIE 테스트에 사용할 플레이 모드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|MiniGame|PIE Test")
	EPTBPlayMode DirectPIEPlayMode = EPTBPlayMode::Single;

	/** 미니게임 종료 결과 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameFinished OnMiniGameFinished;

	/** 미니게임 시작 준비 완료 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameReadyToStart OnMiniGameReadyToStart;

	/** 미니게임 실제 시작 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameStarted OnMiniGameStarted;

	/** Audio와 Chart가 실제로 시작될 때 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Flow")
	FPTBOnMiniGameStarted OnMiniGameGameplayStarted;

	/** 결과 전달 전 종료 연출이 시작될 때 발생. EndReason으로 성공/실패 연출을 구분합니다. */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Flow")
	FPTBMiniGameOutroDelegate OnMiniGameOutroStarted;

	/** 종료 연출 시간이 끝나고 결과 전달 직전에 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Flow")
	FPTBMiniGameOutroDelegate OnMiniGameOutroFinished;

	/** 채보 노트가 실제 이벤트 타이밍에 도달했을 때 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Events")
	FPTBMiniGameNoteDelegate OnMiniGameChartNote;

	/** 노트가 판정 대기열에 등록됐을 때 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Events")
	FPTBMiniGameNoteDelegate OnMiniGameNoteArmed;

	/** 노트 선행 비주얼 큐 타이밍에 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Events")
	FPTBMiniGameNoteDelegate OnMiniGameNoteCue;

	/** 공통 점수/SFX 처리가 반영된 판정 결과 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Events")
	FPTBMiniGameJudgementDelegate OnMiniGameJudgement;

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

	/** 입력 해제를 홀드 유지 검사로 전달 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	virtual void HandleRhythmInputReleased(EPTBActionType Action, float TimeMs = -1.0f);

	/** 현재 연출 기준 차트 시간(ms) */
	UFUNCTION(BlueprintPure, Category = "PTB|MiniGame|Time")
	float GetCurrentChartTimeMs() const;

	/** 현재 입력 판정 기준 시간(ms) */
	UFUNCTION(BlueprintPure, Category = "PTB|MiniGame|Time")
	float GetCurrentInputJudgeTimeMs() const;

	/** BP 미니게임에서 채보 노트 타이밍 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Events")
	void ReceiveChartNote(FPTBNoteEvent Note);
	virtual void ReceiveChartNote_Implementation(FPTBNoteEvent Note);

	/** BP 미니게임에서 판정 대기열 등록 타이밍 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Events")
	void ReceiveNoteArmed(FPTBNoteEvent Note);
	virtual void ReceiveNoteArmed_Implementation(FPTBNoteEvent Note);

	/** BP 미니게임에서 선행 큐 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Events")
	void ReceiveNoteCue(FPTBNoteEvent Note);
	virtual void ReceiveNoteCue_Implementation(FPTBNoteEvent Note);

	/** BP 미니게임에서 판정 결과 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Events")
	void ReceiveJudgement(FPTBJudgementResult Result);
	virtual void ReceiveJudgement_Implementation(FPTBJudgementResult Result);

	/** BP 미니게임에서 시작 연출 시점을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Flow")
	void ReceiveIntroStarted();
	virtual void ReceiveIntroStarted_Implementation();

	/** BP 미니게임에서 Audio/Chart 실제 시작 시점 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Flow")
	void ReceiveGameplayStarted();
	virtual void ReceiveGameplayStarted_Implementation();

	/** BP 미니게임에서 결과 전달 전 종료 연출을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Flow")
	void ReceiveOutroStarted(FPTBRoundResult Result, EPTBRoundEndReason EndReason);
	virtual void ReceiveOutroStarted_Implementation(FPTBRoundResult Result, EPTBRoundEndReason EndReason);

	/** BP 미니게임에서 종료 연출 완료 시점을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Flow")
	void ReceiveOutroFinished(FPTBRoundResult Result, EPTBRoundEndReason EndReason);
	virtual void ReceiveOutroFinished_Implementation(FPTBRoundResult Result, EPTBRoundEndReason EndReason);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 고유 ID */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame", AdvancedDisplay)
	FName MiniGameId;

	/** 2글자 코드 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame", AdvancedDisplay)
	FName MiniGameCode;

	/** UI 표시 이름 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame", AdvancedDisplay)
	FText DisplayName;

	/** 자동 생성되는 Wwise 런타임 매니저 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio", AdvancedDisplay)
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

	/** 시작 연출 또는 게임플레이 시작 대기 중 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bStartSequenceActive = false;

	/** 종료 처리 또는 종료 연출 진행 중 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bFinishSequenceActive = false;

	/** 입력 잠금 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame", AdvancedDisplay)
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

	/** 유지 중인 Hold 노트 */
	TArray<FPTBActiveHoldState> ActiveHoldStates;

	/** 인트로 후 Audio/Chart 시작 타이머 */
	FTimerHandle IntroTimerHandle;

	/** 아웃트로 후 결과 전달 타이머 */
	FTimerHandle OutroTimerHandle;

	/** 결과 전달 대기 중인 종료 사유 */
	EPTBRoundEndReason PendingEndReason = EPTBRoundEndReason::Completed;

	/** 라운드 시작 전 에셋 준비(하위 override 권장) */
	virtual void PreloadAssets();

	/** 오브젝트 / 상태 구성(하위 override) */
	virtual void BuildRuntimeState();

	/** 오디오 에셋 준비 */
	virtual void PreloadAudioAssets();

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

	/** 미니게임 진행 중에 사용할 GameOnly 입력 모드 적용 */
	virtual void ApplyGameOnlyInputMode();

	/** 로딩/일시정지/결과 UI에서 사용할 GameAndUI 입력 모드 적용 */
	virtual void ApplyGameAndUIInputMode();

	/** Audio와 Chart를 실제로 시작 */
	virtual void BeginGameplaySequence();

	/** 결과 전달 전 종료 연출을 시작하거나 즉시 완료 */
	virtual void BeginOutroSequence(EPTBRoundEndReason Reason);

	/** 결과 델리게이트를 전달하고 GameMode 플로우로 넘김 */
	virtual void CompleteFinishSequence();

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

	/** Hold 시작 입력 판정 */
	virtual FPTBJudgementResult EvaluateHoldInput(EPTBActionType Action, float TimeMs);

	/** Hold 성공 유지 시간 도달 처리 */
	void ResolveSatisfiedHoldInputs(float CurrentTimeMs);

	/** Hold 성공 확정 */
	void ConfirmActiveHold(int32 HoldIndex);

	/** Hold 조기 해제 실패 처리 */
	void FailActiveHoldEarlyRelease(int32 HoldIndex, float ReleaseTimeMs);

	/** 지연 판정 결과 전달 */
	void DispatchDeferredJudgementResult(const FPTBJudgementResult& Result);

	/** 점수 / HUD / SFX 반영 */
	UFUNCTION()
	virtual void HandleJudgementResult(FPTBJudgementResult Result);

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

	/** 라운드 종료 기준 차트 시간(ms) */
	float GetRoundEndChartTimeMs() const;

	UFUNCTION()
	void HandleBGMFinished(int32 PlayingId);

	UFUNCTION()
	void HandleAllNotesPassed();

};
