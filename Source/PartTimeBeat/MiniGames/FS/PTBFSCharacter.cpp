// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSCharacter.h"
#include "CableComponent.h"
#include "PTBFSMiniGame.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
APTBFSCharacter::APTBFSCharacter()
{

	PrimaryActorTick.bCanEverTick = false;
	
	FRotator FishingRodMeshRotator= FRotator(162.f,434.f,517.f);
	FVector FishingRodMeshLocation = FVector(-4.3f,-2.5f,-2.2f);
	FVector FishingRodMeshScale = FVector(0.166667f, 0.166667f, 0.166667f);
	
	FishingRodMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FishingRodMesh"));
	FishingRodMesh->SetupAttachment(GetMesh(),FName(TEXT("FishingRodSocket")));
	FishingRodMesh->SetRelativeRotation(FishingRodMeshRotator);
	FishingRodMesh->SetRelativeLocation(FishingRodMeshLocation);
	FishingRodMesh->SetRelativeScale3D(FishingRodMeshScale);
	
	FishingCable = CreateDefaultSubobject<UCableComponent>(TEXT("FishingCable"));
	FishingCable->SetupAttachment(FishingRodMesh,FName(TEXT("FishLine")));
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

	void APTBFSCharacter::UpdateFishingLineLength(float FishDistance)
	{
		if (!FishingCable) return;
	FishingCable->CableLength = 50.0f + (2500.0f * FishDistance);
	FishingCable->CableGravityScale = 0.0f;
	FishingCable->MarkRenderStateDirty();
	}


void APTBFSCharacter::SetFishLineTarget(AActor* InFishActor)
{
	FishLineTarget = InFishActor;
}

void APTBFSCharacter::AttachFishingLine()
{
	if (FishingCable && FishLineTarget)
	{
		UMeshComponent* FishMesh = FishLineTarget->FindComponentByClass<UMeshComponent>();
		if (FishMesh)
		{
			FishingCable->bAttachEnd=true;
			FishingCable->SetAttachEndToComponent(FishMesh,FishSocketName);
			FishingCable->EndLocation = FVector::ZeroVector; 
		}
	}
}
