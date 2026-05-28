// Fill out your copyright notice in the Description page of Project Settings.


#include "FishActor.h"



AFishActor::AFishActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AFishActor::SetTargetLocation(FVector NewLocation)
{
	TargetLocation = NewLocation;
}

void AFishActor::BeginPlay()
{
	Super::BeginPlay();
	TargetLocation = GetActorLocation();
}

void AFishActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	FVector Current = GetActorLocation();
	FVector NewLocation = FMath::VInterpTo(
		Current,     
		TargetLocation,
		DeltaTime,
		InterpSpeed  
	);
	SetActorLocation(NewLocation);
}

