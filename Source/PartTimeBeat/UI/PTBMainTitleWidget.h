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
	virtual void NativeDestruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();
	UFUNCTION()
	void OnStartClicked();
	UFUNCTION()
	void OnSettingsClicked();
	UFUNCTION()
	void OnQuitClicked();
	UFUNCTION()
	void OnAchievementClicked();

	// 로고 화면에서 타이틀 화면으로 전환
	UFUNCTION()
	void TransitionToTitleScreen();

protected:
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonStart = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSettings = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonQuit = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonAchievement = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageLogo = nullptr;


	// 전환할 타이틀 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "PTB|UI")
	TSubclassOf<UUserWidget> TitleScreenWidgetClass;

	// 자동 전환 타이머
	FTimerHandle TransitionTimerHandle;
};
