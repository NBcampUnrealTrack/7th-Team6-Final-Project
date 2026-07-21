#include "MiniGames/DW/PTBDWCameraRig.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"

APTBDWCameraRig::APTBDWCameraRig()
{
	PrimaryActorTick.bCanEverTick = true;

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

	if (bStartAtIntroPose)
	{
		ApplyPose(IntroArmLength, IntroPitch, IntroYaw, IntroFieldOfView);
	}
	else
	{
		ApplyPose(ArmLength, Pitch, Yaw, FieldOfView);
	}

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

void APTBDWCameraRig::ApplyPose(float InArm, float InPitch, float InYaw, float InFOV)
{
	if (SpringArm)
	{
		SpringArm->TargetArmLength = InArm;
		SpringArm->SetRelativeRotation(FRotator(InPitch, InYaw, 0.f));
	}
	if (Camera)
	{
		Camera->SetFieldOfView(InFOV);
	}
}

void APTBDWCameraRig::StartIntroMove()
{
	bIntroMoving = true;
	IntroElapsed = 0.f;
	ApplyPose(IntroArmLength, IntroPitch, IntroYaw, IntroFieldOfView);
}

void APTBDWCameraRig::SnapToIntroPose()
{
	bIntroMoving = false;
	ApplyPose(IntroArmLength, IntroPitch, IntroYaw, IntroFieldOfView);
}

void APTBDWCameraRig::PlayFailShake(float Intensity, float Duration, float Frequency)
{
	if (Intensity <= 0.f || Duration <= 0.f)
	{
		return;
	}
	ShakeIntensity = Intensity;
	ShakeDuration  = Duration;
	ShakeFrequency = FMath::Max(0.1f, Frequency);
	ShakeTimer     = Duration;
	ShakePhase     = 0.f;
}

void APTBDWCameraRig::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ShakeTimer > 0.f && !bIntroMoving)
	{
		ShakeTimer -= DeltaSeconds;
		ShakePhase += DeltaSeconds * ShakeFrequency;
		const float Elapsed = 1.f - FMath::Clamp(ShakeTimer / FMath::Max(0.01f, ShakeDuration), 0.f, 1.f);
		const float AttackFrac = 0.15f;
		float Env;
		if (Elapsed < AttackFrac)
		{
			Env = FMath::InterpEaseOut(0.f, 1.f, Elapsed / AttackFrac, 2.f);
		}
		else
		{
			const float DecayT = (Elapsed - AttackFrac) / FMath::Max(0.01f, 1.f - AttackFrac);
			Env = FMath::InterpEaseInOut(1.f, 0.f, DecayT, 2.f);
		}
		const float Amp  = ShakeIntensity * Env;
		const float OffP = FMath::Sin(ShakePhase * 2.f * PI) * Amp;
		const float OffY = FMath::Sin(ShakePhase * 2.f * PI * 1.37f + 1.1f) * Amp;
		if (SpringArm)
		{
			SpringArm->SetRelativeRotation(FRotator(Pitch + OffP, Yaw + OffY, 0.f));
		}
		if (ShakeTimer <= 0.f && SpringArm)
		{
			SpringArm->SetRelativeRotation(FRotator(Pitch, Yaw, 0.f));
		}
	}

	if (!bIntroMoving) { return; }

	IntroElapsed += DeltaSeconds;
	const float Dur = FMath::Max(0.01f, IntroMoveDuration);
	const float Alpha = FMath::Clamp(IntroElapsed / Dur, 0.f, 1.f);
	const float E = FMath::InterpEaseInOut(0.f, 1.f, Alpha, FMath::Max(1.f, IntroEaseExp));
	ApplyPose(
		FMath::Lerp(IntroArmLength, ArmLength, E),
		FMath::Lerp(IntroPitch, Pitch, E),
		FMath::Lerp(IntroYaw, Yaw, E),
		FMath::Lerp(IntroFieldOfView, FieldOfView, E));

	if (Alpha >= 1.f)
	{
		ApplyPose(ArmLength, Pitch, Yaw, FieldOfView);
		bIntroMoving = false;
		OnIntroMoveFinished.Broadcast();
	}
}
