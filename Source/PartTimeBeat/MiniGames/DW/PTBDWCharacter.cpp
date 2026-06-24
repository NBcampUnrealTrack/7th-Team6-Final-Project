#include "MiniGames/DW/PTBDWCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequenceBase.h"
#include "Animation/AnimInstance.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"


//에셋 구하기전 임시 placeholder
namespace DWCueConstants
{
	constexpr float HopHeight = 40.0f;
	constexpr float DuckDepth = 25.0f;
}

namespace DWReactionConstants
{
	constexpr float JumpHeight     = 120.0f;
	constexpr float JumpHeightLong = 160.0f;
	constexpr float SlideDepth     = 55.0f;
	constexpr float FailDip        = 18.0f;
	constexpr float FailHop        = 35.0f;

	constexpr float JumpDuration   = 0.40f;
	constexpr float SlideDuration  = 0.35f;
	constexpr float SlideDurLong   = 0.55f;
	constexpr float FailDuration   = 0.25f;
}

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

void APTBDWCharacter::SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InJump, UAnimSequenceBase* InSlide, UAnimSequenceBase* InFail, TSubclassOf<UAnimInstance> InAnimClass)
{
	RunAnim = InRun;
	JumpAnim = InJump;
	SlideAnim = InSlide;
	FailAnim = InFail;

	bMontageMode = (InAnimClass != nullptr);
	if (bMontageMode && Body)
	{
		Body->SetAnimInstanceClass(InAnimClass);
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

void APTBDWCharacter::PlayCue(EPTBActionType Action, float DurationSec)
{
	if (DurationSec <= 0.f)
	{
		return;
	}

	bCueActive  = true;
	CueAction   = Action;
	CueDuration = DurationSec;
	CueTimer    = DurationSec;
}

void APTBDWCharacter::UpdateCue(float DeltaTime)
{
	if (!bCueActive || !Body)
	{
		return;
	}

	CueTimer -= DeltaTime;

	const float Progress = FMath::Clamp(1.0f - CueTimer / FMath::Max(KINDA_SMALL_NUMBER, CueDuration), 0.f, 1.f);
	const float Arc = FMath::Sin(PI * Progress);

	float DeltaZ = 0.f;
	switch (CueAction)
	{
	case EPTBActionType::ActionA: DeltaZ =  DWCueConstants::HopHeight  * Arc; break;
	case EPTBActionType::ActionB: DeltaZ = -DWCueConstants::DuckDepth  * Arc; break;
	default: break;
	}

	FVector Loc = Body->GetRelativeLocation();
	Loc.Z = DeltaZ;
	Body->SetRelativeLocation(Loc);

	if (CueTimer <= 0.f)
	{
		bCueActive = false;
		Loc.Z = 0.f;
		Body->SetRelativeLocation(Loc);
	}
}

void APTBDWCharacter::PlayReaction(bool bSuccess, EPTBActionType Action, bool bIsLong)
{
	if (bSuccess)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: PlayJump(bIsLong);  break;
		case EPTBActionType::ActionB: PlaySlide(bIsLong); break;
		default: break;
		}
		return;
	}

	PlayFail(Action);
}

bool APTBDWCharacter::TryPlayReactionAnim(UAnimSequenceBase* Anim)
{
	if (!Anim || !Body)
	{
		return false;
	}
	bReactionActive = false;
	FVector Loc = Body->GetRelativeLocation();
	Loc.Z = 0.f;
	Body->SetRelativeLocation(Loc);

	if (bMontageMode)
	{
		if (UAnimInstance* AnimInst = Body->GetAnimInstance())
		{
			AnimInst->PlaySlotAnimationAsDynamicMontage(Anim, TEXT("DefaultSlot"), 0.15f, 0.15f);
			return true;
		}
	}

	Body->PlayAnimation(Anim, false);
	AnimReturnTimer = Anim->GetPlayLength();
	return true;
}

void APTBDWCharacter::StartPlaceholderReaction(EDWReactionMotion Motion, float DurationSec, float Amplitude)
{
	bReactionActive  = true;
	ReactionMotion   = Motion;
	ReactionDuration = DurationSec;
	ReactionTimer    = DurationSec;
	ReactionAmp      = Amplitude;
}

void APTBDWCharacter::PlayJump(bool bIsLong)
{
	if (TryPlayReactionAnim(JumpAnim))
	{
		return;
	}
	const float Amp = bIsLong ? DWReactionConstants::JumpHeightLong : DWReactionConstants::JumpHeight;
	StartPlaceholderReaction(EDWReactionMotion::JumpUp, DWReactionConstants::JumpDuration, Amp);
}

void APTBDWCharacter::PlaySlide(bool bIsLong)
{
	if (TryPlayReactionAnim(SlideAnim))
	{
		return;
	}
	const float Dur = bIsLong ? DWReactionConstants::SlideDurLong : DWReactionConstants::SlideDuration;
	StartPlaceholderReaction(EDWReactionMotion::SlideDown, Dur, DWReactionConstants::SlideDepth);
}

void APTBDWCharacter::PlayFail(EPTBActionType Action)
{
	if (TryPlayReactionAnim(FailAnim))
	{
		return;
	}
	if (Action == EPTBActionType::ActionB)
	{
		StartPlaceholderReaction(EDWReactionMotion::FailHop, DWReactionConstants::FailDuration, DWReactionConstants::FailHop);
	}
	else
	{
		StartPlaceholderReaction(EDWReactionMotion::FailDip, DWReactionConstants::FailDuration, DWReactionConstants::FailDip);
	}
}

void APTBDWCharacter::UpdateReaction(float DeltaTime)
{
	if (!bReactionActive || !Body)
	{
		return;
	}

	ReactionTimer -= DeltaTime;
	const float Progress = FMath::Clamp(1.0f - ReactionTimer / FMath::Max(KINDA_SMALL_NUMBER, ReactionDuration), 0.f, 1.f);
	const float Arc = FMath::Sin(PI * Progress);

	float DeltaZ = 0.f;
	switch (ReactionMotion)
	{
	case EDWReactionMotion::JumpUp:    DeltaZ =  ReactionAmp * Arc; break;
	case EDWReactionMotion::SlideDown: DeltaZ = -ReactionAmp * Arc; break;
	case EDWReactionMotion::FailHop:   DeltaZ =  ReactionAmp * Arc; break;
	case EDWReactionMotion::FailDip:   DeltaZ = -ReactionAmp * Arc; break;
	default: break;
	}

	FVector Loc = Body->GetRelativeLocation();
	Loc.Z = DeltaZ;
	Body->SetRelativeLocation(Loc);

	if (ReactionTimer <= 0.f)
	{
		bReactionActive = false;
		ReactionMotion = EDWReactionMotion::None;
		Loc.Z = 0.f;
		Body->SetRelativeLocation(Loc);
	}
}

void APTBDWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateCue(DeltaTime);
	UpdateReaction(DeltaTime);

	if (AnimReturnTimer > 0.f)
	{
		AnimReturnTimer -= DeltaTime;
		if (AnimReturnTimer <= 0.f && bRunning && RunAnim)
		{
			Body->PlayAnimation(RunAnim, true);
		}
	}
}
