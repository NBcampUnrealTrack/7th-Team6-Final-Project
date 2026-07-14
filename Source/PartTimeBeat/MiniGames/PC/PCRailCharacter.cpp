#include "MiniGames/PC/PCRailCharacter.h"
#include "MiniGames/PC/PCTileSpawner.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SplineComponent.h"

APCRailCharacter::APCRailCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

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

}

void APCRailCharacter::PlayHandMontage(UAnimMontage* Montage)
{
    if (!HandsMesh || !Montage) return;

    if (UAnimInstance* AnimInst = HandsMesh->GetAnimInstance())
    {
        AnimInst->Montage_Play(Montage);
    }
}

void APCRailCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsMoving) return;

    FVector CurrentLocation = GetActorLocation();
    FRotator CurrentRotation = GetActorRotation();

    FVector NewLocation = FMath::VInterpTo(CurrentLocation, TargetLocation, DeltaTime, MoveInterpSpeed);
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, MoveInterpSpeed);

    SetActorLocationAndRotation(NewLocation, NewRotation);

 
    if (FVector::Dist(NewLocation, TargetLocation) < 1.f)
    {
        SetActorLocationAndRotation(TargetLocation, TargetRotation);
        bIsMoving = false;
    }
}

void APCRailCharacter::MoveOneStep()
{
    if (!PCRailPath) return;

    CurrentSplineDistance += StepDistance;

    float MaxDist = PCRailPath->Spline->GetSplineLength();
    CurrentSplineDistance = FMath::Clamp(CurrentSplineDistance, 0.f, MaxDist);


    TargetLocation = PCRailPath->Spline->GetLocationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
    TargetRotation = PCRailPath->Spline->GetRotationAtDistanceAlongSpline(CurrentSplineDistance, ESplineCoordinateSpace::World);
    TargetRotation.Pitch = -10.f;
    TargetRotation.Roll = 0.f;

    bIsMoving = true;
}

void APCRailCharacter::SelectRailByDifficulty(EPTBDifficulty Difficulty)
{

    switch (Difficulty)
    {
    case EPTBDifficulty::Easy:
        PCRailPath = EasyRailPath;
        break;
    case EPTBDifficulty::Insane:
        PCRailPath = InsaneRailPath;
        break;
    default:
        PCRailPath = StandardRailPath;
        break;
    }

    CurrentSplineDistance = 0.f;

    // 선택된 레일 시작점으로 순간이동
    if (PCRailPath)
    {
        FVector StartLoc = PCRailPath->Spline->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
        FRotator StartRot = PCRailPath->Spline->GetRotationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World);
        StartRot.Pitch = 0.f;
        StartRot.Roll = 0.f;
        SetActorLocationAndRotation(StartLoc, StartRot);
    }
}
