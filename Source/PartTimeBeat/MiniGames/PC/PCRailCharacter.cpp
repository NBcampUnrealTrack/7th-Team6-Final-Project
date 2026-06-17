#include "MiniGames/PC/PCRailCharacter.h"
#include "MiniGames/PC/PCTileSpawner.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"

APCRailCharacter::APCRailCharacter()
{
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    RootComponent = Camera;

    HandsMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HandsMesh"));
    HandsMesh->SetupAttachment(Camera);

    JudgementZone = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("JudgementZone"));
    JudgementZone->SetupAttachment(Camera);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
}

void APCRailCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (PCRailPath)
    {
        FVector StartLocation = PCRailPath->Spline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
        FRotator StartRotation = PCRailPath->Spline->GetRotationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
        StartRotation.Pitch = -10.f;
        StartRotation.Roll = 0.f;

        SetActorLocationAndRotation(StartLocation, StartRotation);
    }
}

void APCRailCharacter::MoveOneStep()
{
    if (!PCRailPath) return;

    CurrentSplineDistance += StepDistance;

    float MaxDist = PCRailPath->Spline->GetSplineLength();
    CurrentSplineDistance = FMath::Clamp(CurrentSplineDistance, 0.f, MaxDist);

    FVector NewLocation = PCRailPath->Spline->GetLocationAtDistanceAlongSpline(
        CurrentSplineDistance, ESplineCoordinateSpace::World);

    FRotator NewRotation = PCRailPath->Spline->GetRotationAtDistanceAlongSpline(
        CurrentSplineDistance, ESplineCoordinateSpace::World);
    NewRotation.Pitch = -10.f;
    NewRotation.Roll = 0.f;

    SetActorLocationAndRotation(NewLocation, NewRotation);
}

void APCRailCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    
    PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &APCRailCharacter::MoveOneStep);
    PlayerInputComponent->BindKey(EKeys::F, IE_Pressed, this, &APCRailCharacter::TestSpawnTile);
}

void APCRailCharacter::TestSpawnTile()
{
    if (TileSpawner)
        TileSpawner->SpawnTile();
}