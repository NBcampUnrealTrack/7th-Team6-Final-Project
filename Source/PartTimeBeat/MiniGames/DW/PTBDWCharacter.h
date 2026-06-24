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

	/** 개 예고 — Cue 시점에 짧게 해당 액션을 미리 연기. */
	void PlayCue(EPTBActionType Action, float DurationSec);

	/** 주인공 리액션 — 없으면 placeholder. */
	void PlayReaction(bool bSuccess, EPTBActionType Action, bool bIsLong);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USkeletalMeshComponent> Body;

private:
	void PlayJump(bool bIsLong);
	void PlaySlide(bool bIsLong);
	void PlayFail(EPTBActionType Action);

	/** 전용 애니가 있으면 재생, 없으면 placeholder 모션. */
	bool TryPlayReactionAnim(UAnimSequenceBase* Anim);
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

	/** 모션 복귀 관련. */
	float AnimReturnTimer = 0.f;
	bool bRunning = false;

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
