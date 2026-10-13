#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBJJMiniGame.generated.h"

class UPTBJJMiniGameRuleSet;
class APTBJJJumpActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBJJJumpEvent, int32, CharacterIndex, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPTBJJLandingEvent, int32, CharacterIndex, FPTBJudgementResult, Result, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBJJNoteClearedEvent, int32, NoteId, EPTBJudgementType, JudgementType);

/**
 * 체공시간 산출 규칙 묶음
 * 체공시간 = clamp(이전 같은 액션 노트와의 간격 × Ratio, Min, Max)
 */
USTRUCT(BlueprintType)
struct FPTBJJAirtimeProfile
{
	GENERATED_BODY()

	/** 이전 같은 액션 노트와의 간격 중 체공에 쓰는 비율 (1.0에 가까울수록 착지 직후 바로 다시 뜸) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AirtimeRatio = 0.85f;

	/** 최소 체공시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0"))
	float MinAirtimeMs = 250.0f;

	/** 최대 체공시간(ms). 간격이 이보다 길면 이 값으로 고정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0"))
	float MaxAirtimeMs = 2000.0f;

	/** 노트마다 Ratio에 주는 랜덤 편차(±). 0이면 사용 안 함. 시드 고정이라 Retry 시 동일 패턴 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0", ClampMax = "0.5"))
	float RatioJitter = 0.0f;
};

/**
 * 공용 리듬 코어를 점프/착지 연출로 해석하는 JumpJump 미니게임 Actor입니다
 *
 * 채보 노트를 좌/중/우 캐릭터 점프로 매핑하고, 판정 결과를 착지 피드백으로 연출합니다
 */
UCLASS()
class PARTTIMEBEAT_API APTBJJMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	APTBJJMiniGame();

	/** 점프 큐 이벤트(선행 비주얼) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|JumpJump")
	FPTBJJJumpEvent OnJJJumpCue;

	/** 점프 발동 이벤트(정시점 도달) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|JumpJump")
	FPTBJJJumpEvent OnJJJumpTriggered;

	/** 착지 판정 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|JumpJump")
	FPTBJJLandingEvent OnJJLanding;

	/** 노트 제거 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|JumpJump")
	FPTBJJNoteClearedEvent OnJJNoteCleared;

	virtual void BeginPlay() override;

	virtual void InitializeMiniGame(const FPTBMiniGameContext& Context) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	/** 오브젝트 / 상태 구성 */
	virtual void BuildRuntimeState() override;

	/** 채보 이벤트 처리(정시점 도달 → 점프 발동) */
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;

	/** 판정 등록 가능 상태 진입 */
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;

	/** 선행 비주얼 큐 */
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;

	/** 점수 / HUD / SFX 반영 */
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

	/** 전용 결과 */
	virtual FPTBMiniGameResultPayload BuildResultPayload() const override;

	/** 지정 캐릭터 점프 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void TriggerCharacterJump(int32 CharacterIndex, float JumpPowerScale);

	/** 착지 판정 연출 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void PlayLandingFeedback(int32 CharacterIndex, const FPTBJudgementResult& Result);

	/** 변속 적용 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void ApplyVariableJumpSpeed(float SpeedScale);

	/** JJ Action 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void HandleJJInput(EPTBActionType Action, float TimeMs = -1.0f);

	/** JJ Action 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void HandleJJInputReleased(EPTBActionType Action, float TimeMs = -1.0f);

	/** 좌/중/우 점프 캐릭터 Actor 등록(인덱스 = ResolveCharacterIndex 결과) */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void SetJumpActor(int32 CharacterIndex, APTBJJJumpActor* JumpActor);

protected:
	/** 미니게임 전용 피드백(착지 연출 연결점) */
	//virtual void PlayJudgementFeedback(const FPTBJudgementResult& Result) override;

	/** JJ RuleSet 조회 */
	const UPTBJJMiniGameRuleSet* GetJJRuleSet() const;

	/** Action → 캐릭터 인덱스(좌0 / 중1 / 우2) 매핑 */
	int32 ResolveCharacterIndex(EPTBActionType Action) const;

