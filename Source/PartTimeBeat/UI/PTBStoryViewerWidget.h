// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBStoryViewerWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

/**
 * 스토리 대사와 캐릭터 비주얼을 순차적으로 표시하는 스토리 뷰어 위젯
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBStoryViewerWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextDialogue = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageCharacter = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSkip = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;
};
