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
	/** 생성 시 Input Mode + 마우스 커서 자동 설정 */
	virtual void NativeConstruct() override;
};

