// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGames/Common/PTBBaseMiniGame.h"

// Sets default values
APTBBaseMiniGame::APTBBaseMiniGame()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APTBBaseMiniGame::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APTBBaseMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

