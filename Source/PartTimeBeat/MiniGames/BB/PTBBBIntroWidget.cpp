#include "MiniGames/BB/PTBBBIntroWidget.h"

void UPTBBBIntroWidget::ResetIntroPages()
{
	CurrentPageIndex = 0;
	ShowCurrentPage();
}

FReply UPTBBBIntroWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() != EKeys::Enter)
	{
		return FReply::Unhandled();
	}

	if (CurrentPageIndex + 1 < IntroTextures.Num())
	{
		++CurrentPageIndex;
		ShowCurrentPage();
		return FReply::Handled();
	}

	// 마지막 페이지: 부모 클래스의 시작 처리로 이어짐 (RequestStartIfReady)
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPTBBBIntroWidget::ShowCurrentPage()
{
	if (!IntroTextures.IsValidIndex(CurrentPageIndex))
	{
		return;
	}
	OnIntroPageChanged(IntroTextures[CurrentPageIndex], CurrentPageIndex);
}
