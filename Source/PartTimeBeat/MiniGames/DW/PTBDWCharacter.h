#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h"
#include "PTBDWCharacter.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USkeletalMesh;
class UMaterialInterface;
class UAnimSequenceBase;

UCLASS()
class PARTTIMEBEAT_API APTBDWCharacter : public AActor
{
	GENERATED_BODY()

public:
	APTBDWCharacter();
	virtual void Tick(float DeltaTime) override;

	void InitPlaceholder(USkeletalMesh* InMesh, FVector InScale, float InYaw, FLinearColor InColor, UMaterialInterface* InBaseMaterial);
	void SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InJump);

	/** 달리기 루프 시작. */
	void StartRunning();

	/** 개 예고 — 지금은 미연결. */
	void PlayCue(EPTBActionType Action);

	/** 주인공 리액션 — 지금은 Action A 성공만 점프 재생. */
	void PlayReaction(bool bSuccess, EPTBActionType Action);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USkeletalMeshComponent> Body;

private:
	void PlayJump();

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> RunAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> JumpAnim;

	float JumpReturnTimer = 0.f;
	bool bRunning = false;
};
