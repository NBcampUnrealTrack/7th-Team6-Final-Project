// Fill out your copyright notice in the Description page of Project Settings.


#include "FSGameMode.h"
#include "FishActor.h"
#include "Blueprint/UserWidget.h"
#include "PTBFSMiniGame.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"

AFSGameMode::AFSGameMode()
{
	SpawnedFishActor = nullptr;
	FishingMiniGame = nullptr;
	HUDWidget = nullptr;
}

void AFSGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	if (!FishActorClass)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("FishActorClass 미설정"));
		return;
	}
	if (!MiniGameClass)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("MiniGameClass 미설정"));
		return;
	}
	
	FVector FishActorLocation = FVector(0,0,-100);
	FRotator FishActorRotation = FRotator(0,90,0);
	SpawnedFishActor = GetWorld()->SpawnActor<AFishActor>(FishActorClass, FishActorLocation, FishActorRotation);
	
	FActorSpawnParameters Params;
	Params.bDeferConstruction = true;
	FishingMiniGame = GetWorld()->SpawnActor<APTBFSMiniGame>(MiniGameClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
	
	
	if (FishingMiniGame)
	{
		FishingMiniGame->SetFishActor(SpawnedFishActor); 
		FishingMiniGame->FinishSpawning(FTransform::Identity);  

		FishingMiniGame->OnMiniGameFinished.AddDynamic(
			this, &AFSGameMode::HandleFishingMiniGameFinished);
		FishingMiniGame->OnStartFishingGame();
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
