#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "PTBItemTypes.generated.h"

class UTexture2D;

/**
 * 기본 카테고리 ID.
 * 카테고리는 FName이라 Project Settings → Game → "PTB 아이템 설정"에서 C++ 수정 없이 추가 가능.
 * 여기 상수는 코드에서 자주 쓰는 카테고리를 오타 없이 참조하기 위한 용도.
 */
namespace PTBItemCategory
{
	inline const FName Portrait  = TEXT("Portrait");
	inline const FName Hair      = TEXT("Hair");
	inline const FName JudgeSkin = TEXT("JudgeSkin");
	inline const FName Vehicle   = TEXT("Vehicle");
}

/** 사물함/상점 탭 하나의 정의 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBItemCategoryDef
{
	GENERATED_BODY()

	/** 카테고리 ID (아이템 DataTable의 Category와 같은 값) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FName CategoryId = NAME_None;

	/** 탭에 표시할 이름 (다국어 수집 대상) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FText DisplayName;

	/** 탭 아이콘 (선택) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** false면 사물함/상점 탭에 표시하지 않음 (준비 중인 카테고리 숨김용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	bool bVisible = true;
};

/**
 * 아이템 정의 (DataTable 행).
 * 행 이름(Row Name)이 곧 ItemId. 저장 데이터에는 ItemId만 남으므로 출시 후에는 행 이름을 바꾸지 말 것.
 *
 * 규칙
 *  - 카테고리마다 bIsDefault 아이템이 정확히 1개 있어야 함 (없거나 2개 이상이면 로그 경고)
 *  - 기본 아이템은 구매 없이 항상 보유한 것으로 취급 (세이브에 따로 저장하지 않음)
 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBItemRow : public FTableRowBase
{
	GENERATED_BODY()

	/**
	 * 소속 카테고리 ID (예: Portrait, Hair, JudgeSkin, Vehicle).
	 * "PTB 아이템 설정"의 Categories에 없는 값이면 시작 시 로그 경고
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FName Category = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item", meta = (MultiLine = true))
	FText Description;

	/** 목록 아이콘 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 가격 (골드) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item", meta = (ClampMin = "0"))
	int32 Price = 0;

	/** 카테고리 기본 아이템 (구매 없이 보유, 장착 대상이 없을 때 자동 장착) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	bool bIsDefault = false;

	/** 상점 진열 여부. false면 상점에는 안 나오고 다른 경로(보상 등)로만 획득 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	bool bSoldInShop = true;

	/** 같은 카테고리 안에서의 정렬 순서 (작을수록 앞) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	int32 SortOrder = 0;

	// ── 카테고리별 적용 데이터 (해당 카테고리에서 쓰는 것만 채우면 됨) ──

	/** 색상 (Hair: 머리색) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Payload")
	FLinearColor Color = FLinearColor::White;

	/** 에셋 (Portrait: 초상화 텍스처, JudgeSkin: 판정 스킨 에셋 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Payload")
	TSoftObjectPtr<UObject> Asset;

	/**
	 * 숫자 파라미터 (Vehicle 등).
	 * 예: SpeedMultiplier=1.5, ExtraJumpCount=1, BoostSpeedMultiplier=1.8
	 * 새 탈것이 기존 이동 로직의 수치만 바꾸는 경우 C++ 수정 없이 추가 가능
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Payload")
	TMap<FName, float> NumericParams;
};

/** UI 목록 한 칸 표시용 (정의 + 활성 프로필 기준 상태) */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBItemView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FName ItemId = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FPTBItemRow Data;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	bool bOwned = false;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	bool bEquipped = false;
};

UENUM(BlueprintType)
enum class EPTBPurchaseResult : uint8
{
	Success          UMETA(DisplayName = "구매 성공"),
	AlreadyOwned     UMETA(DisplayName = "이미 보유"),
	NotEnoughMoney   UMETA(DisplayName = "골드 부족"),
	NotForSale       UMETA(DisplayName = "판매하지 않는 아이템"),
	InvalidItem      UMETA(DisplayName = "존재하지 않는 아이템"),
	NoActiveProfile  UMETA(DisplayName = "활성 프로필 없음")
};
