#include "PTBCreditWidget.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"

void UPTBCreditWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);

	// UserWidget의 기본 Visibility는 Self Hit Test Invisible이라 자기 자신은 마우스 입력(클릭/휠/드래그)을 받지 못함.
	// Visible로 바꿔야 NativeOnMouseButtonDown / NativeOnMouseWheel / NativeOnMouseMove가 호출됨.
	SetVisibility(ESlateVisibility::Visible);

	if (ButtonClose)
		ButtonClose->OnClicked.AddUniqueDynamic(this, &UPTBCreditWidget::OnCloseClicked);

	if (ScrollCredit)
	{
		ScrollCredit->SetScrollBarVisibility(ESlateVisibility::Collapsed);
		ScrollCredit->SetAllowOverscroll(false);
		// 휠 입력을 ScrollBox가 먹지 않고 이 위젯(NativeOnMouseWheel)으로 넘기도록 설정
		// → 휠로 스크롤할 때 자동 스크롤 일시정지 처리를 한 곳에서 할 수 있음
		ScrollCredit->SetConsumeMouseWheel(EConsumeMouseWheel::Never);
	}

	RestartCredit();
}

void UPTBCreditWidget::RestartCredit()
{
	ElapsedDelay = 0.f;
	EndElapsed = 0.f;
	bFastForward = false;
	bDragging = false;

	if (ScrollCredit)
		ScrollCredit->SetScrollOffset(0.f);

	ResumeAutoScroll();
}

void UPTBCreditWidget::ResumeAutoScroll()
{
	EndElapsed = 0.f;
	if (!bAutoScroll)
	{
		bAutoScroll = true;
		OnAutoScrollChanged(true);
	}
}

void UPTBCreditWidget::PauseAutoScroll()
{
	EndElapsed = 0.f;
	if (bAutoScroll)
	{
		bAutoScroll = false;
		OnAutoScrollChanged(false);
	}
}

void UPTBCreditWidget::SetClampedScrollOffset(float NewOffset)
{
	if (!ScrollCredit) return;

	const float EndOffset = FMath::Max(0.f, ScrollCredit->GetScrollOffsetOfEnd());
	ScrollCredit->SetScrollOffset(FMath::Clamp(NewOffset, 0.f, EndOffset));
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

	// 직접 조작 중(휠/드래그 후 일시정지 상태)에는 자동 스크롤·자동 닫기 모두 하지 않음
	if (!bAutoScroll) return;

	const float CurrentOffset = ScrollCredit->GetScrollOffset();
	if (CurrentOffset < EndOffset)
	{
		EndElapsed = 0.f;
		const float Speed = ScrollSpeed * (bFastForward ? FastForwardMultiplier : 1.f);
		SetClampedScrollOffset(CurrentOffset + Speed * InDeltaTime);
	}
	else if (AutoCloseDelay > 0.f)
	{
		// 자동 스크롤로 끝에 도달 → 잠시 뒤 자동으로 닫기
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
	const FKey Button = InMouseEvent.GetEffectingButton();

	// 휠 클릭: 자동 스크롤 재개
	if (Button == EKeys::MiddleMouseButton)
	{
		ResumeAutoScroll();
		return FReply::Handled();
	}

	// 좌클릭: 드래그 시작 (자동 스크롤 일시정지)
	if (Button == EKeys::LeftMouseButton)
	{
		bDragging = true;
		LastDragLocalY = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()).Y;
		PauseAutoScroll();
		// 위젯 밖으로 마우스가 나가도 드래그가 이어지도록 마우스 캡처
		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Handled();
}

FReply UPTBCreditWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && bDragging)
	{
		bDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Handled();
}

FReply UPTBCreditWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging && ScrollCredit)
	{
		const float CurrentY = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()).Y;
		const float DeltaY = CurrentY - LastDragLocalY;
		LastDragLocalY = CurrentY;

		// 위로 끌어올리면 아래 내용이 보이도록 (스마트폰 스크롤과 같은 방향)
		SetClampedScrollOffset(ScrollCredit->GetScrollOffset() - DeltaY);
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UPTBCreditWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (ScrollCredit)
	{
		PauseAutoScroll();
		// 휠을 아래로 굴리면(WheelDelta < 0) 아래 내용이 보이도록
		SetClampedScrollOffset(ScrollCredit->GetScrollOffset() - InMouseEvent.GetWheelDelta() * WheelScrollAmount);
		return FReply::Handled();
	}
	return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
}

void UPTBCreditWidget::NativeOnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
	bDragging = false;
}

void UPTBCreditWidget::OnCloseClicked()
{
	CloseCredit();
}

void UPTBCreditWidget::CloseCredit()
{
	bFastForward = false;
	bDragging = false;
	RemoveFromParent();
}
