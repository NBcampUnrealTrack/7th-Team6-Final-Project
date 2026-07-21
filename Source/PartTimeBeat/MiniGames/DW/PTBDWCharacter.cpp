#include "MiniGames/DW/PTBDWCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"


APTBDWCharacter::APTBDWCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APTBDWCharacter::InitPlaceholder(USkeletalMesh* InMesh, FVector InScale, float InYaw, FLinearColor InColor, UMaterialInterface* InBaseMaterial)
{
	if (InMesh)
	{
		Body->SetSkeletalMeshAsset(InMesh);
	}
	Body->SetRelativeScale3D(InScale);
	Body->SetRelativeLocation(FVector::ZeroVector);
	Body->SetRelativeRotation(FRotator(0.f, InYaw, 0.f));

	if (InBaseMaterial)
	{
		if (UMaterialInstanceDynamic* MID = Body->CreateDynamicMaterialInstance(0, InBaseMaterial))
		{
			MID->SetVectorParameterValue(TEXT("Color"), InColor);
		}
	}
}

void APTBDWCharacter::SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InActA, UAnimSequenceBase* InActB, UAnimSequenceBase* InFail, TSubclassOf<UAnimInstance> InAnimClass)
{
	RunAnim = InRun;
	ActAAnim = InActA;
	ActBAnim = InActB;
	FailAnim = InFail;

	bMontageMode = (InAnimClass != nullptr);
	if (bMontageMode && Body)
	{
		Body->SetAnimInstanceClass(InAnimClass);
	}
}

void APTBDWCharacter::SetMotionPlayRate(float Rate)
{
	if (Body)
	{
		Body->GlobalAnimRateScale = FMath::Max(0.01f, Rate);
	}
}

void APTBDWCharacter::StartRunning()
{
	bRunning = true;
	AnimReturnTimer = 0.f;
	if (bMontageMode)
	{
		return;
	}
	if (RunAnim)
	{
		Body->PlayAnimation(RunAnim, true);
	}
}

void APTBDWCharacter::PlayReaction(bool bSuccess, EPTBActionType Action, bool bIsLong)
{
	if (bSuccess)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: PlayActA(bIsLong);  break;
		case EPTBActionType::ActionB: PlayActB(bIsLong); break;
		case EPTBActionType::ActionD: PlayActD();         break;
		case EPTBActionType::ActionE: PlayActE();    break;
		default: break;
		}
		return;
	}

	if (Action == EPTBActionType::ActionA)
	{
		return;
	}
	if (Action == EPTBActionType::ActionD)
	{
		PlayActD();
		return;
	}

	PlayFail(Action);
}

void APTBDWCharacter::SetReactionRates(float InActARate, float InActBRate, float InFailRate, float InBlendIn, float InBlendOut)
{
	ActARate    = FMath::Max(0.01f, InActARate);
	ActBRate   = FMath::Max(0.01f, InActBRate);
	FailAnimRate    = FMath::Max(0.01f, InFailRate);
	ReactionBlendIn  = FMath::Max(0.0f,  InBlendIn);
	ReactionBlendOut = FMath::Max(0.0f,  InBlendOut);
}

void APTBDWCharacter::SetActDAnim(UAnimSequenceBase* Anim, float Rate)
{
	ActDAnim     = Anim;
	ActDRate = FMath::Max(0.01f, Rate);
}

void APTBDWCharacter::SetActEAnim(UAnimSequenceBase* Anim, float Rate, FName UpperSlot)
{
	ActEAnim = Anim;
	ActERate = FMath::Max(0.01f, Rate);
	if (!UpperSlot.IsNone())
	{
		UpperSlotName = UpperSlot;
	}
}

void APTBDWCharacter::PlayActE()
{
	BeginUpperReact(ActEAnim, ActERate);
	TryPlayReactionAnim(ActEAnim, ActERate, UpperSlotName);
}

void APTBDWCharacter::BeginUpperReact(UAnimSequenceBase* Anim, float Rate)
{
	if (!Anim)
	{
		return;
	}
	const float SafeRate = FMath::Max(0.01f, Rate);
	UpperReactTimer  = Anim->GetPlayLength() / SafeRate;
	bIsUpperReacting = (UpperReactTimer > 0.f);
}

void APTBDWCharacter::PlayActD()
{
	BeginUpperReact(ActDAnim, ActDRate);
	TryPlayReactionAnim(ActDAnim, ActDRate, UpperSlotName);
}

void APTBDWCharacter::PlayPreviewShout(UAnimSequenceBase* Anim, FName SlotName, float BlendIn, float BlendOut, float Rate)
{
	if (!Anim || !Body)
	{
		return;
	}
	if (UAnimInstance* AnimInst = Body->GetAnimInstance())
	{
		AnimInst->PlaySlotAnimationAsDynamicMontage(Anim, SlotName, FMath::Max(0.f, BlendIn), FMath::Max(0.f, BlendOut), FMath::Max(0.01f, Rate));
	}
}

bool APTBDWCharacter::TryPlayReactionAnim(UAnimSequenceBase* Anim, float Rate, FName SlotName)
{
	if (!Anim || !Body)
	{
		return false;
	}

	const float SafeRate = FMath::Max(0.01f, Rate);
	if (bMontageMode)
	{
		if (UAnimInstance* AnimInst = Body->GetAnimInstance())
		{
			const FName UseSlot = SlotName.IsNone() ? FName(TEXT("DefaultSlot")) : SlotName;
			AnimInst->PlaySlotAnimationAsDynamicMontage(Anim, UseSlot, ReactionBlendIn, ReactionBlendOut, SafeRate);
			return true;
		}
	}

	Body->PlayAnimation(Anim, false);
	AnimReturnTimer = Anim->GetPlayLength() / SafeRate;
	return true;
}

void APTBDWCharacter::PlayActA(bool bIsLong)
{
	bIsUpperReacting = false;
	UpperReactTimer  = 0.f;
	TryPlayReactionAnim(ActAAnim, ActARate, FName(TEXT("DefaultSlot")));
}

void APTBDWCharacter::PlayActB(bool bIsLong)
{
	bIsUpperReacting = false;
	UpperReactTimer  = 0.f;
	TryPlayReactionAnim(ActBAnim, ActBRate, FName(TEXT("DefaultSlot")));
}

void APTBDWCharacter::PlayFail(EPTBActionType Action)
{
	BeginUpperReact(FailAnim, FailAnimRate);
	TryPlayReactionAnim(FailAnim, FailAnimRate, UpperSlotName);
}

void APTBDWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (UpperReactTimer > 0.f)
	{
		UpperReactTimer -= DeltaTime;
		if (UpperReactTimer <= 0.f)
		{
			bIsUpperReacting = false;
		}
	}

	if (AnimReturnTimer > 0.f)
	{
		AnimReturnTimer -= DeltaTime;
		if (AnimReturnTimer <= 0.f && bRunning && RunAnim)
		{
			Body->PlayAnimation(RunAnim, true);
		}
	}
}
