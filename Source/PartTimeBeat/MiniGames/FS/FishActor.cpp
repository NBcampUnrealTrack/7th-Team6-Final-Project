// Fill out your copyright notice in the Description page of Project Settings.


#include "FishActor.h"
#include "FFSSkFish.h"
	
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
	if (FishDataTable)
	{
		TArray<FFFSSkFish*> Rows;
		FishDataTable->GetAllRows<FFFSSkFish>(TEXT("FishActor"), Rows);
    
		float TotalWeight = 0.0f;
		for (const FFFSSkFish* Row : Rows)
			TotalWeight += Row->SpawnWeight;
    
		float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
		float AccumulatedWeight = 0.0f;
		const FFFSSkFish* SelectedFish = nullptr;
    
		for (const FFFSSkFish* Row : Rows)
		{
			AccumulatedWeight += Row->SpawnWeight;
			if (RandomValue <= AccumulatedWeight)
			{
				SelectedFish = Row;
				break;
			}
		}
    
		if (SelectedFish)
		{
			if (SelectedFish->FishMesh)
				SkeletalMesh->SetSkeletalMesh(SelectedFish->FishMesh);
			SwimMontage = SelectedFish->SwimMontage;
			IdleMontage = SelectedFish->IdleMontage;
		}
	}
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
	float MoveDist = FVector::Dist(Current, NewLocation);
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
