// Fill out your copyright notice in the Description page of Project Settings.


#include "FSMoveActor.h"


// Sets default values
AFSMoveActor::AFSMoveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;
}

void AFSMoveActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    
	ElapsedTime += DeltaTime;
	float NewScale = BaseScale + Amplitude * FMath::Sin(ElapsedTime * Speed);
	SetActorScale3D(FVector(NewScale));	
}

void AFSMoveActor::BeginPlay()
{
	Super::BeginPlay();
	BaseScale = GetActorScale3D().X;
	Amplitude = BaseScale/2;
}


