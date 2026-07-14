#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "Templates/SubclassOf.h"
#include "PTBDWCharacter.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UMaterialInterface;
class UAnimSequenceBase;
class UAnimInstance;

/** 주인공 placeholder 리액션 모션. 실제 애니 도입 시 삭제 예정. */
enum class EDWReactionMotion : uint8 { None, JumpUp, SlideDown, FailDip, FailHop };

UCLASS()
class PARTTIMEBEAT_API APTBDWCharacter : public AActor
{
	GENERATED_BODY()

public:
	APTBDWCharacter();
	virtual void Tick(float DeltaTime) override;

	void InitPlaceholder(USkeletalMesh* InMesh, FVector InScale, float InYaw, FLinearColor InColor, UMaterialInterface* InBaseMaterial);
	void SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InJump, UAnimSequenceBase* InSlide, UAnimSequenceBase* InFail, TSubclassOf<UAnimInstance> InAnimClass);

	/** 달리기 루프 시작. */
	void StartRunning();

	/** 아이들,런 블렌드용. */
	UFUNCTION(BlueprintPure, Category = "PTB|DW")
	bool IsRunning() const { return bRunning; }

	/** 상체 리액션(D/B실패/E) 재생 중인지. ABP가 UpperBody blend weight 제어에 사용. */
	UFUNCTION(BlueprintPure, Category = "PTB|DW")
	bool IsUpperReacting() const { return bIsUpperReacting; }

	/** 난이도별 모션 배속. */
	void SetMotionPlayRate(float Rate);

	/** 개 예고 — Cue 시점에 짧게 해당 액션을 미리 연기. */
	void PlayCue(EPTBActionType Action, float DurationSec);

	/** 주인공 리액션 — 없으면 placeholder. */
	void PlayReaction(bool bSuccess, EPTBActionType Action, bool bIsLong);

	/** Action D 공치기(성공/실패 공용). */
	void PlayKick();

	/** Action E 성공 리액션(상체 blend). */
	void PlayEReaction();

	/** 상체 리액션 시작 마킹(애님 길이만큼 bIsUpperReacting=true). */
	void BeginUpperReact(UAnimSequenceBase* Anim, float Rate);

	/** preview 상체 소리치기 — Cue마다 상체 슬롯 몽타주 재생(Layered blend per bone 대상). */
	void PlayPreviewShout(UAnimSequenceBase* Anim, FName SlotName, float BlendIn, float BlendOut, float Rate);

	/** 리액션 애님 개별 배속 세팅. SpawnCharactersIfNeeded에서 RuleSet 값으로 1회 호출. */
	void SetReactionRates(float InJumpRate, float InSlideRate, float InFailRate, float InBlendIn = 0.06f, float InBlendOut = 0.06f);

	/** Action D 공치기 애님 세팅(성공/실패 공용). */
	void SetKickAnim(UAnimSequenceBase* Anim, float Rate);

	/** Action E 성공 애님 + 상체 슬롯 이름 세팅. */
	void SetEReaction(UAnimSequenceBase* Anim, float Rate, FName UpperSlot);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USkeletalMeshComponent> Body;

private:
	void PlayJump(bool bIsLong);
	void PlaySlide(bool bIsLong);
	void PlayFail(EPTBActionType Action);

	/** 전용 애니가 있으면 재생, 없으면 placeholder 모션. */
	bool TryPlayReactionAnim(UAnimSequenceBase* Anim, float Rate = 1.0f, FName SlotName = NAME_None);
	void StartPlaceholderReaction(EDWReactionMotion Motion, float DurationSec, float Amplitude);

	/** 개 예고 placeholder 타임라인 갱신. 실제 애니 도입 시 삭제. */
	void UpdateCue(float DeltaTime);

	/** 주인공 리액션 placeholder 타임라인 갱신. 실제 애니 도입 시 삭제. */
	void UpdateReaction(float DeltaTime);

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> RunAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> JumpAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> SlideAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> FailAnim;

	/** 리액션 개별 배속·blend(RuleSet → SetReactionRates로 세팅). */
	float JumpAnimRate    = 1.0f;
	float SlideAnimRate   = 1.0f;
	float FailAnimRate    = 1.0f;
	float ReactionBlendIn  = 0.06f;
	float ReactionBlendOut = 0.06f;

	/** Action D 공치기(성공/실패 공용). */
	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> KickAnim;
	float KickAnimRate = 1.0f;

	/** Action E 성공 리액션. */
	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> EReactionAnim;
	float EReactionRate = 1.0f;

	/** 상체(허리위) 슬롯 이름. 전신은 DefaultSlot. */
	FName UpperSlotName = FName(TEXT("UpperBody"));

	/** 모션 복귀 관련. */
	float AnimReturnTimer = 0.f;
	bool bRunning = false;

	/** 상체 리액션 진행 상태 + 남은 시간(초). */
	bool  bIsUpperReacting = false;
	float UpperReactTimer  = 0.f;

	/** AnimBP 사용 여부. */
	bool bMontageMode = false;

	// ── 개 예고(Cue) placeholder 상태 ──
	bool  bCueActive = false;
	float CueTimer = 0.f;
	float CueDuration = 0.f;
	EPTBActionType CueAction = EPTBActionType::None;

	// ── 주인공 리액션 placeholder 상태 ──
	bool  bReactionActive = false;
	float ReactionTimer = 0.f;
	float ReactionDuration = 0.f;
	float ReactionAmp = 0.f;
	EDWReactionMotion ReactionMotion = EDWReactionMotion::None;
};
