// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "FSWidget.generated.h"

class UImage;
class APTBFSMiniGame;

UCLASS()
class PARTTIMEBEAT_API UFSWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta = (BindWidget),BlueprintReadOnly)
	UImage* ArrowLeft;

	UPROPERTY(meta = (BindWidget),BlueprintReadOnly)
	UImage* ArrowRight;

	UPROPERTY(meta = (BindWidget),BlueprintReadOnly)
	UImage* ArrowUp;

	UPROPERTY()
	APTBFSMiniGame* FSMiniGame;

	float TargetTimeMs;
	float CueLeadTimeMs;
	bool bIsActive = false;
	EPTBActionType CurrentAction;

public:
	void InitializeWidget(APTBFSMiniGame* InGame);

	UFUNCTION()
	void OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs);
};