// Fill out your copyright notice in the Description page of Project Settings.


#include "FSGamMode.h"
#include "FishActor.h"
#include "FSGamMode.h"
#include "PTBFSMiniGame.h"

AFSGameMode::AFSGameMode()
{
	SpawnedFishActor = nullptr;
	FishingMiniGame = nullptr;
	HUDWidget = nullptr;
}

void AFSGameMode::BeginPlay()
{
	Super::BeginPlay();
	FVector FishActorLocation = FVector::ZeroVector;
	FRotator FishActorRotation = FRotator::ZeroRotator;
	SpawnedFishActor = GetWorld()->SpawnActor<AFishActor>(FishActorClass, FishActorLocation, FishActorRotation);
	
	FVector FSGameActorLocation = FVector::ZeroVector;
	FRotator FSGameActorRotation = FRotator::ZeroRotator;
	FishingMiniGame = GetWorld()->SpawnActor<APTBFSMiniGame>(MiniGameClass,FSGameActorLocation,FSGameActorRotation);
	
	if (FishingMiniGame)
	{
		FishingMiniGame->OnMiniGameFinished.AddDynamic(
			this, &AFSGameMode::HandleFishingMiniGameFinished);
	}
	
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();
		}
	}
	
}

void AFSGameMode::HandleFishingMiniGameFinished(FPTBRoundResult Result)
{
	if (SpawnedFishActor)
	{
		SpawnedFishActor->Destroy();
		SpawnedFishActor = nullptr;
	}
	
	if (FishingMiniGame)
	{
		FishingMiniGame->Destroy();
		FishingMiniGame = nullptr;
	}
	
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}
}
