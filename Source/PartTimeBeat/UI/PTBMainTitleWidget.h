// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBMainTitleWidget.generated.h"

class UButton;
class UImage;

/**
 * 게임 진입 시 가장 먼저 표시되는 메인 타이틀 화면 위젯
 * 시작/설정/종료로의 1차 네비게이션과 로고 표출을 담당
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBMainTitleWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonStart = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSettings = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonQuit = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageLogo = nullptr;
};
