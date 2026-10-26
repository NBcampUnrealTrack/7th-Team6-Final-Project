#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
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

	const FPTBItemCategoryDef* FindCategory(FName CategoryId) const;
};
