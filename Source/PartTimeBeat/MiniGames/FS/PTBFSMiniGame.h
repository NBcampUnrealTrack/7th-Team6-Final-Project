// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBFSMiniGame.generated.h"


class AFishActor;
class UPTBFSMiniGameRuleSet;
class APTBRhythmCharacterBase;

UENUM(BlueprintType)
enum class EFishingLineState : uint8
{
	Loose,
	Taut,
	Maximum,
};

UCLASS()
class PARTTIMEBEAT_API APTBFSMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
 
public:
	APTBFSMiniGame();
 
	void BuildRuntimeState();
	
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionAInput(); 
 
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionBInput(); 
 
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionCInput(); 
 
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionDInput(); 
	
	UFUNCTION(BlueprintCallable, Category = "Fishing|Input")
	void HandleActionEInput();
protected:
	void CacheFishingRuleSet();
private:
	UPROPERTY()
	TObjectPtr<AFishActor> FishActor;
	
	UPROPERTY()
	TObjectPtr<UPTBFSMiniGameRuleSet> FishRuleSet;
	
	int32 NoteCount;
};
