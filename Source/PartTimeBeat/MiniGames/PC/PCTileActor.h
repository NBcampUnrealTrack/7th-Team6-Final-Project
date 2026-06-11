#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "PCTileActor.generated.h"


UCLASS()
class PARTTIMEBEAT_API APCTileActor : public AActor
{
    GENERATED_BODY()

public:
    APCTileActor();

    // 스폰 후 방향 설정용 (Spawner가 호출)
    void Launch(FVector Direction);

    UPROPERTY(EditAnywhere)
    float MoveSpeed = 1000.f;

    UPROPERTY(VisibleAnywhere)
    class UProjectileMovementComponent* ProjectileMovement;

    // 몇 초 뒤 자동 소멸
    UPROPERTY(EditAnywhere)
    float AutoDestroyDelay = 3.f;

    void AutoDestroy();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle DestroyTimerHandle;
};