// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBHUDBase.generated.h"

class UButton;

/**
 * 인게임 HUD 계열 위젯의 공통 베이스 클래스
 * - HUD 표시/숨김 정책
 * - 공통 입력 버튼(일시정지 등) 바인딩 포인트
 * 자식 HUD에서 재사용하기 위한 기반 역할을 담당
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBHUDBase : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void ShowHud();

	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void HideHud();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonPause = nullptr;
};
