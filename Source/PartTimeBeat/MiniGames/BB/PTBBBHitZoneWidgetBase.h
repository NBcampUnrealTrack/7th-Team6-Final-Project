#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "PTBBBHitZoneWidgetBase.generated.h"

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
};
