// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBPauseMenuWidget.generated.h"

class UButton;

/**
 * 플레이 중 일시정지 상태에서 표시되는 인게임 메뉴 위젯
 * 재개/재시도/설정/종료 등 플레이 흐름 분기 액션을 제공
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonResume = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonRetry = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSettings = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonQuit = nullptr;
};
