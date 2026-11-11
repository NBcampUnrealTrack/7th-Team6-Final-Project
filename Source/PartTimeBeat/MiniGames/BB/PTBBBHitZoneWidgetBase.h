#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "PTBBBHitZoneWidgetBase.generated.h"

class UWidget;

/**
 * BB 미니게임 히트존 마커 위젯 베이스.
 *
 * 각 ActionType 앵커 위치(HUD의 AnchorPositions)에 고정 배치되어
 * "노트가 여기 도달했을 때 입력해야 한다"는 위치·타이밍을 항상 보여준다.
 * 접근하는 노트 큐(UPTBBBCueWidgetBase)와 달리 화면에서 이동하지 않고 고정된다.
 *
 * WBP_BB_HitZone 등 BB 전용 히트존 위젯이 이 클래스를 부모로 사용한다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBBBHitZoneWidgetBase : public UPTBActionCueWidgetBase
{
	GENERATED_BODY()

public:
	/** ActionType 설정 후 OnActionTypeSet(BP)을 호출해 키 라벨·색상을 반영한다 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|HitZone")
	void InitHitZone(EPTBActionType InActionType);

	/**
	 * 이 히트존이 담당하는 액션의 노트가 정확한 타이밍(판정선 도달)에 도달했을 때 호출된다.
	 * BP에서 강조 펄스 애니메이션을 재생해 "지금 눌러야 함"을 보강 연출한다.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HitZone")
	void PulseHitZone();

	/**
	 * 플레이어가 이 히트존의 키를 누르고 있는지 표시한다 (입력 피드백).
	 * 노트 도달 펄스(PulseHitZone)와 별개로, 키를 누르는 동안 유지되고 떼면 원래대로 돌아간다.
	 * 기본 동작: IMG_PressHighlight 표시/숨김 + PressedScale 적용. 이후 OnPressedChanged(BP) 호출.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|HitZone")
	void SetPressed(bool bInPressed);

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HitZone")
	bool IsPressed() const { return bIsPressed; }

	/** 눌렸을 때의 렌더 스케일 (1.0 = 변화 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|HitZone", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PressedScale = 1.1f;

protected:
	/** 눌림 표시용 위젯(테두리 이미지 등). Designer에 같은 이름으로 두면 자동 연결. 없어도 된다. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> IMG_PressHighlight;

	/** SetPressed 기본 처리 후 호출. BP에서 추가 연출(색 변경·사운드 등) 가능. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|HitZone")
	void OnPressedChanged(bool bPressed);

	virtual void NativeConstruct() override;

private:
	bool bIsPressed = false;
};
