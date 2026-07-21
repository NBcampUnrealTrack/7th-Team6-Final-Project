// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTBDialogHostWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBResultWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPTBResultWidgetEvent);

/**
 * 미니게임 종료 후 결과 배경 이미지를 표시하는 베이스 위젯
 * ShowResult() 호출 시 배경 갱신 + PopupDelay 후 팝업 자동 표시
 * 팝업 관리는 PTBDialogHostWidget(부모)이 담당
 * 재도전/맵으로 이동은 외부에 델리게이트로 전달
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBResultWidget : public UPTBDialogHostWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void ShowResult(const FPTBRoundResult& InResult, const FPTBRewardSummary& InReward);

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	bool ShowResultFromFlowSubsystem();

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnRetryRequested;

	UPROPERTY(BlueprintAssignable, BlueprintCallable, Category = "PTB|UI|Event")
	FPTBResultWidgetEvent OnMapRequested;

	UFUNCTION(BlueprintPure, Category = "PTB|UI")
	const FPTBRoundResult& GetRoundResult() const { return CurrentRoundResult; }

	UFUNCTION(BlueprintPure, Category = "PTB|UI")
	const FPTBRewardSummary& GetRewardSummary() const { return CurrentRewardSummary; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	float PopupDelay = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI")
	bool bAutoShowFlowResultOnConstruct = true;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** EPTBGradeType 에 따라 배경 이미지 교체 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI")
	void OnDisplayTypeChanged(EPTBGradeType Grade);

	/** PopupDelay 초 후 호출 — BP에서 WBP_ResultPopup 생성 후 PushDialog */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI")
	void OnPopupTimerFired();

	UPROPERTY(BlueprintReadOnly, Category = "PTB|UI")
	FPTBRoundResult CurrentRoundResult;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|UI")
	FPTBRewardSummary CurrentRewardSummary;

private:
	FTimerHandle PopupTimerHandle;
	void FirePopupTimer();
};
