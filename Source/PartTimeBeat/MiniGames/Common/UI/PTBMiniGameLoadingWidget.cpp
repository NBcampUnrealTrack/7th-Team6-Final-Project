#include "MiniGames/Common/UI/PTBMiniGameLoadingWidget.h"

#include "MiniGames/Common/PTBBaseMiniGame.h"

void UPTBMiniGameLoadingWidget::InitializeLoadingWidget(APTBBaseMiniGame* InOwnerMiniGame)
{
	OwnerMiniGame = InOwnerMiniGame;
	SetIsFocusable(true);
}

void UPTBMiniGameLoadingWidget::SetLoadingState()
{
	bCanRequestStart = false;
	HandleLoadingStarted();
}

void UPTBMiniGameLoadingWidget::SetReadyToStartState()
{
	bCanRequestStart = true;
	HandleReadyToStart();
	SetKeyboardFocus();
	SetUserFocus(GetOwningPlayer());
}

FReply UPTBMiniGameLoadingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	return RequestStartIfReady();
}

FReply UPTBMiniGameLoadingWidget::RequestStartIfReady()
{
	if (!bCanRequestStart || !OwnerMiniGame)
	{
		return FReply::Unhandled();
	}

	OwnerMiniGame->HandleStartInput();
	return FReply::Handled();
}
