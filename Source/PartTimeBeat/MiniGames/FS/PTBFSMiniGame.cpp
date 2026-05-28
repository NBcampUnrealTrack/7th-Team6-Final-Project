// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBFSMiniGame.h"

#include "FishActor.h"


// Sets default values
APTBFSMiniGame::APTBFSMiniGame()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APTBFSMiniGame::BeginPlay()
{
	Super::BeginPlay();
	
}

void APTBFSMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	// 물고기 위치 가져오기
	//
}

// Called every frame
void APTBFSMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

