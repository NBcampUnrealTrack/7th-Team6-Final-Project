// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSCharacter.h"
#include "CableComponent.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
APTBFSCharacter::APTBFSCharacter()
{

	PrimaryActorTick.bCanEverTick = false;
	
	FRotator FishingRodMeshRotator= FRotator(162.f,434.f,517.f);
	FVector FishingRodMeshLocation = FVector(-4.3f,-2.5f,-2.2f);
	FVector FishingRodMeshScale = FVector(0.166667f, 0.166667f, 0.166667f);
	
	FishingRodMesh = CreateDefaultSubobject<UStaticMeshComponent>("FishingRodMesh");
	FishingRodMesh->SetupAttachment(GetMesh(),FName(FName("FishingRodSocket")));
	FishingRodMesh->SetRelativeRotation(FishingRodMeshRotator);
	FishingRodMesh->SetRelativeLocation(FishingRodMeshLocation);
	FishingRodMesh->SetRelativeScale3D(FishingRodMeshScale);
	
	FishingCable = CreateDefaultSubobject<UCableComponent>(TEXT("FishingCable"));
	FishingCable->SetupAttachment(FishingRodMesh,FName("FishLine"));
}

void APTBFSCharacter::OnPlayCastAnimMontage()
{	
	if (CastAnimMontage)
	{
		PlayAnimMontage(CastAnimMontage);
	}
}

void APTBFSCharacter::OnPlayRealAnimMontage()
{
	if (RealAnimMontage)
	{
		PlayAnimMontage(RealAnimMontage);
	}
}



