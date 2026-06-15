#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBBBPlayerActor.generated.h"

class APTBBBMiniGame;
class USkeletalMeshComponent;
class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class UAkAudioEvent;
struct FPTBJudgementResult;
struct FPTBProfileData;

/**
 * BB(보스 잡기) 미니게임 플레이어 Actor 베이스.
 *
 * BindToMiniGame()으로 APTBBBMiniGame의 델리게이트를 구독한다.
 *  - 패링 성공(OnBBParrySuccess) → PlayAttackMontage() : ActionType별 공격 애니메이션
 *  - 패링 실패(OnBBParryFail)   → PlayDamageMontage() : 피격 애니메이션
 *
 * 프로필 캐릭터 적용:
 *  BindToMiniGame() 호출 시 ProfileSubsystem에서 활성 프로필을 조회하고
 *  ApplyProfileCharacter()를 자동으로 호출한다.
 *  CharacterMeshByGender / AnimBlueprintByGender에 Gender별 에셋을 에디터에서 지정한다.
 *
 * BP(BP_BB_Player)에서 이 클래스를 상속받아 에셋을 설정하고,
 * 각 BlueprintNativeEvent에서 추가 연출을 구현할 수 있다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBBBPlayerActor : public AActor
{
	GENERATED_BODY()

public:
	APTBBBPlayerActor();

	/**
	 * 미니게임에 플레이어를 연결한다.
	 * Level BP의 BeginPlay 또는 미니게임 시작 이벤트에서 호출한다.
	 * ProfileSubsystem에서 활성 프로필을 조회해 ApplyProfileCharacter()를 자동 호출한다.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Player")
	void BindToMiniGame(APTBBBMiniGame* InMiniGame);

	// ── 애니메이션 이벤트 ─────────────────────────────────────────

	/**
	 * 패링 성공 시 호출된다(플레이어 공격 타이밍).
	 * 기본 구현: Result.ActionType에 맞는 AttackMontages 재생 + AttackSFXMap 재생.
	 * BP에서 재정의해 추가 연출 구현 가능.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|Player")
	void PlayAttackMontage(const FPTBJudgementResult& Result);
	virtual void PlayAttackMontage_Implementation(const FPTBJudgementResult& Result);

	/**
	 * 패링 실패(Miss) 시 호출된다.
	 * 기본 구현: DamageMontage 재생 + DamageSFX 재생.
	 * BP에서 재정의해 추가 연출 구현 가능.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|BB|Player")
	void PlayDamageMontage(const FPTBJudgementResult& Result);
	virtual void PlayDamageMontage_Implementation(const FPTBJudgementResult& Result);

	/**
	 * 프로필 데이터를 기반으로 캐릭터 메시와 AnimBP를 적용한다.
	 * 기본 구현: Profile.Gender로 CharacterMeshByGender / AnimBlueprintByGender를 조회.
	 * BP에서 재정의해 커스텀 로직 구현 가능.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PTB|BB|Player")
	void ApplyProfileCharacter(const FPTBProfileData& Profile);
	virtual void ApplyProfileCharacter_Implementation(const FPTBProfileData& Profile);

	// ── 컴포넌트 ─────────────────────────────────────────────────

	/** 플레이어 스켈레탈 메시. ApplyProfileCharacter에서 런타임에 교체된다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|BB|Player")
	TObjectPtr<USkeletalMeshComponent> PlayerMesh;

	// ── 에디터 설정 : 애니메이션 ─────────────────────────────────

	/**
	 * ActionType별 공격 애니메이션 몽타주.
	 * 키: ActionA(Z) ~ ActionE(B). 해당 키가 없으면 DefaultAttackMontage를 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Animation")
	TMap<EPTBActionType, TObjectPtr<UAnimMontage>> AttackMontages;

	/** AttackMontages에 해당 ActionType 항목이 없을 때 사용하는 기본 공격 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Animation")
	TObjectPtr<UAnimMontage> DefaultAttackMontage;

	/** 패링 실패(Miss) 시 재생할 피격 애니메이션 몽타주 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Animation")
	TObjectPtr<UAnimMontage> DamageMontage;

	// ── 에디터 설정 : 효과음 ─────────────────────────────────────

	/**
	 * ActionType별 공격 효과음 Wwise 이벤트.
	 * 키: ActionA(Z) ~ ActionE(B). 해당 키가 없으면 DefaultAttackSFX를 사용한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Audio")
	TMap<EPTBActionType, TObjectPtr<UAkAudioEvent>> AttackSFXMap;

	/** AttackSFXMap에 해당 ActionType 항목이 없을 때 사용하는 기본 공격 효과음 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Audio")
	TObjectPtr<UAkAudioEvent> DefaultAttackSFX;

	/** 패링 실패(Miss) 시 재생할 피격 효과음 Wwise 이벤트 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Audio")
	TObjectPtr<UAkAudioEvent> DamageSFX;

	// ── 에디터 설정 : 캐릭터 ─────────────────────────────────────

	/**
	 * 성별별 캐릭터 스켈레탈 메시.
	 * ApplyProfileCharacter 기본 구현이 Profile.Gender로 조회한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Character")
	TMap<EPTBGender, TObjectPtr<USkeletalMesh>> CharacterMeshByGender;

	/**
	 * 성별별 애니메이션 블루프린트 클래스.
	 * ApplyProfileCharacter 기본 구현이 Profile.Gender로 조회한다.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Player|Character")
	TMap<EPTBGender, TSubclassOf<UAnimInstance>> AnimBlueprintByGender;

	/** 현재 바인딩된 미니게임 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|BB|Player")
	TObjectPtr<APTBBBMiniGame> BBMiniGame;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ── 내부 헬퍼 ────────────────────────────────────────────────

	/** AnimInstance를 안전하게 가져온다. 없으면 nullptr과 함께 경고 출력. */
	UAnimInstance* GetPlayerAnimInstance() const;

	// ── 델리게이트 핸들러 ────────────────────────────────────────

	UFUNCTION()
	void HandleBBParrySuccess(FPTBJudgementResult Result, float BossHPPercent);

	UFUNCTION()
	void HandleBBParryFail(FPTBJudgementResult Result, float PlayerHPPercent);
};