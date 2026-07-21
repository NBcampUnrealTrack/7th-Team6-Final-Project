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

/** 채보의 모든 노트 판정이 끝난 시점, 그때의 보스 체력이 0이면 발행 (처치 연출 트리거) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBBBOnBossDefeated);

// ─────────────────────────────────────────────────────────────────

/**
 * BB(보스 잡기) 미니게임.
 *
 * 플레이어와 적 AI가 HP를 가지며, 채보에 맞춰 패링 성공/실패로 서로 데미지를 주고받는다.
 * 보스 체력은 채보 전체 노트 수에 정확히 맞춰 깎이는 진행도 지표다 — 모든 노트를 성공해야만
 * 마지막 노트에서 정확히 0이 되며(OnBBBossDefeated), 하나라도 놓치면 완전히 처치되지 않는다.
 * 라운드 실패(Failed)는 플레이어 HP 0 도달 시에만 발생하며, 그 외의 경우 결과 등급은
 * 보스 체력과 무관하게 노트 정확도·미스 수로 결정된다.
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

	/** 채보의 모든 노트 판정이 끝난 시점에 보스 체력이 0이었다면 발행 (최대 1회) */
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

	/** 현재 라운드의 난이도. 보스/HUD 등이 난이도별 연출(텍스처 등)을 적용할 때 사용한다. */
	UFUNCTION(BlueprintPure, Category = "PTB|BB")
	EPTBDifficulty GetDifficulty() const { return GameContext.SessionRequest.Difficulty; }

	/**
	 * 현재 라운드의 채보(ChartAsset)에 해당 액션을 쓰는 노트가 하나라도 있는지 여부.
	 * 난이도마다 채보에 등장하는 액션 종류가 다를 수 있어(예: Easy는 ActionA~C만, Insane은 ActionA~E),
	 * HUD가 히트존 마커를 난이도에 맞게 표시/숨김 처리할 때 사용한다. ChartAsset이 없으면 false.
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|BB")
	bool IsActionUsedInChart(EPTBActionType Action) const;

	UFUNCTION(BlueprintPure, Category = "PTB|BB|Flow")
	bool IsBBIntroSequenceActive() const { return bStartSequenceActive; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|Flow")
	bool IsBBGameplayActive() const { return bIsRoundActive; }

	UFUNCTION(BlueprintPure, Category = "PTB|BB|Flow")
	bool IsBBOutroSequenceActive() const { return bFinishSequenceActive; }

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

	/**
	 * 라운드 종료 경로(Tick의 완료/실패 처리, 페이드 타이머 OnFadeFinished, BGM 종료 콜백 등)와
	 * 무관하게 결과가 확정되기 전에 보스 처치 확정 및 페이드 타이머 정리를 보장한다.
	 * (base의 HandleBGMFinished/HandleAllNotesPassed 등 다른 경로가 OnFadeFinished보다
	 * 먼저 라운드를 끝내더라도, 이 함수가 유일한 공통 진입점이므로 여기서 안전하게 처리한다.)
	 */
	virtual FPTBRoundResult FinishMiniGame(EPTBRoundEndReason Reason)  override;

private:
	/** BBRuleSet 캐스팅 헬퍼 */
	const UPTBBBMiniGameRuleSet* GetBBRuleSet() const;

	/** 보스 체력에 데미지 적용 및 델리게이트 발행 */
	void ApplyBossDamage(float Damage);

	/** 플레이어 체력에 데미지 적용 및 델리게이트 발행 */
	void ApplyPlayerDamage(float Damage);

	/**
	 * 보스 체력이 0인지 확인해 처치를 확정한다(bBossDefeated 가드로 중복 확정 방지).
	 * OnFadeFinished와 FinishMiniGame 양쪽에서 호출되어, 어느 경로로 라운드가 먼저
	 * 끝나든 결과가 확정되기 전에 반드시 처치 여부가 먼저 확정되도록 한다.
	 */
	void ConfirmBossDefeatIfNeeded();

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

	/** 모든 노트 판정이 끝난 시점 기준 보스 처치 확정 여부 (OnFadeFinished에서 설정, 중복 발행 방지) */
	bool bBossDefeated = false;

	/** 이 라운드의 목표 점수 (BuildRuntimeState에서 캐시) */
	int32 CachedTargetScore = 0;

	/** 패링 성공 시 보스에게 주는 데미지 (BuildRuntimeState에서 캐시) */
	float CachedDamagePerParry = 10.f;

	/** Miss 시 플레이어가 받는 데미지 (BuildRuntimeState에서 캐시) */
	float CachedDamageTakenOnMiss = 15.f;

	/** Miss 판정 시 재생할 카메라 쉐이크 (BP_BB_MiniGame 디테일 패널에서 설정) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UCameraShakeBase> MissCameraShakeClass;

	// ── 시간 제한 / BGM 페이드 ───────────────────────────────────

public:
	/** 모든 노트 발행 후 BGM 페이드아웃 지속 시간 (초). 이 시간이 지나면 아웃트로로 전환. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|BB|Timing")
	float BGMFadeDurationSec = 3.0f;

protected:
	virtual void ReceiveGameplayStarted_Implementation() override;

private:
	UFUNCTION()
	void OnAllNotesDispatched();

	void OnFadeFinished();

	FTimerHandle TimeLimitTimerHandle;
};
