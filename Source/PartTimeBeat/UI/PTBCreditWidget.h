#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBCreditWidget.generated.h"

class UScrollBox;
class UButton;

/**
 * 엔딩 크레딧 위젯
 * - ScrollBox를 자동으로 천천히 스크롤
 * - 마우스 휠 / 좌클릭 드래그로 직접 스크롤 (직접 조작하면 자동 스크롤 일시정지)
 * - 휠 클릭(가운데 버튼)으로 자동 스크롤 재개
 * - Space/Enter 누르는 동안 빨리 감기, Esc로 닫기
 * - 어떤 방식이든 맨 위/맨 아래를 넘어가지 않음
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
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent) override;

public:
	// 처음부터 다시 재생
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void RestartCredit();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void CloseCredit();

	// 자동 스크롤 재개 (휠 클릭과 같은 동작)
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	void ResumeAutoScroll();

	// 자동 스크롤 중인지 (BP에서 안내 문구 표시 등에 사용)
	UFUNCTION(BlueprintPure, Category = "PTB|UI")
	bool IsAutoScrolling() const { return bAutoScroll; }

protected:
	UFUNCTION()
	void OnCloseClicked();

	// 자동 스크롤이 멈추거나(직접 조작) 재개될 때 호출. BP에서 "휠 클릭: 자동 재생" 안내 표시/숨김 등에 사용 (선택)
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI")
	void OnAutoScrollChanged(bool bIsAutoScrolling);

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

	// 휠 한 칸당 스크롤 픽셀
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float WheelScrollAmount = 120.f;

	// 시작 전 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float StartDelay = 1.f;

	// 자동 스크롤로 끝에 도달한 뒤 자동으로 닫히기까지 대기 시간 (0 이하면 자동으로 닫지 않음)
	// 직접 스크롤해서 끝에 도달한 경우에는 자동으로 닫지 않음
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Credit")
	float AutoCloseDelay = 3.f;

private:
	// 맨 위(0) ~ 맨 아래(End) 범위로 제한해서 스크롤 오프셋 설정
	void SetClampedScrollOffset(float NewOffset);
	// 직접 조작 시작 → 자동 스크롤 일시정지
	void PauseAutoScroll();

	float ElapsedDelay = 0.f;
	float EndElapsed = 0.f;
	bool bFastForward = false;
	bool bAutoScroll = true;

	// 좌클릭 드래그
	bool bDragging = false;
	float LastDragLocalY = 0.f;
};
