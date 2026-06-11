
#include "MiniGames/PC/PCTileActor.h"
#include "GameFramework/ProjectileMovementComponent.h"

APCTileActor::APCTileActor()
{
    PrimaryActorTick.bCanEverTick = true;

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 0.f;
    ProjectileMovement->bAutoActivate = false;

}

void APCTileActor::BeginPlay()
{
    Super::BeginPlay();
}

void APCTileActor::Launch(FVector Direction)
{
    ProjectileMovement->Activate();
    ProjectileMovement->Velocity = Direction.GetSafeNormal() * MoveSpeed;

    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &APCTileActor::AutoDestroy,
        AutoDestroyDelay,
        false
    );
}

void APCTileActor::AutoDestroy()
{
    Destroy();
}
