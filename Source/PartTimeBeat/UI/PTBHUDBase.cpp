// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBHUDBase.h"

void UPTBHUDBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPTBHUDBase::ShowHud()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UPTBHUDBase::HideHud()
{
	SetVisibility(ESlateVisibility::Collapsed);
}