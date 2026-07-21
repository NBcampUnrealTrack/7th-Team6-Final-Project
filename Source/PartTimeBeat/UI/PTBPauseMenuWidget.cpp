// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBPauseMenuWidget.h"
#include "Core/PTBGameModeBase.h"

UPTBPauseMenuWidget::UPTBPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RestoreMode    = EPTBMenuRestoreMode::GameOnly;
	bCloseOnEscape = false;
}

void UPTBPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	InitializeView();
}

void UPTBPauseMenuWidget::InitializeView()
{
}

void UPTBPauseMenuWidget::RequestResume()
{
	if (APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>())
	{
		GM->ResumeGame();
	}
}

void UPTBPauseMenuWidget::RequestRetry()
{
	if (APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>())
	{
		GM->RetryGame();
	}
}

void UPTBPauseMenuWidget::RequestExitToMenu()
{
	if (APTBGameModeBase* GM = GetWorld()->GetAuthGameMode<APTBGameModeBase>())
	{
		GM->ExitToMenu();
	}
}

FReply UPTBPauseMenuWidget::NativeOnKeyDown(
	const FGeometry& InGeometry,
	const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		RequestResume();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}