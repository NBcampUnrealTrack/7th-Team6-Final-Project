// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "Core/PTBStructEnums.h"
#include "PTBJJCueWidgetBase.generated.h"


UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBJJCueWidgetBase : public UPTBActionCueWidgetBase
{
	GENERATED_BODY()
public:
	/** 이 큐가 나타내는 노트 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|TG|Cue")
	int32 GetNoteId() const { return NoteId; }

	/**
	 * 큐 위젯 공통 초기화.
	 * NoteId를 저장하고 ActionType을 설정(OnActionTypeSet 호출)한 뒤
	 * OnCueStarted()를 발행한다. WBP에서 InitCue()를 구현할 때 Super로 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JJ|Cue")
	virtual void InitCue(int32 InNoteId, EPTBActionType InActionType);

	/**
	 * 큐 위젯을 즉시 뷰포트에서 제거.
	 * 판정 완료 또는 놓침 처리 후 WBP_TG_HUD에서 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JJ|Cue")
	virtual void RemoveCue();

protected:
	/** InitCue() 호출 후 BP에서 접근 애니메이션 시작 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JJ|Cue")
	void OnCueStarted(int32 InNoteId, EPTBActionType InActionType);

	/** RemoveCue() 호출 직전에 BP에서 정리 작업(사운드 중지/상태 플래그 등) 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JJ|Cue")
	void OnCueRemoved();

	UPROPERTY(BlueprintReadWrite, Category = "PTB|JJ|Cue")
	int32 NoteId = 0;
};
