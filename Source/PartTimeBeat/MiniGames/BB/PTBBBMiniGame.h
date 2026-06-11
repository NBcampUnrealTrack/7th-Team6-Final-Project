#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBBBMiniGame.generated.h"

class UPTBBBMiniGameRuleSet;

// ── BP 연출용 델리게이트 ──────────────────────────────────────────

/** 노트 예고 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBBBOnNoteCue,
	FPTBNoteEvent, Note);

/** 노트 판정 대기 진입 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBBBOnNoteArm,
	FPTBNoteEvent, Note);

/** 노트 정시점 도달 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPTBBBOnNoteReached,
	FPTBNoteEvent, Note);

/** 패링 성공(HighPerfect/Perfect/Good) - 판정 결과 + 보스 HP 비율 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBBBOnParrySuccess,
	FPTBJudgementResult, Result, float, BossHPPercent);

/** 패링 실패(Miss) - 판정 결과 + 플레이어 HP 비율 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBBBOnParryFail,
	FPTBJudgementResult, Result, float, PlayerHPPercent);

/** 보스/플레이어 HP 변경 - 새 HP 값, 최대 HP */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBBBOnHPChanged,
	float, NewHP, float, MaxHP);

/** 보스 HP 0 도달 (곡이 끝날 때까지 게임 계속) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBBBOnBossDefeated);

// ─────────────────────────────────────────────────────────────────

/**
 * BB(보스 잡기) 미니게임.
 *
 * 플레이어와 적 AI가 HP를 가지며, 채보에 맞춰 패링 성공/실패로 서로 데미지를 주고받는다.
 * 보스 HP 0 도달 시 게임이 끝나지 않고 곡이 끝날 때까지 계속 진행된다.
 * 최종 결과는 보스 HP 잔량, 플레이어 HP 잔량, 목표 점수 도달 여부로 결정된다.
 *
 * 입력 키 → Action 매핑: Z=ActionA  X=ActionB  C=ActionC  V=ActionD  B=ActionE
 */
UCLASS()
class PARTTIMEBEAT_API APTBBBMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	// ── 연출 델리게이트 ──────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnNoteCue OnBBNoteCue;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnNoteArm OnBBNoteArm;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnNoteReached OnBBNoteReached;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnParrySuccess OnBBParrySuccess;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnParryFail OnBBParryFail;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnHPChanged OnBBBossHPChanged;

	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnHPChanged OnBBPlayerHPChanged;

	/** 보스 HP가 처음 0이 되는 순간 발행 (이후 데미지에는 발행 안 함) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|BB|Events")
	FPTBBBOnBossDefeated OnBBBossDefeated;

	// ── 런타임 상태 조회 ─────────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetBossHP() const { return BossCurrentHP; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetBossMaxHP() const { return BossMaxHP; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetBossHPPercent() const;

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetPlayerHP() const { return PlayerCurrentHP; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetPlayerMaxHP() const { return PlayerMaxHP; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|HP")
	float GetPlayerHPPercent() const;

	UFUNCTION(BlueprintPure, Category = "PTB|BB")
	bool IsBossDefeated() const { return bBossDefeated; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB")
	int32 GetTargetScore() const { return CachedTargetScore; }

	// ── 입력 편의 함수 (z/x/c/v/b 키) ───────────────────────────

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Input")
	void HandleActionAInput(float TimeMs = -1.0f);   // Z

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Input")
	void HandleActionBInput(float TimeMs = -1.0f);   // X

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Input")
	void HandleActionCInput(float TimeMs = -1.0f);   // C

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Input")
	void HandleActionDInput(float TimeMs = -1.0f);   // V

	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Input")
	void HandleActionEInput(float TimeMs = -1.0f);   // B

protected:
	virtual void BuildRuntimeState()                                  override;
	virtual void PreloadAudioAssets()                                 override;
	virtual void HandleNoteCue(FPTBNoteEvent Note)                    override;
	virtual void HandleNoteArm(FPTBNoteEvent Note)                    override;
	virtual void HandleChartEvent(FPTBNoteEvent Note)                 override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result)    override;
	virtual FPTBMiniGameResultPayload BuildResultPayload() const      override;
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const       override;

private:
	/** BBRuleSet 캐스팅 헬퍼 */
	const UPTBBBMiniGameRuleSet* GetBBRuleSet() const;

	/** 보스 체력에 데미지 적용 및 델리게이트 발행 */
	void ApplyBossDamage(float Damage);

	/** 플레이어 체력에 데미지 적용 및 델리게이트 발행 */
	void ApplyPlayerDamage(float Damage);

	// ── 런타임 상태 ──────────────────────────────────────────────

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (AllowPrivateAccess = "true"))
	float BossCurrentHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (AllowPrivateAccess = "true"))
	float BossMaxHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (AllowPrivateAccess = "true"))
	float PlayerCurrentHP = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|HP",
		meta = (AllowPrivateAccess = "true"))
	float PlayerMaxHP = 100.f;

	/** 보스 HP가 이미 0에 도달했는지 (중복 발행 방지) */
	bool bBossDefeated = false;

	/** 이 라운드의 목표 점수 (BuildRuntimeState에서 캐시) */
	int32 CachedTargetScore = 0;

	/** 패링 성공 시 보스에게 주는 데미지 (BuildRuntimeState에서 캐시) */
	float CachedDamagePerParry = 10.f;

	/** Miss 시 플레이어가 받는 데미지 (BuildRuntimeState에서 캐시) */
	float CachedDamageTakenOnMiss = 15.f;
};
