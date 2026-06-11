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
	UPTBBBCueWidgetBase();

	/** 이 큐가 나타내는 노트 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|BB|Cue")
	int32 GetNoteId() const { return NoteId; }

	/**
	 * 큐 위젯 초기화.
	 * NoteId를 저장하고 ActionType을 설정(OnActionTypeSet 호출)한 뒤
	 * OnCueStarted()를 발행한다. ApproachStartTranslationX가 설정되어 있으면
	 * NativeTick 기반 Translation 이동을 자동으로 시작한다.
	 * @param InApproachDurationSec  큐가 판정선에 도달할 때까지의 접근 시간(초).
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

	/**
	 * Translation 이동을 즉시 멈추고 히트존(X=0)에 스냅한다.
	 * Blueprint의 OnJudgement 시작 시점에 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Cue")
	void StopApproachTranslation();

	/**
	 * 스폰 시 HUD가 설정하는 초기 이동 오프셋.
	 * 양수면 보스가 히트존 오른쪽, 음수면 왼쪽에 위치함을 의미한다.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|Cue")
	float ApproachStartTranslationX = 0.f;

protected:
	/** InitCue() 호출 후 BP에서 접근 애니메이션 시작 등 처리. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueStarted(int32 InNoteId, EPTBActionType InActionType, float ApproachDurationSec);

	/** RemoveCue() 호출 후 BP에서 페이드아웃 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Cue")
	void OnCueRemoved();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(BlueprintReadWrite, Category = "PTB|BB|Cue")
	int32 NoteId = 0;

private:
	bool bApproaching = false;
	float ApproachElapsedSec = 0.f;
	float ApproachTotalDurationSec = 0.f;
};
