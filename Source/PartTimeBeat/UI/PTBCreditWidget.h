#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBCreditWidget.generated.h"

class UScrollBox;
class UButton;

/**
 * 엔딩 크레딧 위젯
 * ScrollBox를 자동으로 스크롤하며, 입력 중 빨리 감기 / Esc로 닫기 지원
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBCreditWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	// 처음부터 다시 재생
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void RestartCredit();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void CloseCredit();

protected:
	UFUNCTION()
	void OnCloseClicked();

protected:
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UScrollBox> ScrollCredit = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonClose = nullptr;

	// 초당 스크롤 픽셀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float ScrollSpeed = 60.f;

	// 빨리 감기 배율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float FastForwardMultiplier = 5.f;

	// 시작 전 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float StartDelay = 1.f;

	// 끝난 뒤 자동으로 닫히기까지 대기 시간 (0 이하면 자동으로 닫지 않음)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float AutoCloseDelay = 3.f;

private:
	float ElapsedDelay = 0.f;
	float EndElapsed = 0.f;
	bool bFastForward = false;
	bool bReachedEnd = false;
};