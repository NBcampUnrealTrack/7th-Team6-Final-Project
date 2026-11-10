#include "MiniGames/DW/PTBDWKeySlotWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"

void UPTBDWKeySlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	bIsPressed = false;
	ApplyVisual();
}

void UPTBDWKeySlotWidget::SetupSlot(const FLinearColor& InNoteColor, const FText& InKeyLabel)
{
	NoteColor = InNoteColor;
	NoteColor.A = 1.0f;
	if (TXT_Key)
	{
		TXT_Key->SetText(InKeyLabel);
	}
	ApplyVisual();
	OnSlotSetup(NoteColor, InKeyLabel);
}

void UPTBDWKeySlotWidget::SetPressed(bool bInPressed)
{
	if (bIsPressed == bInPressed)
	{
		return;
	}
	bIsPressed = bInPressed;
	ApplyVisual();
	OnPressedChanged(bIsPressed);
}

void UPTBDWKeySlotWidget::ApplyVisual()
{
	if (IMG_Star)
	{
		FLinearColor StarColor = NoteColor;
		if (bIsPressed)
		{
			StarColor.R = FMath::Min(StarColor.R * PressedBrightness, 1.0f);
			StarColor.G = FMath::Min(StarColor.G * PressedBrightness, 1.0f);
			StarColor.B = FMath::Min(StarColor.B * PressedBrightness, 1.0f);
		}
		StarColor.A = bIsPressed ? 1.0f : IdleStarOpacity;
		IMG_Star->SetColorAndOpacity(StarColor);
	}

	if (IMG_PressHighlight)
	{
		IMG_PressHighlight->SetVisibility(bIsPressed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		if (bTintHighlightWithNoteColor)
		{
			if (UImage* HighlightImage = Cast<UImage>(IMG_PressHighlight))
			{
				HighlightImage->SetColorAndOpacity(NoteColor);
			}
		}
	}

	const float Scale = bIsPressed ? PressedScale : 1.0f;
	SetRenderScale(FVector2D(Scale, Scale));
}
