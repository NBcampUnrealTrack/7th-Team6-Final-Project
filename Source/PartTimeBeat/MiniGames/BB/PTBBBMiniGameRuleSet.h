#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBBBMiniGameRuleSet.generated.h"

/**
 * 난이도별 BB 미니게임 전용 수치 설정.
 * 패링 성공/실패 데미지와 목표 점수가 난이도마다 달라진다.
 */
USTRUCT(BlueprintType)
struct FPTBBBDifficultyConfig
{
	GENERATED_BODY()

	/** 패링 성공(HighPerfect/Perfect/Good) 시 보스에게 주는 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.0"))
	float DamagePerParry = 10.f;

	/** Miss 판정 시 플레이어가 받는 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.0"))
	float DamageTakenOnMiss = 15.f;

	/**
	 * bAutoScaleBossHP 사용 시 클리어에 필요한 패링 성공 비율.
	 * 예) 0.667 → 전체 노트의 2/3를 성공하면 보스 HP가 정확히 0이 된다.
	 * 범위: 0.01 ~ 1.0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ClearRatio = 0.667f;

	/** 이 난이도의 목표 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Score",
		meta = (ClampMin = "0"))
	int32 TargetScore = 50000;
};

/**
 * BB(보스 잡기) 미니게임 RuleSet.
 *
 * 공통 RuleSet 위에 HP 시스템과 난이도별 데미지 설정을 추가한다.
 * 보스 HP가 0이 되어도 곡이 끝날 때까지 게임이 진행된다.
 * (bBossDefeated 플래그만 세우고 연출 이벤트를 발행)
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBBBMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/**
	 * true이면 런타임에 채보의 Tap/Hold 노트 수 × DamagePerParry로 BossMaxHP를 자동 계산한다.
	 * 모든 노트를 완벽하게 쳐야 마지막 노트에서 보스가 정확히 처치된다.
	 * false이면 아래 BossMaxHP 고정값을 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP")
	bool bAutoScaleBossHP = true;

	/** bAutoScaleBossHP가 false일 때 사용하는 고정 보스 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (ClampMin = "1.0", EditCondition = "!bAutoScaleBossHP"))
	float BossMaxHP = 100.f;

	/** 플레이어 최대 체력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (ClampMin = "1.0"))
	float PlayerMaxHP = 100.f;

	/**
	 * 플레이어 HP가 0이 되면 라운드를 즉시 실패 처리할지 여부.
	 * false(기본)이면 곡이 끝날 때까지 계속 진행한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Failure")
	bool bFailOnPlayerHPDepleted = false;

	/** 난이도별 데미지/목표 점수 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Difficulty")
	TMap<EPTBDifficulty, FPTBBBDifficultyConfig> DifficultyConfigs;

	/** 현재 난이도 설정 조회. 없으면 기본값 반환 */
	FPTBBBDifficultyConfig GetDifficultyConfig(EPTBDifficulty Difficulty) const;
};
