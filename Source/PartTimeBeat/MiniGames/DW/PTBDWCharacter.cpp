#include "MiniGames/DW/PTBDWCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequenceBase.h"
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

void APTBDWCharacter::SetAnimations(UAnimSequenceBase* InRun, UAnimSequenceBase* InJump)
{
	RunAnim = InRun;
	JumpAnim = InJump;
}

void APTBDWCharacter::StartRunning()
{
	bRunning = true;
	JumpReturnTimer = 0.f;
	if (RunAnim)
	{
		Body->PlayAnimation(RunAnim, true);
	}
}

void APTBDWCharacter::PlayCue(EPTBActionType Action)
{
	// TODO: 개 예고 애니 준비되면 여기서 짧게 재생.
}

void APTBDWCharacter::PlayReaction(bool bSuccess, EPTBActionType Action)
{
	if (bSuccess && Action == EPTBActionType::ActionA)
	{
		PlayJump();
	}
	// TODO: ActionB 슬라이드 / 실패 모션은 애니 준비되면 추가.
}

void APTBDWCharacter::PlayJump()
{
	if (!JumpAnim)
	{
		return;
	}
	Body->PlayAnimation(JumpAnim, false);
	JumpReturnTimer = JumpAnim->GetPlayLength();
}

void APTBDWCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (JumpReturnTimer > 0.f)
	{
		JumpReturnTimer -= DeltaTime;
		if (JumpReturnTimer <= 0.f && bRunning && RunAnim)
		{
			Body->PlayAnimation(RunAnim, true);
		}
	}
}
