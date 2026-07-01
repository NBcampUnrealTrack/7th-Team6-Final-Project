#include "PTBModalMenuWidget.h"

void UPTBModalMenuWidget::OpenMenu()
{
	AddToViewport(MenuZOrder);

	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		bPreviousCursorVisible = PC->bShowMouseCursor;

		// SetWidgetToFocus: UIOnly 입력 모드에서 이 위젯을 키보드 포커스 대상으로 지정
		// → 클릭 없이도 열자마자 ESC 등 키 입력이 이 위젯으로 전달됨
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}

	// Slate 포커스도 명시적으로 이 위젯에 설정
	SetKeyboardFocus();
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

FReply UPTBModalMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 터널링 단계에서 ESC 처리 → 자식 버튼이 포커스를 갖고 있어도 동작함
	if (bCloseOnEscape && InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UPTBModalMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 이 위젯 자체가 포커스를 가질 때의 폴백 처리
	if (bCloseOnEscape && InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseMenu();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
