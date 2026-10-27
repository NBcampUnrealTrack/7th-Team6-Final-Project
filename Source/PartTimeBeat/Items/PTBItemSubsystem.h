#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/PTBItemTypes.h"
#include "PTBItemSubsystem.generated.h"

class UDataTable;
class UPTBProfileSubsystem;
struct FPTBProfileData;

/** 보유/장착 상태가 바뀌었을 때. Category=None이면 전체 갱신(활성 프로필 변경 등) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPTBItemStateChanged, FName, Category);

/** 소지금이 바뀌었을 때 (구매 등) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPTBMoneyChanged, int32, CurrentMoney);

/**
 * 사물함(보유·장착) / 상점(구매) 로직.
 *
 * 데이터 위치
 *  - 아이템 정의: "PTB 아이템 설정"의 ItemTable (DataTable)
 *  - 보유·장착·사용 금액: 활성 프로필(FPTBProfileData)에 저장 → 프로필별로 따로 관리
 *
 * 규칙 (3안)
 *  - 구매한 아이템은 영구 보유, 카테고리마다 1개 장착
 *  - 장착해도 목록에서 사라지지 않음 (bEquipped로 표시)
 *  - 기본 아이템(bIsDefault)은 항상 보유. 장착 기록이 없거나 잘못되면 기본 아이템이 장착된 것으로 취급
 *
 * 개발 프로필(PIE 직접 실행)에서 구매한 내용은 디스크에 저장되지 않음 (기존 개발 프로필 정책과 동일)
 */
UCLASS()
class PARTTIMEBEAT_API UPTBItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 카탈로그 ──────────────────────────────────────────────

	/** 탭 목록 (bVisible=false 제외, 설정 순서대로) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	TArray<FPTBItemCategoryDef> GetVisibleCategories() const;

	/** 아이템 정의 조회 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	FPTBItemRow GetItemData(FName ItemId, bool& bOutFound) const;

	/** 카테고리의 기본 아이템 ID. 없으면 None */
	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	FName GetDefaultItemId(FName Category) const;

	// ── 목록 (UI용) ───────────────────────────────────────────

	/** 사물함 탭 목록: 해당 카테고리에서 보유한 아이템 (기본 아이템 포함), 정렬 완료 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item|Locker")
	TArray<FPTBItemView> GetOwnedItemsInCategory(FName Category) const;

	/** 상점 탭 목록: 해당 카테고리에서 상점 진열 아이템 (보유 여부 포함), 정렬 완료 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item|Shop")
	TArray<FPTBItemView> GetShopItemsInCategory(FName Category) const;

	// ── 상태 조회 (활성 프로필 기준) ─────────────────────────

	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	bool IsItemOwned(FName ItemId) const;

	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	bool IsItemEquipped(FName ItemId) const;

	/** 카테고리에 장착 중인 아이템 ID (기록 없으면 기본 아이템) */
	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	FName GetEquippedItemId(FName Category) const;

	/** 카테고리에 장착 중인 아이템 정의 (캐릭터 머리색·판정 스킨·탈것 적용 시 사용) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	FPTBItemRow GetEquippedItemData(FName Category, bool& bOutFound) const;

	/** 활성 프로필의 소지금 (누적 알바비 - 사용 금액) */
	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	int32 GetCurrentMoney() const;

	// ── 행동 ─────────────────────────────────────────────────

	/** 구매 가능 여부만 확인 (버튼 활성화 판단용, 상태 변경 없음) */
	UFUNCTION(BlueprintPure, Category = "PTB|Item|Shop")
	EPTBPurchaseResult CanPurchase(FName ItemId) const;

	/** 구매. 성공 시 즉시 저장. bEquipAfterPurchase=true면 구매 후 바로 장착 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item|Shop")
	EPTBPurchaseResult TryPurchase(FName ItemId, bool bEquipAfterPurchase = false);

	/** 장착. 보유한 아이템만 가능. 같은 카테고리의 기존 장착은 자동 해제 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item|Locker")
	bool TryEquip(FName ItemId);

	/** 보상 등으로 아이템 지급 (구매 없이 보유 추가). 이미 보유 중이면 false */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	bool GrantItem(FName ItemId);

	// ── 이벤트 ───────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "PTB|Item")
	FOnPTBItemStateChanged OnItemStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "PTB|Item")
	FOnPTBMoneyChanged OnMoneyChanged;

private:
	void LoadCatalog();
	void ValidateCatalog() const;

	const FPTBItemRow* FindItem(FName ItemId) const;
	TArray<FPTBItemView> BuildViews(FName Category, bool bOwnedOnly, bool bShopOnly) const;

	UPTBProfileSubsystem* GetProfileSubsystem() const;
	const FPTBProfileData* GetActiveProfileConst() const;
	FPTBProfileData* GetActiveProfileMutable() const;

	bool IsOwnedInProfile(const FPTBProfileData& Profile, FName ItemId, const FPTBItemRow& Row) const;
	FName ResolveEquipped(const FPTBProfileData& Profile, FName Category) const;

	void HandleActiveProfileChanged(const FPTBProfileData& NewProfile);

	/** 로드한 DataTable 원본 (참조 유지용) */
	UPROPERTY(Transient)
	TObjectPtr<UDataTable> ItemTable;

	/** 아이템 정의 (ItemId → Row 복사본) */
	TMap<FName, FPTBItemRow> ItemsById;

	/** 카테고리 → 기본 아이템 ID */
	TMap<FName, FName> DefaultItemByCategory;

	FDelegateHandle ActiveProfileChangedHandle;
};
