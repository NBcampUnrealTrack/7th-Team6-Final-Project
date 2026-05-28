// Fill out your copyright notice in the Description page of Project Settings.


#include "FishActor.h"

#include "Debug/PTBTeamLog.h"


AFishActor::AFishActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SetRootComponent(SkeletalMesh);
}

void AFishActor::SetTargetLocation(FVector NewLocation)
{
	TargetLocation = NewLocation;
}

void AFishActor::BeginPlay()
{
	Super::BeginPlay();
	
	TargetLocation = GetActorLocation();
	PTB_WARNING(LogPTBMiniGames,TEXT("미니게임 엑터 호출완료"));
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