	/** 노트 추적 */
	void TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note);

	/** 노트 추적 해제 */
	bool RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId);

	/** 추적 노트 조회 */
	bool FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const;

	/** 노트 디버그 로그 출력 */
	void LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const;

	/** 현재 난이도에 맞는 체공 프로필 결정 */
	FPTBJJAirtimeProfile ResolveAirtimeProfile() const;

	/** 채보 전체를 스캔해 노트별 체공시간을 미리 계산 */
	void PrecomputeAirtimes();

	/** NoteId로 미리 계산된 체공시간 조회(없으면 기본값) */
	float GetAirtimeMsForNote(int32 NoteId) const;

	/** Insane처럼 "이전 착지 직후 바로 다음 점프"를 예약하는 모드인지 */
	bool IsChainJumpEnabled() const;

	/** 노트 하나의 점프를 (정시점 - 체공시간)에 시작되도록 예약. 이미 예약됐으면 무시 */
	void ScheduleJumpForNote(const FPTBNoteEvent& Note, float NowMs);

	/** 점프를 실제로 발동(타이머 콜백) */
	void StartScheduledJump(int32 CharacterIndex, float AirtimeMs, float HeightScale);

	/** JumpActorClass를 JumpSpawnTransforms 위치에 스폰하고 JumpActors에 등록 */
	void SpawnJumpActors();
	/** 점프 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	int32 JumpCount = 0;

	/** 착지(판정) 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	int32 LandingCount = 0;

	/** Cue 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	int32 CueCount = 0;

	/** 캐릭터별 속도 배율(좌/중/우) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump")
	TArray<float> JumpSpeeds;

	/** 착지 피드백 허용 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump")
	int32 LandingWindowMs = 120;

	/** Cue 상태 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TArray<FPTBNoteEvent> CuedNotes;

	/** Arm 상태 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TArray<FPTBNoteEvent> ArmedNotes;

	/** 정시점 도달 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TArray<FPTBNoteEvent> ReachedNotes;

	/** [Easy/Normal] 최대 선행 점프 시간 상한(ms). 노트 간격이 이보다 길면 이 값으로 고정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Airtime", meta = (ClampMin = "0"))
	float MaxAirtimeMs = 2000.0f;

	/** [Easy/Normal] 최소 체공시간 하한(ms). 간격이 너무 짧아도 이보다 짧게는 안 뜀 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Airtime", meta = (ClampMin = "0"))
	float MinAirtimeMs = 250.0f;

	/** [Easy/Normal] 체공시간 비율. 노트 간격(Gap)의 몇 %를 체공에 쓸지 (0.85 = 85%).
	 *  1.0보다 작게 두면 다음 노트 전에 착지 여유가 생겨 빠른 구간이 덜 빡빡해진다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Airtime", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float AirtimeRatio = 0.85f;

	/**
	 * [Insane] 체공 프로필
	 * Ratio를 1.0 가까이 두면 착지 직후 바로 다음 점프가 시작되어 "거의 착지하지 않는" 상태가 되고,
	 * 노트 간격마다 낙하 속도가 달라져 착지 타이밍 읽기가 어려워진다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Airtime")
	FPTBJJAirtimeProfile InsaneAirtimeProfile = { 0.95f, 200.0f, 4000.0f, 0.0f };

	/** 테스트용: 난이도와 무관하게 Insane 체공 프로필 강제 적용 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Airtime")
	bool bForceInsaneAirtime = false;

	/** 이번 라운드에 실제 적용 중인 체공 프로필 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump|Airtime")
	FPTBJJAirtimeProfile ActiveAirtimeProfile;

	/** 이번 라운드가 Insane 난이도인지 (InitializeMiniGame에서 설정) */
	bool bIsInsaneDifficulty = false;

	/** 체공 랜덤 편차용 시드 (세션 RandomSeed) */
	int32 AirtimeSeed = 0;

	/** 좌/중/우 점프 캐릭터 Actor (인덱스 0/1/2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump")
	TArray<TObjectPtr<APTBJJJumpActor>> JumpActors;

	/** NoteId → 미리 계산된 체공시간(ms) */
	TMap<int32, float> NoteAirtimeMs;

	/** 점프 발동 예약 타이머 핸들 모음(라운드 종료 시 정리용) */
	TArray<FTimerHandle> PendingJumpTimers;

	/** NoteId → 같은 액션의 다음 노트 (체인 점프 예약용) */
	TMap<int32, FPTBNoteEvent> NextSameActionNote;

	/** 점프가 이미 예약된 NoteId (Cue / 체인 중복 예약 방지) */
	TSet<int32> ScheduledJumpNoteIds;

	/** 스폰할 점프 캐릭터 클래스(BP_JJ_JumpChar 등을 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Spawn")
	TSubclassOf<APTBJJJumpActor> JumpActorClass;

	/** 좌/중/우 스폰 Transform (인덱스 0=좌, 1=중, 2=우). 월드 기준 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump|Spawn")
	TArray<FTransform> JumpSpawnTransforms;
};