// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTBDialogHostWidget.h"
#include "PTBProfileSelectWidget.generated.h"

/**
 * 프로필 선택 화면의 팝업 표시와 입력 처리를 관리하는 위젯
 */
UCLASS()
class PARTTIMEBEAT_API UPTBProfileSelectWidget : public UPTBDialogHostWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	/** 소멸 시 bIgnoreInput 초기화 — 레벨 이동 후 입력 차단 방지 */
	virtual void NativeDestruct() override;
};

