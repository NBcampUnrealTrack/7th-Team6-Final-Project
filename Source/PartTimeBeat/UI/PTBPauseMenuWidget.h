// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PTBModalMenuWidget.h"
#include "PTBPauseMenuWidget.generated.h"

class UButton;
class APTBGameModeBase;

/**
 * 플레이 중 일시정지 상태에서 표시되는 인게임 메뉴 위젯
 *
 * PauseGame() → ShowPauseMenu() → OpenMenu() 경로로 열림
 * ESC 는 CloseMenu() 대신 GameMode->ResumeGame() 을 호출해
 * 게임 상태와 위젯 해제를 함께 처리
 * 버튼 핸들러는 BP에서 Request* 함수를 연결
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBPauseMenuWidget : public UPTBModalMenuWidget
{
	GENERATED_BODY()

public:
	UPTBPauseMenuWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

	/** 게임 재개 — GameMode::ResumeGame() 호출 (위젯은 GameMode 측에서 닫음) */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|PauseMenu")
	void RequestResume();

	/** 현재 라운드 재시작 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|PauseMenu")
	void RequestRetry();

	/** 미니게임 선택 메뉴로 복귀 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|PauseMenu")
	void RequestExitToMenu();

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonResume = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonRetry = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSettings = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonQuit = nullptr;
};