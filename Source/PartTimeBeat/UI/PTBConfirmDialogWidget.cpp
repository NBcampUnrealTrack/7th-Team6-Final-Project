#include "UI/PTBConfirmDialogWidget.h"

#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "UI/PTBDialogHostWidget.h"

void UPTBConfirmDialogWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Yes) { Btn_Yes->OnClicked.AddUniqueDynamic(this, &UPTBConfirmDialogWidget::Confirm); }
	if (Btn_No)  { Btn_No->OnClicked.AddUniqueDynamic(this, &UPTBConfirmDialogWidget::Cancel); }
}

UPTBDialogHostWidget* UPTBConfirmDialogWidget::FindDialogHostOf(const UUserWidget* Widget)
{
	// Dialog Host의 Overlay_Dialog에 붙어 있으면: 부모 패널 → (WidgetTree) → Dialog Host 위젯
	if (Widget)
	{
		if (UPanelWidget* ParentPanel = Widget->GetParent())
		{
			return ParentPanel->GetTypedOuter<UPTBDialogHostWidget>();
		}
	}
	return nullptr;
}

void UPTBConfirmDialogWidget::CloseSelf()
{
	if (UPTBDialogHostWidget* Host = FindDialogHostOf(this))
	{
		if (Host->IsDialogOpen(GetClass()))
		{
			Host->CloseTopDialog();
			return;
		}
	}
	RemoveFromParent();
}

void UPTBConfirmDialogWidget::Confirm()
{
	CloseSelf();
	OnConfirmed.Broadcast();
}

void UPTBConfirmDialogWidget::Cancel()
{
	CloseSelf();
	OnCancelled.Broadcast();
}
