// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileSelectWidget.h"

void UPTBProfileSelectWidget::NativeConstruct()
  {
	Super::NativeConstruct();

	SetKeyboardFocus();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
  }
