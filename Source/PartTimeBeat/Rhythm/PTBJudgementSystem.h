#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgementSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJudgementResult, FPTBJudgementResult, JudgementResult);

/**
 * 채보 노트와 Wwise 기준 입력 시간을 비교해 판정 결과를 생성하는 리듬 판정 컴포넌트입니다.
 *
 * 이 컴포넌트는 점수 누적을 직접 처리하지 않고 PendingNotes 관리, 입력 판정, 만료 Miss 발생만 담당합니다.
 * 미니게임은 Conductor 또는 NoteCue에서 노트를 등록하고 입력 시 EvaluateInput을 호출합니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBJudgementSystem : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** 기본값 초기화 */
	UPTBJudgementSystem();

protected:
	/** 게임 시작 처리 */
	virtual void BeginPlay() override;

public:	
	/** 프레임 처리 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	/** 채보와 사용자 판정 오프셋으로 초기화 */
	void Initialize(const FPTBChartData& Chart, float UserOffset);
	
	/** Conductor 또는 NoteCue에서 전달된 노트 등록 */
	void RegisterNoteEvent(const FPTBNoteEvent& Note);
	
	/** 입력 액션과 입력 시간 기준 판정 */
	FPTBJudgementResult EvaluateInput(EPTBActionType Action, float InputTimeMs);

	/** 입력 액션과 입력 시간 기준 판정 */
	FPTBJudgementResult EvaluateInput(EPTBActionType Action, float InputTimeMs, bool bBroadcastResult);

	/** 특정 노트 ID를 대상으로 입력 액션과 입력 시간 기준 판정 */
	FPTBJudgementResult EvaluateInputForNoteId(int32 NoteId, EPTBActionType InputAction, float InputTimeMs, bool bBroadcastResult = true);

	/** 입력 시간 기준 가장 가까운 대기 노트 조회 */
	bool FindBestPendingNote(EPTBActionType Action, float InputTimeMs, FPTBNoteEvent& OutNote) const;
	
	/** 판정 가능 시간을 넘긴 노트 Miss 처리 */
	TArray<FPTBJudgementResult> ForceMissExpiredNotes(float CurrentTimeMs);
	
	/** 대기 노트와 보정값 같은 런타임 상태 초기화 */
	void Reset();

	/** High Perfect 판정 허용 범위(ms) */
	float HitWindowHighPerfectMs = 21.0;
	
	/** Perfect 판정 허용 범위(ms) */
	float HitWindowPerfectMs = 50.0;
	
	/** Good 판정 허용 범위(ms) */
	float HitWindowGoodMs = 120.0;
	
	/** Miss 입력 소비 허용 범위(ms) */
	float HitWindowMissMs = 250.0;

	/** 노트 판정 범위에서 다른 Action 입력 시 해당 노트를 Miss 처리 */
	bool bMissNoteOnWrongInput = false;

	/** 오입력 Miss 처리 시 같은 타이밍의 다른 Action 노트를 소비할지 여부 */
	bool bSupportsSimultaneousInputs = true;

	/** 동시노트로 간주할 시간 차(ms) */
	float SimultaneousNoteToleranceMs = 1.0f;
	
	/** 판정 대기 노트 목록 */
	TArray<FPTBNoteEvent> PendingNotes;
	
	/** 사용자 판정 보정값(ms) */
	float JudgementOffsetMs = 0.0;

	/** 판정 결과 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Score")
	FOnJudgementResult OnJudgementResult;

};
