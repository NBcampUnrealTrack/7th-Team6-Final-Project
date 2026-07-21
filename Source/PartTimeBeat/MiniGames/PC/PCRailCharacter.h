#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MiniGames/PC/PCRailPath.h"
#include "Core\PTBStructEnums.h"
#include "PCRailCharacter.generated.h"


class APCTileSpawner;

UCLASS()
class PARTTIMEBEAT_API APCRailCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APCRailCharacter();

    UPROPERTY(VisibleAnywhere)
    class UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    class USkeletalMeshComponent* HandsMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Anim")
    UAnimMontage* HandSuccessMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Anim")
    UAnimMontage* HandFailMontage;

    void PlayHandMontage(UAnimMontage* Montage);

    UPROPERTY(VisibleAnywhere)
    class UStaticMeshComponent* JudgementZone;


    UPROPERTY(EditAnywhere)
    float StepDistance = 100.f;

    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere)
    float MoveInterpSpeed = 50.f;

 
    void MoveOneStep();

    APCRailPath* PCRailPath;

    UPROPERTY(EditAnywhere)
    APCRailPath* EasyRailPath;

    UPROPERTY(EditAnywhere)
    APCRailPath* StandardRailPath;

    UPROPERTY(EditAnywhere)
    APCRailPath* InsaneRailPath;

    UFUNCTION(BlueprintCallable)
    void SelectRailByDifficulty(EPTBDifficulty Difficulty);

    UFUNCTION(BlueprintCallable)
    void UpdateGridVisibility();
 
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
