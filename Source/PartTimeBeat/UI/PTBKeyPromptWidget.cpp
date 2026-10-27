#include "UI/PTBKeyPromptWidget.h"

#include "Components/TextBlock.h"
#include "UI/PTBKeyTextLibrary.h"

UPTBKeyPromptWidget::UPTBKeyPromptWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PromptText = NSLOCTEXT("PTBPrompt", "DefaultInteract", "상호작용: {Interact} 키");
}

void UPTBKeyPromptWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RefreshPromptText();
}

void UPTBKeyPromptWidget::SetPromptText(const FText& InPromptText)
{
	PromptText = InPromptText;
	RefreshPromptText();
}

FText UPTBKeyPromptWidget::GetFormattedPromptText() const
{
	return UPTBKeyTextLibrary::FormatKeyTokens(this, PromptText);
}

void UPTBKeyPromptWidget::RefreshPromptText()
{
	const FText Formatted = GetFormattedPromptText();
	if (Text_Prompt)
	{
		Text_Prompt->SetText(Formatted);
	}
	OnPromptRefreshed(Formatted);
}
