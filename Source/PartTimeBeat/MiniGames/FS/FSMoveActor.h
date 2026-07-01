// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FSMoveActor.generated.h"

UCLASS()
class PARTTIMEBEAT_API AFSMoveActor : public AActor
{
	GENERATED_BODY()


protected:
	AFSMoveActor();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Animation")
	float BaseScale =1;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Amplitude =0.1;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float Speed = 6.28f;

	float ElapsedTime = 0.0f;

	float LastScale = 0.0f;
	
	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
