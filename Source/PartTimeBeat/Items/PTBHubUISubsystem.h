#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "InputCoreTypes.h"
#include "PTBHubUISubsystem.generated.h"

class APlayerController;
class UPTBHubPopupWidget;
class UPTBLockerWidget;
class UPTBShopWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPTBHubInteractTargetChanged, AActor*, NewTarget);

/**
 * 허브(L_GameMap) 전용 UI 관리: 사물함·상점 팝업과 Enter 상호작용 대상.
 *
 * 입력 연결
 *  - C++: PTBPlayerControllerBase::InputKey에서 HandleHubKeyPressed 호출 (I → 사물함, Enter → 상호작용)
 *  - BP : ESC 메뉴의 "사물함" 버튼, 허브 HUD 버튼 등에서 Get PTBHubUISubsystem → Open Locker 호출
 *
 * 팝업은 한 번에 하나만 열림. 허브가 아닌 맵, 미니게임 진행 중, 일시정지 중에는 열리지 않음
 * 팝업이 열린 동안에는 UIOnly 입력이라 키가 PlayerController로 오지 않음 → 닫기는 팝업이 직접 처리
 */
UCLASS()
class PARTTIMEBEAT_API UPTBHubUISubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	virtual void Deinitialize() override;

	// ── 팝업 ──

	/** 사물함 열기. 열렸으면 true */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	bool OpenLocker();

	/**
	 * 상점 열기. 열렸으면 true
	 * @param SoldCategories 판매 카테고리 (비어 있으면 전체)
	 * @param WidgetClassOverride 비어 있으면 "PTB 아이템 설정"의 ShopWidgetClass
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	bool OpenShop(const FText& ShopName, const TArray<FName>& SoldCategories, TSubclassOf<UPTBShopWidget> WidgetClassOverride = nullptr);

	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	void CloseActivePopup();

	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	bool IsPopupOpen() const;

	/** 지금 팝업을 열 수 있는 상태인지 (허브 맵, 미니게임·일시정지 아님, 다른 팝업 없음) */
	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	bool CanOpenPopup() const;

	// ── 상호작용 대상 ──

	/** 범위에 들어온 상호작용 오브젝트 등록 (IPTBHubInteractable 구현 액터) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	void SetInteractTarget(AActor* Target);

	/** 범위에서 나갈 때. 현재 대상이 Target일 때만 해제 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	void ClearInteractTarget(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	AActor* GetInteractTarget() const { return InteractTarget.Get(); }

	/** 현재 대상과 상호작용. 처리했으면 true */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	bool TryInteract();

	/** 상호작용 대상 변경 (BP: HUD에 "Enter: 상점" 안내 표시 등) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|Hub")
	FOnPTBHubInteractTargetChanged OnInteractTargetChanged;

	// ── 입력 ──

	/** 허브 키 처리 (I / Enter). 소비했으면 true. PlayerController 또는 BP 입력에서 호출 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Hub")
	bool HandleHubKeyPressed(FKey Key);

	/** 현재 월드가 허브 맵인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|Hub")
	bool IsHubWorld() const;

private:
	/** 팝업을 열 수 없는 이유 (열 수 있으면 빈 문자열). 로그용 */
	FString GetBlockReason() const;

	APlayerController* GetLocalPlayerController() const;
	bool ShowPopup(UPTBHubPopupWidget* Popup);
	void HandlePopupClosed();

	UPROPERTY(Transient)
	TObjectPtr<UPTBHubPopupWidget> ActivePopup;

	TWeakObjectPtr<AActor> InteractTarget;
};
