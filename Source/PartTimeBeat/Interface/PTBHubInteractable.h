#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PTBHubInteractable.generated.h"

class APlayerController;

/**
 * 허브(L_GameMap)에서 Enter로 상호작용하는 오브젝트 (상점 등).
 * (미니게임용 IPTBInteractable과는 별개)
 *
 * 흐름: 플레이어가 범위에 들어오면 오브젝트가 PTBHubUISubsystem::SetInteractTarget(this) 호출
 *       → Enter 입력 시 서브시스템이 OnHubInteract 호출
 * BP 액터에서도 Class Settings → Interfaces에 추가해 구현 가능
 */
UINTERFACE(BlueprintType, Blueprintable)
class UPTBHubInteractable : public UInterface
{
	GENERATED_BODY()
};

class PARTTIMEBEAT_API IPTBHubInteractable
{
	GENERATED_BODY()

public:
	/** 상호작용 실행. 실제로 처리했으면 true (false면 Enter 입력을 다른 곳에서 처리하도록 넘김) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "PTB|Hub")
	bool OnHubInteract(APlayerController* Player);
};
