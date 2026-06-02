#pragma once
#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "Core/PTBStructEnums.h"
#include "PTBDodgeMiniGameRuleSet.generated.h"

/**
 * 리듬 피하기 미니게임 규칙 정의
 */
UCLASS()
class PARTTIMEBEAT_API UPTBDodgeMiniGameRuleSet : public UPTBMiniGameRuleSet
{
    GENERATED_BODY()

public:
    UPTBDodgeMiniGameRuleSet();

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
};