#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBCueWidgetBase.generated.h"

/**
 * BB 미니게임 노트 큐 위젯 베이스.
 *
 * PTBActionCueWidgetBase(키 라벨·색상 제공)를 상속하고
 * BB에서 공통으로 필요한 NoteId 추적과 큐 시작·제거 인터페이스를 추가한다.
 *
 * WBP_BB_AttackCue 등 BB 전용 큐 위젯이 이 클래스를 부모로 사용한다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBBBCueWidgetBase : public UPTBActionCueWidgetBase
{
	GENERATED_BODY()

public:
	/** 이 큐가 나타내는 노트 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|BB|Cue")
	int32 GetNoteId() const { return NoteId; }

	/**
	 * 큐 위젯 초기화.
	 * NoteId를 저장하고 ActionType을 설정(OnActionTypeSet 호출)한 뒤
	 * OnCueStarted()를 발행한다.
	 * @param InApproachDurationSec  큐가 판정선에 도달할 때까지의 접근 시간(초). 애니메이션 속도 계산에 사용.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	virtual void InitCue(int32 InNoteId, EPTBActionType InActionType, float InApproachDurationSec = 0.f);

	/**
	 * 큐 위젯을 즉시 뷰포트에서 제거.
	 * 판정 완료 또는 놓침 처리 후 HUD에서 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	virtual void RemoveCue();

	/**
	 * 판정 결과가 나왔을 때 HUD에서 호출.
	 * BP에서 판정 애니메이션을 재생하고, 애니메이션 종료 시 RemoveCue()를 직접 호출한다.
	 * (HUD는 이 시점에 TMap에서 NoteId를 제거하므로 중복 호출 걱정 없음)
	 */
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnJudgement(EPTBJudgementType JudgementType);

protected:
	/** InitCue() 호출 후 BP에서 접근 애니메이션 시작 등 처리. ApproachDurationSec으로 애니메이션 속도를 조정한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueStarted(int32 InNoteId, EPTBActionType InActionType, float ApproachDurationSec);

	/** RemoveCue() 호출 후 BP에서 페이드아웃 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueRemoved();

	UPROPERTY(BlueprintReadWrite, Category = "PTB|BB|Cue")
	int32 NoteId = 0;
};
