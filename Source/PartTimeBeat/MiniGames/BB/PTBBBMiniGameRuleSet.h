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

	/** Miss 판정 시 플레이어가 받는 데미지 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Damage",
		meta = (ClampMin = "0.0"))
	float DamageTakenOnMiss = 15.f;

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
 * 모든 노트가 발행된 시점에 보스 체력이 0이거나 미스 수가 BossDefeatMaxMissCount
 * 이하일 때 결정된다. 실패(Failed)는 플레이어 체력 0 도달(bFailOnPlayerHPDepleted) 시에만 발생한다.
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBBBMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/**
	 * 미스 수가 이 값 이하면, 보스 체력이 진행도 계산상 완전히 0이 되지 않았어도
	 * 처치(쓰러짐, OnBBBossDefeated)로 인정한다. 모든 난이도 공통 값.
	 * (보스 체력 자체는 항상 채보 노트 수에 정확히 1:1로 맞춰 깎이는 진행도 지표라서,
	 * 미스가 하나라도 있으면 마지막 노트에서 정확히 0에 도달하지 않기 때문에 별도로 필요하다.)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (ClampMin = "0"))
	int32 BossDefeatMaxMissCount = 5;

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
