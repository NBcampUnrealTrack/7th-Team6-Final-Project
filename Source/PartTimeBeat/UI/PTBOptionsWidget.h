#pragma once

#include "CoreMinimal.h"
#include "UI/PTBModalMenuWidget.h"
#include "PTBOptionsWidget.generated.h"

/**
 * 옵션 메뉴 위젯
 *
 * 타이틀/로비처럼 UI 포커스 상태에서 열리므로 RestoreMode 기본값은 UIOnly
 * 실제 설정 항목(음량, 해상도 등)은 BP에서 구현
 */
UCLASS()
class PARTTIMEBEAT_API UPTBOptionsWidget : public UPTBModalMenuWidget
{
	GENERATED_BODY()

public:
	UPTBOptionsWidget(const FObjectInitializer& ObjectInitializer);
};