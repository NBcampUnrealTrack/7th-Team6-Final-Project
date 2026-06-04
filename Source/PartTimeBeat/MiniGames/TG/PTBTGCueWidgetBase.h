#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "Core/PTBStructEnums.h"
#include "PTBTGCueWidgetBase.generated.h"

/**
 * TG 미니게임 노트 큐 위젯 베이스.
 *
 * PTBActionCueWidgetBase(키 라벨·색상 제공)를 상속하고
 * TG에서 공통으로 필요한 NoteId 추적과 큐 시작·제거 인터페이스를 추가한다.
 *
 * WBP_TG_TapCue, WBP_TG_HoldCue 두 위젯이 이 클래스를 부모로 사용한다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBTGCueWidgetBase : public UPTBActionCueWidgetBase
{
	GENERATED_BODY()

public:
	/** 이 큐가 나타내는 노트 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|TG|Cue")
	int32 GetNoteId() const { return NoteId; }

	/**
	 * 큐 위젯 공통 초기화.
	 * NoteId를 저장하고 ActionType을 설정(OnActionTypeSet 호출)한 뒤
	 * OnCueStarted()를 발행한다. WBP에서 StartCue()를 구현할 때 Super로 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TG|Cue")
	virtual void InitCue(int32 InNoteId, EPTBActionType InActionType);

	/**
	 * 큐 위젯을 즉시 뷰포트에서 제거.
	 * 판정 완료 또는 놓침 처리 후 WBP_TG_HUD에서 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|TG|Cue")
	virtual void RemoveCue();

protected:
	/** InitCue() 호출 후 BP에서 접근 애니메이션 시작 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|TG|Cue")
	void OnCueStarted(int32 InNoteId, EPTBActionType InActionType);

	/** RemoveCue() 호출 후 BP에서 페이드아웃 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|TG|Cue")
	void OnCueRemoved();

	UPROPERTY(BlueprintReadWrite, Category = "PTB|TG|Cue")
	int32 NoteId = 0;
};
