#include "PTBCreditWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"

void UPTBCreditWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	if (ButtonClose)
		ButtonClose->OnClicked.AddUniqueDynamic(this, &UPTBCreditWidget::OnCloseClicked);

	if (ScrollCredit)
	{
		ScrollCredit->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		ScrollCredit->SetAllowOverscroll(false);
	}

	RestartCredit();
}

void UPTBCreditWidget::RestartCredit()
{
	ElapsedDelay = 0.f;
	EndElapsed = 0.f;
	bFastForward = false;
	bReachedEnd = false;

	if (ScrollCredit)
		ScrollCredit->SetScrollOffset(0.f);
}

void UPTBCreditWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!ScrollCredit) return;

	// 시작 대기
	if (ElapsedDelay < StartDelay)
	{
		ElapsedDelay += InDeltaTime;
		return;
	}

	const float EndOffset = ScrollCredit->GetScrollOffsetOfEnd();

	// 레이아웃 계산 전에는 End가 0일 수 있음
	if (EndOffset <= 0.f) return;

	if (!bReachedEnd)
	{
		const float Speed = ScrollSpeed * (bFastForward ? FastForwardMultiplier : 1.f);

		// 현재 오프셋 기준으로 더해야 휠 수동 스크롤과 충돌하지 않음
		const float NewOffset = FMath::Min(ScrollCredit->GetScrollOffset() + Speed * InDeltaTime, EndOffset);
		ScrollCredit->SetScrollOffset(NewOffset);

		if (NewOffset >= EndOffset)
			bReachedEnd = true;
	}
	else if (AutoCloseDelay > 0.f)
	{
		EndElapsed += InDeltaTime;
		if (EndElapsed >= AutoCloseDelay)
			CloseCredit();
	}
}

FReply UPTBCreditWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::Escape)
	{
		CloseCredit();
		return FReply::Handled();
	}
	if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
	{
		bFastForward = true;
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UPTBCreditWidget::NativeOnKeyUp(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();

	if (Key == EKeys::SpaceBar || Key == EKeys::Enter)
	{
		bFastForward = false;
		return FReply::Handled();
	}
	return Super::NativeOnKeyUp(InGeometry, InKeyEvent);
}

FReply UPTBCreditWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bFastForward = true;
	return FReply::Handled();
}

FReply UPTBCreditWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bFastForward = false;
	return FReply::Handled();
}

void UPTBCreditWidget::OnCloseClicked()
{
	CloseCredit();
}

void UPTBCreditWidget::CloseCredit()
{
	bFastForward = false;
	RemoveFromParent();
}