// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBResultWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBResultWidgetEvent);

/**
 * 스테이지 종료 후 결과와 보상을 표시하는 위젯
 * 결과/보상 표시만 담당
 * 재시도/다음 진행은 외부에 요청만 전달
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBResultWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnRetryRequested;

	UPROPERTY(BlueprintAssignable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnNextRequested;
	
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void SetRoundResult(const FPTBRoundResult& RoundResult);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void ShowRewardSummary(const FPTBRewardSummary& RewardSummary);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void OnRetryClicked();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void OnNextClicked();

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(BlueprintReadOnly, Category = "PTB|UI")
	FPTBRoundResult CurrentRoundResult;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> RankText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> ScoreText = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UPanelWidget> RewardPanel = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonRetry = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;
};
