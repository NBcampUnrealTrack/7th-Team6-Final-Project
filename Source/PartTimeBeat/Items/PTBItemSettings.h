#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "InputCoreTypes.h"
#include "Items/PTBItemTypes.h"
#include "PTBItemSettings.generated.h"

class UDataTable;

/**
 * 아이템(사물함/상점) 설정.
 * 에디터: Edit → Project Settings → Game → "PTB 아이템 설정"
 * 저장 위치: Config/DefaultGame.ini
 *
 * 패키징 주의: Config의 소프트 참조는 쿡 대상으로 자동 포함되지 않음.
 * ItemTable이 들어있는 폴더를 Project Settings → Packaging → Additional Asset Directories to Cook에 추가할 것.
 * (DataTable 안의 아이콘·에셋 소프트 참조는 DataTable이 쿡되면 같이 따라감)
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB 아이템 설정"))
class PARTTIMEBEAT_API UPTBItemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** 아이템 정의 DataTable (Row Structure: PTBItemRow) */
	UPROPERTY(Config, EditAnywhere, Category = "Item", meta = (RequiredAssetDataTags = "RowStructure=/Script/PartTimeBeat.PTBItemRow"))
	TSoftObjectPtr<UDataTable> ItemTable;

	/** 사물함/상점 탭 목록. 배열 순서 = 탭 순서 */
	UPROPERTY(Config, EditAnywhere, Category = "Item", meta = (TitleProperty = "CategoryId"))
	TArray<FPTBItemCategoryDef> Categories;

	// ── 허브 UI ──

	/** 사물함 WBP (부모: PTBLockerWidget) */
	UPROPERTY(Config, EditAnywhere, Category = "Hub UI")
	TSoftClassPtr<class UPTBLockerWidget> LockerWidgetClass;

	/** 상점 WBP 기본값 (부모: PTBShopWidget). 상점 오브젝트에서 개별 지정하면 그쪽이 우선 */
	UPROPERTY(Config, EditAnywhere, Category = "Hub UI")
	TSoftClassPtr<class UPTBShopWidget> ShopWidgetClass;

	/**
	 * 상호작용 대상(상점 등) 근처에서 화면에 띄울 키 안내 위젯 (부모: PTBKeyPromptWidget, 예: WBP_Hud_EnterGuide).
	 * 미니게임 입구 안내와 같은 위젯을 쓰면 방식이 통일됨
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Hub UI")
	TSoftClassPtr<class UPTBKeyPromptWidget> InteractPromptWidgetClass;

	/** 허브에서 사물함을 여는 키 */
	UPROPERTY(Config, EditAnywhere, Category = "Hub UI")
	FKey LockerKey = EKeys::I;

	/** 허브에서 상점 등 오브젝트와 상호작용하는 키 */
	UPROPERTY(Config, EditAnywhere, Category = "Hub UI")
	TArray<FKey> InteractKeys = { EKeys::Enter };

	const FPTBItemCategoryDef* FindCategory(FName CategoryId) const;
};
