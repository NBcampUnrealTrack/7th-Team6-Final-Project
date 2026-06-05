// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBPlayerControllerBase.h"
#include "InputActionValue.h"
#include "FSPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
UCLASS()
class PARTTIMEBEAT_API APTBFSPlayerController : public APTBPlayerControllerBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputMappingContext> FishingMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_ActionA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_ActionB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_ActionC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_ActionD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
	TObjectPtr<UInputAction> IA_ActionE;

private:
	void OnActionA(const FInputActionValue& Value);
	void OnActionB(const FInputActionValue& Value);
	void OnActionC(const FInputActionValue& Value);
	void OnActionD(const FInputActionValue& Value);
	void OnActionE(const FInputActionValue& Value);
	
	void OnActionAReleased(const FInputActionValue& Value);
	void OnActionBReleased(const FInputActionValue& Value);
	void OnActionCReleased(const FInputActionValue& Value);
	void OnActionDReleased(const FInputActionValue& Value);
	void OnActionEReleased(const FInputActionValue& Value);
};