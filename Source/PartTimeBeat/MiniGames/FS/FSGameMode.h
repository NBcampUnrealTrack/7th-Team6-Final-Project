// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBGameModeBase.h"
#include "FSGamMode.generated.h"

/**
 * 
 */
class APTBFSMiniGame;
class AFishActor;


UCLASS()
class PARTTIMEBEAT_API AFSGameMode : public APTBGameModeBase
{
	GENERATED_BODY()
	
public:
	AFSGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FS|Spawn")
	TSubclassOf<APTBFSMiniGame> MiniGameClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FS|Spawn")
	TSubclassOf<AFishActor> FishActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FS|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

private:
	UPROPERTY(BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	TObjectPtr<APTBFSMiniGame> FishingMiniGame;

	UPROPERTY()
	TObjectPtr<AFishActor> SpawnedFishActor;

	UPROPERTY()
	TObjectPtr<UUserWidget> HUDWidget;

	UFUNCTION()
	void HandleFishingMiniGameFinished(FPTBRoundResult Result);
};
