#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/PTBStructEnums.h"
#include "PTBPlayerControllerBase.generated.h"

class APTBGameModeBase;
class UInputMappingContext;

/**
 * PartTimeBeat 공용 플레이어 컨트롤러.
 *
 * 담당하는 두 가지 입력 영역:
 *  1) ESC → PauseGame / ResumeGame 토글
 *  2) 리듬 입력 키(기본 Z/X/C/V/B) → 활성 미니게임으로 라우팅
 *  3) 허브 전용 키(I: 사물함, Enter: 상점 등 상호작용) → PTBHubUISubsystem
 *
 * 허브(L_GameMap)에서는 HubMappingContexts(이동·시야 IMC)를 등록해 캐릭터 조작을 켬.
 * 미니게임 맵에서는 등록하지 않음 (리듬 키 외 입력으로 캐릭터가 움직이지 않도록)
 *
 * 리듬 키 바인딩은 FPTBUserSettings.RhythmKeys에 저장되며
 * GetRhythmKeyBindings()로 읽어 언제든 반영된다.
 * 미니게임 별로 SupportedActions가 설정된 경우 해당 액션만 전달된다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

protected:
	virtual void BeginPlay() override;

	/**
	 * 허브 맵에서만 등록할 Enhanced Input 매핑 (예: IMC_Default, IMC_MouseLook).
	 * Third Person 템플릿 컨트롤러가 하던 이동·점프·시야 회전 입력 등록을 대신함
	 */
	UPROPERTY(EditDefaultsOnly, Category = "PTB|Input")
	TArray<TObjectPtr<UInputMappingContext>> HubMappingContexts;

	/** 모든 맵에서 등록할 Enhanced Input 매핑 (필요할 때만) */
	UPROPERTY(EditDefaultsOnly, Category = "PTB|Input")
	TArray<TObjectPtr<UInputMappingContext>> DefaultMappingContexts;

public:

	/**
	 * 현재 적용할 리듬 키 바인딩을 반환.
	 * PTBGameInstance::CachedSettings.RhythmKeys를 우선 사용하고
	 * GameInstance가 없으면 기본값(Z/X/C/V/B)을 반환한다.
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Input")
	FPTBRhythmKeyBindings GetRhythmKeyBindings() const;

private:
	/**
	 * 키 이벤트가 리듬 입력인지 확인하고 활성 미니게임으로 라우팅한다.
	 * 실제로 소비한 경우 true 반환.
	 */
	bool TryRouteRhythmInput(const FInputKeyEventArgs& EventArgs);
};
