#pragma once

#include "CoreMinimal.h"
#include "PTBCHTypes.generated.h"

/**
 * 커스텀 햄버거 미니게임 재료 종류
 */
UENUM(BlueprintType)
enum class EPTBCHIngredientType : uint8
{
    None        UMETA(DisplayName = "없음"),
    BreadBottom UMETA(DisplayName = "아랫빵"),
    BreadTop    UMETA(DisplayName = "윗빵"),
    Sauce       UMETA(DisplayName = "소스"),
    Tomato      UMETA(DisplayName = "토마토"),
    Lettuce     UMETA(DisplayName = "상추"),
    Patty       UMETA(DisplayName = "패티"),
    Egg         UMETA(DisplayName = "계란후라이"),
    Bacon       UMETA(DisplayName = "베이컨"),
    Cheese      UMETA(DisplayName = "치즈"),
    Submit      UMETA(DisplayName = "제출"),
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

/**
 * 완성된 햄버거의 재료 구성에 따른 손님 반응 종류
 * (실제 대사 문구는 블루프린트/DataTable에서 이 타입을 키로 관리)
 */
UENUM(BlueprintType)
enum class ECHCustomerReactionType : uint8
{
    /** 주문한 순서/재료와 완전히 일치 */
    Perfect         UMETA(DisplayName = "완벽함"),
    /** 빵 바로 위에 빵을 얹음 (사이에 다른 재료 없음) */
    BreadOnBread    UMETA(DisplayName = "빵 위에 빵"),
    /** 한 자리 이상 재료가 아예 빠짐 (타이밍 놓침) */
    MissingIngredient UMETA(DisplayName = "재료 빠짐"),
    /** 들어간 재료 구성은 같지만 순서가 뒤바뀜 */
    OrderShuffled   UMETA(DisplayName = "순서 뒤바뀜"),
    /** 주문과 다른 재료가 들어감 (그 외 일반적인 실수) */
    WrongIngredient UMETA(DisplayName = "재료 틀림"),
};