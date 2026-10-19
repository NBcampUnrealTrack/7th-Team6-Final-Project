#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "MiniGames/Common/PTBMiniGameIntroData.h"
#include "TimerManager.h"
#include "PTBBaseMiniGame.generated.h"

class UPTBMiniGameRuleSet;
class UPTBSceneWidgetBase;
class UDataTable;
class USoundBase;
class UPTBWwiseEventMapAsset;
class UPTBWwiseAudioManager;
class UPTBWwiseRhythmSyncComponent;
class UPTBRhythmChartAsset;
class UPTBRhythmConductorComponent;
class UPTBJudgementSystem;
class UPTBScoreCalculator;
class UPTBMiniGameLoadingWidget;
class UAkComponent;
class UUserWidget;
struct FStreamableHandle;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBOnMiniGameFinished, FPTBRoundResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBOnMiniGameReadyToStart);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBOnMiniGameStarted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBMiniGameNoteDelegate, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBMiniGameJudgementDelegate, FPTBJudgementResult, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBMiniGameOutroDelegate, FPTBRoundResult, Result, EPTBRoundEndReason, EndReason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBMiniGameScenePhaseDelegate, EPTBMiniGameScenePhase, Phase);

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

	/**
	 * 스토리/튜토리얼 데이터.
	 * 비어 있거나 시작 스토리/튜토리얼 테이블이 비었으면 GameInstance.DefaultIntroData(공용 템플릿) 사용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Scene")
	TObjectPtr<UPTBMiniGameIntroData> IntroData;

	/** 씬 단계(시작 스토리/튜토리얼/시작 대기/결과 직전 스토리) 변경 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|Scene")
	FPTBMiniGameScenePhaseDelegate OnScenePhaseChanged;

	/** 현재 씬 단계 */
	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	EPTBMiniGameScenePhase GetScenePhase() const { return ScenePhase; }

	/** 씬 위젯 종료 알림 (UPTBSceneWidgetBase::FinishScene에서 호출). 시청 기록 후 다음 단계로 진행 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	void NotifySceneFinished(bool bSkipped);

	/**
	 * 튜토리얼 연습 한 판 시작 (튜토리얼 위젯에서 호출).
	 * 스텝의 패턴으로 짧은 채보를 만들어 실제 미니게임으로 재생. 끝나면 씬 위젯의 HandlePracticeFinished 호출.
	 * 연습 결과는 점수/결과/진행도에 반영되지 않음.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Scene")
	bool StartTutorialPractice(const FPTBTutorialStep& Step);

	/** 튜토리얼 연습 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|Scene")
	bool IsTutorialPracticeActive() const { return bTutorialPracticeActive; }

	/** 확인 입력(Enter/Space)을 씬 위젯에 전달. 씬 재생 중이고 처리됐으면 true (PlayerController에서 호출) */
	bool HandleSceneConfirmInput();

	/**
	 * BP 미니게임: 튜토리얼 연습 한 판 시작 직전.
	 * 본게임 시작(Intro Started 등)에서 만들던 HUD/판정 UI를 여기서도 만들어야 연습 중에 보임.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Scene")
	void ReceiveTutorialPracticeStarted();
	virtual void ReceiveTutorialPracticeStarted_Implementation();

	/** BP 미니게임: 튜토리얼 연습 한 판 종료 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Scene")
	void ReceiveTutorialPracticeFinished(bool bPassed);
	virtual void ReceiveTutorialPracticeFinished_Implementation(bool bPassed);

	/** 미니게임 종료 결과 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameFinished OnMiniGameFinished;

	/** 미니게임 시작 준비 완료 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameReadyToStart OnMiniGameReadyToStart;

	/** 인트로를 포함한 미니게임 시작 시퀀스 시작 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame")
	FPTBOnMiniGameStarted OnMiniGameStarted;

	/** 시작 연출이 끝나고 Cue 또는 Gameplay로 넘어갈 때 발생 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|MiniGame|Flow")
	FPTBOnMiniGameStarted OnMiniGameIntroFinished;

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

	/** 라운드 중 스폰한 Actor를 등록한다. Retry로 이 미니게임이 파괴될 때(EndPlay) 함께 자동 Destroy된다 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	void RegisterSpawnedRoundActor(AActor* Actor);

	/**
	 * 지정한 클래스의 위젯이 이미 뷰포트에 떠 있다면 제거한다(Retry로 HUD가 중복 생성되는 것 방지).
	 * WidgetToKeep을 넘기면 그 위젯은 제외 — 위젯 자신의 Construct(이미 AddToViewport된 이후)에서
	 * 자기 클래스를 정리할 때는 Self를 넘겨야 새로 추가된 위젯까지 같이 지워지지 않는다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|MiniGame")
	void RemoveExistingWidgetsOfClass(TSubclassOf<UUserWidget> WidgetClass, UUserWidget* WidgetToKeep = nullptr) const;

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

	/** BP 미니게임에서 시작 연출 완료 시점을 확장합니다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|MiniGame|Flow")
	void ReceiveIntroFinished();
	virtual void ReceiveIntroFinished_Implementation();

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

	/** BP 미니게임에서 씬 단계 변경 시점을 확장합니다. (예: 튜토리얼 중 캐릭터 대기 모션) */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Scene")
	void ReceiveScenePhaseChanged(EPTBMiniGameScenePhase Phase);
	virtual void ReceiveScenePhaseChanged_Implementation(EPTBMiniGameScenePhase Phase);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

	/** 라운드 실패 예약 여부 (Tick에서 즉시 FinishMiniGame(Failed) 호출) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bPendingRoundFailed = false;

	/** 모든 노트 발행 완료 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	bool bAllNotesDispatched = false;

	/** 현재 재생 중인 BGM PlayingId */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	int32 ActiveBGMPlayingId = 0;

	/** 헛입력으로 잠긴 Action별 해제 시간(ms) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	TMap<EPTBActionType, float> EmptyInputActionLockUntilTimeMs;

	/** 사전 로드된 에셋 */
	UPROPERTY()
	TArray<TObjectPtr<UObject>> PreloadedAssets;

	/** RegisterSpawnedRoundActor()로 등록된, 라운드 중 스폰된 Actor 목록. EndPlay에서 자동 정리된다. */
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedRoundActors;

	/** 유지 중인 Hold 노트 */
	TArray<FPTBActiveHoldState> ActiveHoldStates;

	/** 사전 로드 핸들 */
	TSharedPtr<FStreamableHandle> PreloadHandle;

	/** 인트로 후 Audio/Chart 시작 타이머 */
	FTimerHandle IntroTimerHandle;

	/** Cue 선행 발행 후 Audio/Chart 시작 타이머 */
	FTimerHandle CuePreRollTimerHandle;

	/** 아웃트로 후 결과 전달 타이머 */
	FTimerHandle OutroTimerHandle;

	/** 결과 전달 대기 중인 종료 사유 */
	EPTBRoundEndReason PendingEndReason = EPTBRoundEndReason::Completed;

	// ── 스토리/튜토리얼 씬 ─────────────────────────────────────────

	/** 현재 씬 단계 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Scene")
	EPTBMiniGameScenePhase ScenePhase = EPTBMiniGameScenePhase::None;

	/** 표시 중인 씬 위젯 */
	UPROPERTY()
	TObjectPtr<UPTBSceneWidgetBase> ActiveSceneWidget;

	/** 이번 라운드는 본 적 있어도 씬 재생 (다시보기) */
	bool bForceScenePlayback = false;

	/** 로딩 완료 후 시작 스토리 → 튜토리얼 → 시작 대기 순으로 진행 */
	void BeginIntroScenes();

	/** 현재 단계 다음의 시작 씬을 재생하거나, 없으면 시작 대기(HandleReadyToStart)로 */
	void AdvanceIntroScenes();

	/** 해당 단계 씬을 재생할 수 있으면 위젯 띄우고 true */
	bool TryPlayScene(EPTBMiniGameScenePhase Phase);

	/** 재생 조건 (본 적 있음/버전/클리어 여부). 미니게임별로 조건을 바꾸려면 override */
	virtual bool ShouldPlayScene(EPTBMiniGameScenePhase Phase) const;

	/** 해당 단계 내용을 가진 IntroData (미니게임 전용 → 공용 템플릿 순). 없으면 nullptr */
	const UPTBMiniGameIntroData* ResolveSceneData(EPTBMiniGameScenePhase Phase) const;

	/** 해당 단계 위젯 클래스 (GameInstance 설정) */
	TSubclassOf<UPTBSceneWidgetBase> ResolveSceneWidgetClass(EPTBMiniGameScenePhase Phase) const;

	/** 시청 기록 키에 쓸 미니게임 ID */
	FName ResolveSceneMiniGameId() const;

	/** 단계 변경 + 델리게이트/BP 이벤트 */
	void SetScenePhase(EPTBMiniGameScenePhase NewPhase);

	/** 씬 위젯 제거 */
	void CloseActiveSceneWidget();

	/** 로딩 위젯 숨김/복구 (씬 재생 중에는 숨김) */
	void SetLoadingWidgetHidden(bool bShouldHide);

	/** 결과 델리게이트 전달 (결과 직전 스토리 이후 또는 즉시) */
	void DispatchMiniGameFinished();

	// ── 튜토리얼 연습 ─────────────────────────────────────────────

	/** 연습 라운드 진행 중 */
	bool bTutorialPracticeActive = false;

	/** 연습용 임시 채보 */
	UPROPERTY(Transient)
	TObjectPtr<UPTBRhythmChartAsset> TutorialPracticeChart;

	/** 연습 동안 잠시 빼둔 본게임 채보 */
	UPROPERTY(Transient)
	TObjectPtr<UPTBRhythmChartAsset> SavedMainChartAsset;

	FPTBChartData SavedMainChartData;

	/** 연습 중 노트 타이밍 소리 / 메트로놈 소리 */
	UPROPERTY(Transient)
	TObjectPtr<USoundBase> PracticeNoteSound;

	UPROPERTY(Transient)
	TObjectPtr<USoundBase> PracticeMetronomeSound;

	int32 PracticeTotalNotes = 0;
	int32 PracticeRequiredHits = 0;
	FName PracticeMetronomeSFXKey = NAME_None;

	/** 스텝 패턴(또는 지정 채보)으로 연습 채보 생성 */
	UPTBRhythmChartAsset* BuildTutorialPracticeChart(const FPTBTutorialStep& Step);

	/** 연습 한 판 종료 처리: 통과 판정, 본게임 채보 복구, 위젯에 결과 전달 */
	void CompleteTutorialPractice(EPTBRoundEndReason Reason);

	/** 판정/점수 상태를 새 라운드 시작 전으로 초기화 */
	void ResetRoundStateForReplay();

	/**
	 * 라운드 채보가 바뀐 직후 호출 (튜토리얼 연습 시작 / 연습 끝나고 본게임 채보 복구).
	 * 하위 미니게임은 채보 기반으로 미리 계산한 값, NoteId 기반 중복 방지 목록, 예약 타이머 등을 여기서 다시 맞출 것.
	 */
	virtual void HandleRoundChartChanged();

	/** 연습 채보 NoteId가 회차마다 겹치지 않도록 쓰는 카운터 */
	int32 PracticeRunCounter = 0;

	/** 연습 중 매 박 메트로놈 */
	UFUNCTION()
	void HandlePracticeBeatTick(float Beat);

	/** 라운드 시작 전 에셋 준비(하위 override 권장) */
	virtual void PreloadAssets();

	/** 사전 로드 에셋 요청 */
	void RequestPreloadAssets();

	/** 사전 로드 에셋 경로 수집 */
	void GatherPreloadAssetPaths(TArray<FSoftObjectPath>& OutAssetPaths) const;

	/** 사전 로드 완료 확인 */
	bool VerifyPreloadedAssets(const TArray<FSoftObjectPath>& AssetPaths);

	/** 사전 로드 완료 처리 */
	void HandlePreloadAssetsLoaded(TArray<FSoftObjectPath> AssetPaths);

	/** 초기화 마무리 */
	void CompleteMiniGameInitialization();

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

	/** Audio와 Chart 시작 전 Cue 선행 발행 */
	virtual void BeginCuePreRollSequence();

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

	/** Cue 선행 발행 중 여부 */
	bool IsCuePreRollActive() const;

	/** 키 - 액션 매핑(하위 override) */
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const;

	/** 입력 판정 오프셋 계산 */
	virtual float ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const;

	/** 화면 표시 오프셋 계산 */
	virtual float ResolveVisualOffsetMs(const FPTBMiniGameContext& Context) const;

	/** 소리 출력 오프셋 계산 */
	virtual float ResolveSoundOffsetMs(const FPTBMiniGameContext& Context) const;

	/** Cue 선행 발행 시간 계산 */
	virtual float ResolveCuePreRollTimeMs() const;

	/** 라운드 종료 기준 차트 시간(ms) */
	float GetRoundEndChartTimeMs() const;

	UFUNCTION()
	void HandleBGMFinished(int32 PlayingId);

	UFUNCTION()
	void HandleAllNotesPassed();

};
