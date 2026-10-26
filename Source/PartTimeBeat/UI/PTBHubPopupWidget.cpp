#include "UI/PTBHubPopupWidget.h"

#include "Components/Button.h"
#include "GameFramework/PlayerController.h"

UPTBHubPopupWidget::UPTBHubPopupWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeys.Add(EKeys::Escape);
	SetIsFocusable(true);
}

void UPTBHubPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Close)
	{
		Button_Close->OnClicked.AddUniqueDynamic(this, &UPTBHubPopupWidget::HandleCloseClicked);
	}
}

void UPTBHubPopupWidget::OpenPopup()
{
	if (bIsOpen)
	{
		return;
	}
	bIsOpen = true;

	AddToViewport(PopupZOrder);

	if (APlayerController* PC = GetOwningPlayer())
	{
		bPrevCursorVisible = PC->bShowMouseCursor;

		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	SetKeyboardFocus();
	OnPopupOpened();
}

void UPTBHubPopupWidget::ClosePopup()
{
	if (!bIsOpen)
	{
		return;
	}
	bIsOpen = false;

	OnPopupClosed();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (bPrevCursorVisible)
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetHideCursorDuringCapture(false);
			PC->SetInputMode(InputMode);
			PC->SetShowMouseCursor(true);
		}
		else
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->SetShowMouseCursor(false);
		}
	}

	RemoveFromParent();
	OnPopupClosedNative.Broadcast();
}

FReply UPTBHubPopupWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 터널링 단계라 자식 버튼에 포커스가 있어도 먼저 받음
	if (bIsOpen && !InKeyEvent.IsRepeat() && CloseKeys.Contains(InKeyEvent.GetKey()))
	{
		ClosePopup();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UPTBHubPopupWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 빈 곳을 클릭해 포커스를 잃으면 ESC가 안 먹는 문제 방지 → 포커스 되찾기
	SetKeyboardFocus();
	return FReply::Handled();
}

void UPTBHubPopupWidget::HandleCloseClicked()
{
	ClosePopup();
}
