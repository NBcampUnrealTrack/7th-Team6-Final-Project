// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBFSMiniGame.generated.h"

UCLASS()
class PARTTIMEBEAT_API APTBFSMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APTBFSMiniGame();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
