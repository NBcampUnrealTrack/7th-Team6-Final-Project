// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileSelectWidget.h"
#include "Profile/PTBProfileSubsystem.h"

void UPTBProfileSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UPTBProfileSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBProfileSubsystem>() : nullptr)
	{
		ProfileListChangedHandle = PS->OnProfileListChanged.AddUObject(this, &UPTBProfileSelectWidget::HandleProfileListChanged);
	}

	SetKeyboardFocus();

	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;
	
	FInputModeUIOnly InputMode;
	PC->SetInputMode(InputMode);
	PC->SetShowMouseCursor(true);
}

void UPTBProfileSelectWidget::HandleProfileListChanged()
{
	OnProfileListUpdated();
}

void UPTBProfileSelectWidget::NativeDestruct()
{
	if (UPTBProfileSubsystem* PS = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBProfileSubsystem>() : nullptr)
	{
		PS->OnProfileListChanged.Remove(ProfileListChangedHandle);
	}
	ProfileListChangedHandle.Reset();

	Super::NativeDestruct();
	
	APlayerController* PC = GetOwningPlayer();
	if (PC)
	{
		PC->SetInputMode(FInputModeGameAndUI());
	}
}
