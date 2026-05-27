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
	SetKeyboardFocus();
	HandleReadyToStart();
}

FReply UPTBMiniGameLoadingWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (bCanRequestStart && OwnerMiniGame)
	{
		OwnerMiniGame->HandleStartInput();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
