#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBBossActor.generated.h"

class APTBBBMiniGame;
class USkeletalMeshComponent;
class UAnimMontage;
class UAkAudioEvent;
struct FPTBNoteEvent;
struct FPTBJudgementResult;

/**
 * BB(보스 잡기) 미니게임 보스 Actor 베이스.
 *
 * BindToMiniGame()으로 APTBBBMiniGame의 델리게이트를 구독하고,
 * 노트 큐 생성(OnBBNoteCue) 및 패링 성공(OnBBParrySuccess) 타이밍에
 * 애니메이션 몽타주와 효과음을 재생한다.
 *
 * BP(BP_BB_Boss)에서 이 클래스를 상속받아 SkeletalMesh를 설정하고,
 * AttackMontages / DefaultAttackMontage / AttackSFXMap / DefaultAttackSFX /
 * HitReactMontage / HitReactSFX를 에디터에서 지정한다.
 * PlayAttackMontage / PlayHitReactMontage는 BlueprintNativeEvent이므로
 * BP에서 재정의해 추가 연출을 구현할 수 있다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBBBBossActor : public AActor
{
	GENERATED_BODY()

public:
	APTBBBBossActor();

	/**
	 * 미니게임에 보스를 연결한다.
	 * Level BP의 BeginPlay 또는 미니게임 시작 이벤트에서 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Boss")
	void BindToMiniGame(APTBBBMiniGame* InMiniGame);

	// ── 애니메이션 이벤트 ─────────────────────────────────────────

	/**
	 * 노트 큐가 생성될 때 호출된다(보스 공격 타이밍).
	 * 기본 구현: AttackMontage 재생 + AttackSFX 재생.
	 * BP에서 재정의해 추가 연출 구현 가능.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|Boss")
	void PlayAttackMontage(const FPTBNoteEvent& Note);
	virtual void PlayAttackMontage_Implementation(const FPTBNoteEvent& Note);

	/**
	 * 패링 성공(보스 피격) 시 호출된다.
	 * 기본 구현: HitReactMontage 재생 + HitReactSFX 재생.
	 * BP에서 재정의해 추가 연출 구현 가능.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|Boss")
	void PlayHitReactMontage(const FPTBJudgementResult& Result);
	virtual void PlayHitReactMontage_Implementation(const FPTBJudgementResult& Result);

	/**
	 * 보스 HP가 0에 처음 도달했을 때 호출된다.
	 * 기본 구현: DeathMontage를 한 번만 재생.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|Boss")
	void PlayDeathMontage();
	virtual void PlayDeathMontage_Implementation();

	// ── 컴포넌트 ─────────────────────────────────────────────────

	/** 보스 스켈레탈 메시. BP에서 메시·애니메이션 BP를 지정한다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|Boss")
	TObjectPtr<USkeletalMeshComponent> BossMesh;

	// ── 에디터 설정 ───────────────────────────────────────────────

	/**
	 * ActionType별 공격 애니메이션 몽타주.
	 * 키: ActionA(Z) ~ ActionE(B). 해당 키가 없으면 DefaultAttackMontage를 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Animation")
	TMap<EPTBActionType, TObjectPtr<UAnimMontage>> AttackMontages;

	/** AttackMontages에 해당 ActionType 항목이 없을 때 사용하는 기본 공격 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Animation")
	TObjectPtr<UAnimMontage> DefaultAttackMontage;

	/** 패링 성공 시 재생할 피격 반응 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/** 보스 HP가 0이 되었을 때 재생할 쓰러짐 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	/**
	 * ActionType별 공격 효과음 Wwise 이벤트.
	 * 키: ActionA(Z) ~ ActionE(B). 해당 키가 없으면 DefaultAttackSFX를 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Audio")
	TMap<EPTBActionType, TObjectPtr<UAkAudioEvent>> AttackSFXMap;

	/** AttackSFXMap에 해당 ActionType 항목이 없을 때 사용하는 기본 공격 효과음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Audio")
	TObjectPtr<UAkAudioEvent> DefaultAttackSFX;

	/** 패링 성공 시 재생할 피격 효과음 Wwise 이벤트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Boss|Audio")
	TObjectPtr<UAkAudioEvent> HitReactSFX;

	/** 현재 바인딩된 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|Boss")
	TObjectPtr<APTBBBMiniGame> BBMiniGame;

	/** 쓰러짐 애니메이션 중복 재생 방지 */
	bool bDeathMontagePlayed = false;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ── 델리게이트 핸들러 ────────────────────────────────────────

	UFUNCTION()
	void HandleBBNoteCue(FPTBNoteEvent Note);

	UFUNCTION()
	void HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent);

	UFUNCTION()
	void HandleBBBossDefeated();
};
