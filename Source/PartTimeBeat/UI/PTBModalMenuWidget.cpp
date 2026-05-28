#include "PTBModalMenuWidget.h"

void UPTBModalMenuWidget::OpenMenu()
{
	AddToViewport(MenuZOrder);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
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
		if (RestoreMode == EPTBMenuRestoreMode::GameOnly)
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