#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBJJMiniGame.generated.h"

class UPTBJJMiniGameRuleSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBJJJumpEvent, int32, CharacterIndex, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPTBJJLandingEvent, int32, CharacterIndex, FPTBJudgementResult, Result, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBJJNoteClearedEvent, int32, NoteId, EPTBJudgementType, JudgementType);

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
};