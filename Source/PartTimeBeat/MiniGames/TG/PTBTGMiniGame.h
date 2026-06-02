#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBTGMiniGame.generated.h"

class UPTBTGMiniGameRuleSet;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBTGNoteEvent, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBTGJudgementEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBTGNoteClearedEvent, int32, NoteId, EPTBJudgementType, JudgementType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBTGHoldStartedEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);

/**
 * 공용 리듬 코어를 실제로 플레이하며 검증하는 TestGame 미니게임 Actor입니다
 *
 * 이 클래스는 채보 이벤트, 공통 Action 입력, Hold/Release 정보, 판정 결과 생성을 확인하는 데 사용합니다
 */
UCLASS()
class PARTTIMEBEAT_API APTBTGMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	/** 노트 Cue 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGNoteEvent OnTGNoteCue;

	/** 노트 Arm 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGNoteEvent OnTGNoteArm;

	/** 노트 정시점 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGNoteEvent OnTGNoteReached;

	/** 판정 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGJudgementEvent OnTGJudgement;

	/** Hold 시작 입력 성공 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGHoldStartedEvent OnTGHoldStarted;

	/** 노트 제거 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|TestGame")
	FPTBTGNoteClearedEvent OnTGNoteCleared;

	/** 오브젝트 / 상태 구성 */
	virtual void BuildRuntimeState() override;

	/** 채보 이벤트 처리 */
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;

	/** 판정 등록 가능 상태 진입 */
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;

	/** 선행 비주얼 큐 */
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;

	/** 점수 / HUD / SFX 반영 */
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

	/** TestGame Action 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleTGInput(EPTBActionType Action, float TimeMs = -1.0f);

	/** TestGame Action 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleTGInputReleased(EPTBActionType Action, float TimeMs = -1.0f);

	/** ActionA 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionAInput(float TimeMs = -1.0f);

	/** ActionA 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionAReleased(float TimeMs = -1.0f);

	/** ActionB 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionBInput(float TimeMs = -1.0f);

	/** ActionB 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionBReleased(float TimeMs = -1.0f);

	/** ActionC 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionCInput(float TimeMs = -1.0f);

	/** ActionC 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionCReleased(float TimeMs = -1.0f);

	/** ActionD 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionDInput(float TimeMs = -1.0f);

	/** ActionD 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionDReleased(float TimeMs = -1.0f);

	/** ActionE 입력 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionEInput(float TimeMs = -1.0f);

	/** ActionE 입력 해제 처리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TestGame")
	void HandleActionEReleased(float TimeMs = -1.0f);

protected:
	/** Hold 시작 입력 판정 */
	virtual FPTBJudgementResult EvaluateHoldInput(EPTBActionType Action, float TimeMs) override;

	/** TestGame RuleSet 조회 */
	const UPTBTGMiniGameRuleSet* GetTGRuleSet() const;

	/** 노트 추적 */
	void TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note);

	/** 노트 추적 해제 */
	bool RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId);

	/** 추적 노트 조회 */
	bool FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const;

	/** 노트 디버그 로그 출력 */
	void LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const;

	/** 판정 디버그 로그 출력 */
	void LogJudgementDebug(const FPTBJudgementResult& Result) const;

	/** Cue 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	int32 CueCount = 0;

	/** Arm 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	int32 ArmCount = 0;

	/** NoteEvent 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	int32 NoteEventCount = 0;

	/** LongNote Cue 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	int32 LongNoteCueCount = 0;

	/** 판정 누적 수 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	int32 JudgementCount = 0;

	/** Cue 상태 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	TArray<FPTBNoteEvent> CuedNotes;

	/** Arm 상태 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	TArray<FPTBNoteEvent> ArmedNotes;

	/** 정시점 도달 노트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	TArray<FPTBNoteEvent> ReachedNotes;
};
