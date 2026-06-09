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
	/** 소멸 시 입력 모드 복원 — 레벨 이동 후 UI 전용 입력 모드가 남는 문제 방지 */
	virtual void NativeDestruct() override;
};

