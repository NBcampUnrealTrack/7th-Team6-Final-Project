#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBDodgeMiniGameRuleSet.generated.h"

/**
 * 리듬 피하기 미니게임 규칙 정의
 */
UCLASS()
class PARTTIMEBEAT_API UPTBDodgeMiniGameRuleSet : public UDataAsset
{
    GENERATED_BODY()

public:
    UPTBDodgeMiniGameRuleSet();

    // 미니게임 ID (채보 JSON 과 반드시 일치)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    FName MiniGameId = FName("Dodge");

    // 허용 입력 (점프만)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    TArray<EPTBActionType> SupportedActions;

    // Easy 난이도 LookAhead Beats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float LookAheadBeatsEasy = 2.0f;

    // Standard 난이도 LookAhead Beats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float LookAheadBeatsStandard = 1.0f;

    // Insane 난이도 LookAhead Beats
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float LookAheadBeatsInsane = 0.5f;

    // 체력 감소량
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 HealthDecreaseAmount = 10;

    // 최대 미스 횟수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 MaxMissCount = 10;
};
