// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/PTBRhythmCharacterBase.h"
#include "PTBFSCharacter.generated.h"

class UCableComponent;
class APTBFSMiniGame;
UCLASS()
class PARTTIMEBEAT_API APTBFSCharacter : public APTBRhythmCharacterBase
{
	GENERATED_BODY()

public:
	APTBFSCharacter();
	
	void OnPlayCastAnimMontage();
	void OnPlayRealAnimMontage();
	
	UFUNCTION()
	void SetFishLineTarget(AActor* InFishActor);
	
	void AttachFishingLine();
	
	UFUNCTION()
	void OnFishingLineStateChanged(EFishingLineState NewState);
	
	UPROPERTY(EditAnywhere, Category = "Fishing")
	FName FishSocketName = FName("TestSocket");
protected:
	UPROPERTY(VisibleAnywhere, Category = "Fishing")
	TObjectPtr<UStaticMeshComponent> FishingRodMesh;

	UPROPERTY(VisibleAnywhere, Category = "Fishing")
	TObjectPtr<UCableComponent> FishingCable;
	
	UPROPERTY()
	TObjectPtr<AActor> FishLineTarget;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FSAnim",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAnimMontage> RealAnimMontage = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|FSAnim",meta=(AllowPrivateAccess=true))
	TObjectPtr<UAnimMontage> CastAnimMontage = nullptr;
};
