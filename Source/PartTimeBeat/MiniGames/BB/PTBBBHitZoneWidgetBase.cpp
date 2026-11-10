#include "MiniGames/BB/PTBBBHitZoneWidgetBase.h"

#include "Components/Widget.h"

void UPTBBBHitZoneWidgetBase::InitHitZone(EPTBActionType InActionType)
{
	SetActionType(InActionType);
}

void UPTBBBHitZoneWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 눌림 표시는 항상 꺼진 상태
	bIsPressed = false;
	if (IMG_PressHighlight)
	{
		IMG_PressHighlight->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTBBBHitZoneWidgetBase::SetPressed(bool bInPressed)
{
	if (bIsPressed == bInPressed)
	{
		return;
	}
	bIsPressed = bInPressed;

	if (IMG_PressHighlight)
	{
		IMG_PressHighlight->SetVisibility(bIsPressed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	const float Scale = bIsPressed ? PressedScale : 1.0f;
	SetRenderScale(FVector2D(Scale, Scale));

	OnPressedChanged(bIsPressed);
}
