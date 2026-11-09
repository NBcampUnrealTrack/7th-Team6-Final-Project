#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBConfirmDialogWidget.generated.h"

class UButton;
class UPTBDialogHostWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnConfirmDialogResult);

/**
 * 예/아니오 확인 창 공용 베이스 (WBP_DeleteProfileDialog 등의 부모).
 * WBP에 Btn_Yes, Btn_No 이름의 버튼을 두면 자동 연결.
 * 버튼을 누르면 창을 먼저 닫고 OnConfirmed / OnCancelled 를 보냄.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBConfirmDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Dialog|Confirm")
	FOnConfirmDialogResult OnConfirmed;

	UPROPERTY(BlueprintAssignable, Category = "Dialog|Confirm")
	FOnConfirmDialogResult OnCancelled;

	UFUNCTION(BlueprintCallable, Category = "Dialog|Confirm")
	void Confirm();

	UFUNCTION(BlueprintCallable, Category = "Dialog|Confirm")
	void Cancel();

	/** 이 창 닫기 (Dialog Host에 있으면 CloseTopDialog, 아니면 RemoveFromParent) */
	UFUNCTION(BlueprintCallable, Category = "Dialog|Confirm")
	void CloseSelf();

	/** 위젯이 들어 있는 Dialog Host 찾기 (없으면 nullptr) */
	static UPTBDialogHostWidget* FindDialogHostOf(const UUserWidget* Widget);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Yes;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_No;
};
