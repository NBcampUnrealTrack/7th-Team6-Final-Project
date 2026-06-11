// Fill out your copyright notice in the Description page of Project Settings.


#include "FishActor.h"

#include "Debug/PTBTeamLog.h"


AFishActor::AFishActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SetRootComponent(SkeletalMesh);
	if (SkeletalMesh)
	{
		SkeletalMesh->SetRenderCustomDepth(true);
		SkeletalMesh->SetCustomDepthStencilValue(1);
	}
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
	float MoveDist =  FVector::Dist(Current, NewLocation);
	if (MoveDist > 1.0f)
	{
		if (SwimMontage && !SkeletalMesh->GetAnimInstance()->Montage_IsPlaying(SwimMontage))
			SkeletalMesh->GetAnimInstance()->Montage_Play(SwimMontage);
	}
	else
	{
		if (IdleMontage && !SkeletalMesh->GetAnimInstance()->Montage_IsPlaying(IdleMontage))
			SkeletalMesh->GetAnimInstance()->Montage_Play(IdleMontage);
	}
	SetActorLocation(NewLocation);
}

