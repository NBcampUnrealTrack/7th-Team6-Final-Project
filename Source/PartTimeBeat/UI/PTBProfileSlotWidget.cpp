// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBProfileSlotWidget.h"

void UPTBProfileSlotWidget::SetSlotData(
	  bool bInHasProfile, int32 InSlotIndex,EPTBGender InGender,
	  const FString& InNickname, int32 InMoney,
	  int32 InCleared, int32 InTotal)
{
	bHasProfile = bInHasProfile;
	SlotIndex   = InSlotIndex;
	Gender      = InGender;
	Nickname    = InNickname;
	Money       = InMoney;
	ClearedStage = InCleared;
	TotalStage   = InTotal;
	
	RefreshDisplay(); // Blueprint에서 구현한 시각 갱신 호출
}

FReply UPTBProfileSlotWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		OnSlotClicked.Broadcast(SlotIndex);
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
