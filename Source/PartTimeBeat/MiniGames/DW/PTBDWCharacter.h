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

UCLASS()
class PARTTIMEBEAT_API APTBDWCharacter : public AActor
{
	GENERATED_BODY()

public:
	APTBDWCharacter();
	virtual void Tick(float DeltaTime) override;

	void InitPlaceholder(USkeletalMesh* InMesh, FVector InScale, float InYaw, FLinearColor InColor, UMaterialInterface* InBaseMaterial);
	void SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InActA, UAnimSequenceBase* InActB, UAnimSequenceBase* InFail, TSubclassOf<UAnimInstance> InAnimClass);

	/** 달리기 루프 시작. */
	void StartRunning();

	/** 아이들,런 블렌드용. */
	UFUNCTION(BlueprintPure, Category = "PTB|DW")
	bool IsRunning() const { return bRunning; }

	/** 상체 리액션(B/D/E실패) 재생 중인지. ABP가 UpperBody blend 제어에 사용. */
	UFUNCTION(BlueprintPure, Category = "PTB|DW")
	bool IsUpperReacting() const { return bIsUpperReacting; }

	/** 난이도별 모션 배속. */
	void SetMotionPlayRate(float Rate);

	/** 주인공 리액션. */
	void PlayReaction(bool bSuccess, EPTBActionType Action, bool bIsLong);

	/** Action D 공치기. */
	void PlayActD();

	/** Action E 성공 리액션(상체 blend). */
	void PlayActE();

	/** 상체 리액션 시작 마킹. */
	void BeginUpperReact(UAnimSequenceBase* Anim, float Rate);

	/** preview 상체 소리치기. */
	void PlayPreviewShout(UAnimSequenceBase* Anim, FName SlotName, float BlendIn, float BlendOut, float Rate);

	/** 리액션 애님 개별 배속 세팅. */
	void SetReactionRates(float InActARate, float InActBRate, float InFailRate, float InBlendIn = 0.06f, float InBlendOut = 0.06f);

	/** Action D 공 애님 세팅. */
	void SetActDAnim(UAnimSequenceBase* Anim, float Rate);

	/** Action E 성공 애님 + 상체 슬롯 이름 세팅. */
	void SetActEAnim(UAnimSequenceBase* Anim, float Rate, FName UpperSlot);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USkeletalMeshComponent> Body;

private:
	void PlayActA(bool bIsLong);
	void PlayActB(bool bIsLong);
	void PlayFail(EPTBActionType Action);

	/** 전용 애니가 있으면 재생. */
	bool TryPlayReactionAnim(UAnimSequenceBase* Anim, float Rate = 1.0f, FName SlotName = NAME_None);

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> RunAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> ActAAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> ActBAnim;

	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> FailAnim;

	/** 리액션 개별 배속·blend시간. */
	float ActARate    = 1.0f;
	float ActBRate   = 1.0f;
	float FailAnimRate    = 1.0f;
	float ReactionBlendIn  = 0.06f;
	float ReactionBlendOut = 0.06f;

	/** Action D 공 애님 속도. */
	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> ActDAnim;
	float ActDRate = 1.0f;

	/** Action E 성공 애님 속도. */
	UPROPERTY()
	TObjectPtr<UAnimSequenceBase> ActEAnim;
	float ActERate = 1.0f;

	/** 상체 슬롯 이름. 전신은 DefaultSlot. */
	FName UpperSlotName = FName(TEXT("UpperBody"));

	/** 모션 복귀. */
	float AnimReturnTimer = 0.f;
	bool bRunning = false;

	/** 상체 리액션 진행 상태. */
	bool  bIsUpperReacting = false;
	float UpperReactTimer  = 0.f;

	/** AnimBP 사용 여부. */
	bool bMontageMode = false;

};
