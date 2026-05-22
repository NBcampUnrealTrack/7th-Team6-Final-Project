// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBProfileSlotWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlotClicked, int32, SlotIndex);

/**
 * 프로필 선택 화면에서 저장 슬롯의 정보를 표시하는 위젯
 * 슬롯 데이터 갱신과 클릭 선택 이벤트 전달을 담당
 */
UCLASS()
class PARTTIMEBEAT_API UPTBProfileSlotWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	/** 부모 화면에서 한 번에 데이터 세팅 + RefreshDisplay 자동 호출 */
	UFUNCTION(BlueprintCallable, Category = "Profile|Slot")
	void SetSlotData(bool bInHasProfile, int32 InSlotIndex,
					 const FString& InNickname = TEXT(""),
					 int32 InMoney = 0, int32 InCleared = 0, int32 InTotal = 0);

	/** 블루프린트에서 실제 텍스트/이미지 갱신 구현 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile|Slot")
	void RefreshDisplay();

	/** 슬롯 클릭 시 부모로 알리는 델리게이트 */
	UPROPERTY(BlueprintAssignable, Category = "Profile|Slot")
	FOnSlotClicked OnSlotClicked;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	bool bHasProfile = false;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	FString Nickname;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	int32 Money = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	int32 ClearedStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Profile|Slot")
	int32 TotalStage = 0;

protected:
	/** 마우스 클릭 → OnSlotClicked 발동 */
	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent) override;

};
