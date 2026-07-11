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

	/**
	 * 패링 성공(HighPerfect/Perfect/Good) 시 보스에게 주는 데미지.
	 * bAutoScaleBossHP = true일 때는 무시되며, BossMaxHP / RequiredParries로 자동 계산된다.
	 * bAutoScaleBossHP = false일 때만 이 값이 직접 사용된다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.0"))
	float DamagePerParry = 10.f;

	/** Miss 판정 시 플레이어가 받는 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.0"))
	float DamageTakenOnMiss = 15.f;

	/**
	 * bAutoScaleBossHP 사용 시 클리어(보스 처치)에 필요한 패링 성공 비율.
	 * 1.0(기본) → 채보의 모든 노트를 성공해야 마지막 노트에서 보스 HP가 정확히 0이 된다.
	 * 즉, 보스는 노트가 모두 발행되는 시점 이전에는 처치될 수 없다.
	 * 1.0 미만으로 낮추면 그만큼 더 적은 성공으로도 보스 HP가 중간에 0에 도달할 수 있다.
	 * 범위: 0.01 ~ 1.0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float ClearRatio = 1.0f;

	/** 이 난이도의 목표 점수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Score",
		meta = (ClampMin = "0"))
	int32 TargetScore = 50000;
};

/**
 * BB(보스 잡기) 미니게임 RuleSet.
 *
 * 공통 RuleSet 위에 HP 시스템과 난이도별 데미지 설정을 추가한다.
 * 보스 HP가 0이 되어도 곡이 끝날 때까지 게임이 진행되며, 처치(쓰러짐 연출) 확정은
 * 모든 노트가 발행된 시점의 보스 체력으로 결정된다. 실패(Failed)는 플레이어 체력
 * 0 도달(bFailOnPlayerHPDepleted) 시에만 발생한다.
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
	 * true(기본)이면 플레이어 HP 0 도달 즉시 Failed 처리한다(BB의 유일한 실패 조건).
	 * false이면 플레이어가 쓰러져도 곡이 끝날 때까지 계속 진행하며 Failed가 되지 않는다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Failure")
	bool bFailOnPlayerHPDepleted = true;

	/** 난이도별 데미지/목표 점수 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Difficulty")
	TMap<EPTBDifficulty, FPTBBBDifficultyConfig> DifficultyConfigs;

	/** 현재 난이도 설정 조회. 없으면 기본값 반환 */
	FPTBBBDifficultyConfig GetDifficultyConfig(EPTBDifficulty Difficulty) const;
};
