#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBDWMiniGame.generated.h"

class UPTBDWMiniGameRuleSet;

/**
 * 개 산책(DW) 리듬 미니게임 Actor.
 *
 * 공용 리듬 코어(APTBBaseMiniGame) 위에 올라가는 DW의 표현 계층입니다.
 * 이 단계(MVP 2단계)에서는 코어 연결 검증을 위해
 * BuildRuntimeState / HandleNoteCue / HandleJudgementResult 만 override하고
 * 각각 Super를 호출한 뒤 로그만 남깁니다.
 * 마커 보간(Tick), 풀링, 개/주인공 연출은 이후 단계(9단계)에서 추가합니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBDWMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

protected:
	/** 라운드 시작 전 DW 런타임 상태 초기화 (음악 시작 전) */
	virtual void BuildRuntimeState() override;

	/** 선행 비주얼 큐: 개 예고 + 주인공 레인 마커/장애물 스폰 시점 */
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;

	/** 입력 후 판정 결과: 주인공 성공/실패/공입력 연출 */
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

	/** DW 전용 RuleSet 조회 (없으면 nullptr) */
	const UPTBDWMiniGameRuleSet* GetDWRuleSet() const;
};
