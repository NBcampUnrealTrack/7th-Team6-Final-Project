#pragma once

#include "CoreMinimal.h"
#include "PTBCHTypes.generated.h"

/**
 * 커스텀 햄버거 미니게임 재료 종류
 */
UENUM(BlueprintType)
enum class EPTBCHIngredientType : uint8
{
    Bread       UMETA(DisplayName = "빵"),
    Sauce       UMETA(DisplayName = "소스"),
    Tomato      UMETA(DisplayName = "토마토"),
    Lettuce     UMETA(DisplayName = "상추"),
    Patty       UMETA(DisplayName = "패티"),
    Egg         UMETA(DisplayName = "계란후라이"),
    Bacon       UMETA(DisplayName = "베이컨"),
    Cheese      UMETA(DisplayName = "치즈"),
    Submit      UMETA(DisplayName = "제출"),
    None        UMETA(DisplayName = "없음")
};

/**
 * 손님 주문 정보
 */
USTRUCT(BlueprintType)
struct FPTBCHCustomerOrder
{
    GENERATED_BODY()

    /** 손님이 원하는 재료 순서 (빵 포함) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<EPTBCHIngredientType> Ingredients;
};