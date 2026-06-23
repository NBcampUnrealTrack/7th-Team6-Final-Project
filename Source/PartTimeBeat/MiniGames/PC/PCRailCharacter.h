#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MiniGames/PC/PCRailPath.h"
#include "PCRailCharacter.generated.h"


class APCTileSpawner;

UCLASS()
class PARTTIMEBEAT_API APCRailCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APCRailCharacter();

    // 카메라
    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* Camera;

    // 손 메시 붙일 컴포넌트
    UPROPERTY(VisibleAnywhere)
    class USkeletalMeshComponent* HandsMesh;

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* JudgementZone;

    // 레일 참조 (에디터에서 연결)
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    APCRailPath* PCRailPath;

    // 한 박자당 이동 거리
    UPROPERTY(EditAnywhere)
    float StepDistance = 100.f;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    float MoveInterpSpeed = 20.f;

    // 박자 타이밍에 호출 - 레일 위 다음 지점으로 이동
    void MoveOneStep();

    void TestSpawnTile();

    // 스포너 참조
    UPROPERTY(EditAnywhere)
    APCTileSpawner* TileSpawner;

    FTimerHandle MoveDelayHandle;


protected:
    virtual void BeginPlay() override;

private:

    FVector TargetLocation;
    FRotator TargetRotation;
    bool bIsMoving = false;

    // 현재 레일 위 거리값
    UPROPERTY(EditAnywhere)
    float CurrentSplineDistance = 0.f;
};
