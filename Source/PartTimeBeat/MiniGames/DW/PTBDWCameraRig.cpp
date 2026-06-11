#include "MiniGames/DW/PTBDWCameraRig.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

APTBDWCameraRig::APTBDWCameraRig()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Root);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
}

void APTBDWCameraRig::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (SpringArm)
	{
		SpringArm->TargetArmLength = ArmLength;
		SpringArm->SetRelativeRotation(FRotator(Pitch, Yaw, 0.f));
	}
	if (Camera)
	{
		Camera->SetFieldOfView(FieldOfView);
	}
}

void APTBDWCameraRig::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoPossess)
	{
		if (UWorld* World = GetWorld())
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				PC->SetViewTargetWithBlend(this, BlendTime);
			}
		}
	}
}
