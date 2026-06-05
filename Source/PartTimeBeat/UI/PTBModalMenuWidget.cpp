#include "PTBModalMenuWidget.h"

void UPTBModalMenuWidget::OpenMenu()
{
	AddToViewport(MenuZOrder);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		bPreviousCursorVisible = PC->bShowMouseCursor;

		FInputModeUIOnly InputMode;
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	OnMenuOpened();
}

void UPTBModalMenuWidget::CloseMenu()
{
	OnMenuClosed();

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		// RestoreMode가 명시적으로 지정된 경우 그것을 따르고,
		// 그렇지 않으면 OpenMenu() 시점의 상태로 자동 복원
		const bool bRestoreGameOnly =
			(RestoreMode == EPTBMenuRestoreMode::GameOnly) ||
			(RestoreMode != EPTBMenuRestoreMode::UIOnly && !bPreviousCursorVisible);

		if (bRestoreGameOnly)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->SetShowMouseCursor(false);
		}
		else
		{
			PC->SetInputMode(FInputModeUIOnly());
			PC->SetShowMouseCursor(true);
		}
	}

	RemoveFromParent();
}

FReply UPTBModalMenuWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (bCloseOnEscape && InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
