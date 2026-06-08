#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBDWMiniGameRuleSet.generated.h"

/**
 * 개 산책(DW) 미니게임 전용 RuleSet DataAsset.
 *
 * 공통 판정 window, 점수 계산, 결과 구조, Flow 상태는 베이스(UPTBMiniGameRuleSet)와
 * 공통 코드가 담당합니다. 여기에는 DW의 View 전용 연출 파라미터만 둡니다.
 *
 * 공통 필드(MiniGameId/MiniGameCode/SupportedActions/ChartAssetsByDifficulty/
 * CueLeadTimeMode/LookAheadBeats/EmptyInputPolicy/SFX 키 등)는 베이스에서 상속받아
 * DataAsset(DA_DW_RuleSet)에서 설정합니다.
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBDWMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/**
	 * 액션별 장애물 오프셋 α (판정선 대비 진행방향 앞쪽, cm).
	 * 채보가 아닌 순수 View 튜닝값. 마커는 정시점에 판정선에 닿고,
	 * 장애물은 이 값만큼 앞에 배치되어 "입력 후 잠시 뒤 도달" 연출을 만든다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	TMap<EPTBActionType, float> ObstacleOffsetByAction;

	/** 개 예고 연출 최대 길이(박). 0.2~0.3초 제자리 원칙 강제용 상한. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float DogCueMaxBeats = 0.5f;

	/**
	 * 마커 스폰 위치(판정선 기준 진행방향 뒤 거리, cm).
	 * 마커 도달 시점은 음악 시간 보간이 보장하므로, 이 값은 순수 화면 프레이밍이다.
	 * 너무 멀면 "맵 한가운데서 등장", 너무 가까우면 예고가 짧게 느껴진다 → 눈으로 튜닝.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float MarkerSpawnDistance = 1200.0f;
};
