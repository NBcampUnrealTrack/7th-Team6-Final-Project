// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBDialogHostWidget.h"
#include "Components/Overlay.h"

  void UPTBDialogHostWidget::PushDialog(UUserWidget* Dialog)
  {
  	if (!Dialog || !Overlay_Dialog) return;

  	Overlay_Dialog->AddChildToOverlay(Dialog);
  	DialogStack.Add(Dialog);
  }

void UPTBDialogHostWidget::CloseTopDialog()
  {
  	if (DialogStack.Num() == 0) return;

  	UUserWidget* Top = DialogStack.Last();
  	DialogStack.RemoveAt(DialogStack.Num() - 1);

  	if (Top)
  	{
  		Top->RemoveFromParent();
  		OnDialogClosed(Top);
  	}
  }

void UPTBDialogHostWidget::CloseAllDialogs()
  {
  	// 역순으로 닫아야 나중에 쌓인 것부터 처리됨
  	for (int32 i = DialogStack.Num() - 1; i >= 0; --i)
  	{
  		if (DialogStack[i])
  			DialogStack[i]->RemoveFromParent();
  	}
  	DialogStack.Empty();
  }

bool UPTBDialogHostWidget::IsDialogOpen(TSubclassOf<UUserWidget> DialogClass) const
  {
  	for (const UUserWidget* Dialog : DialogStack)
  	{
  		if (Dialog && Dialog->IsA(DialogClass))
  			return true;
  	}
  	return false;
  }

FReply UPTBDialogHostWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
  {
  	if (InKeyEvent.GetKey() == EKeys::Escape && DialogStack.Num() > 0)
  	{
  		CloseTopDialog();
  		return FReply::Handled();
  	}
  	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
  }
