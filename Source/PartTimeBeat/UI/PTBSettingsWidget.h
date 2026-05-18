// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTBHUDBase.h"
#include "PTBSettingsWidget.generated.h"

class UButton;
class USlider;

/**
 * 옵션 화면 위젯
 * 사용자 입력값을 임시 보관하고, Apply 시점에 실제 설정 반영하는 흐름을 가정
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBSettingsWidget : public UPTBHUDBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

protected:
	/** 전체 출력 볼륨 조절 슬라이더 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<USlider> SliderMaster = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<USlider> SliderBgm = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<USlider> SliderSfx = nullptr;

	/**
	 * 판정 타이밍 보정값(오디오/입력 지연)을 조절하는 슬라이더
	 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<USlider> SliderLatency = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonCalibrate = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonApply = nullptr;
};
