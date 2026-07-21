#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBLCRuleSet.generated.h"

/**
 * LC 미니게임 전용 RuleSet입니다.
 * LC 미니게임은 ActionA, ActionB, ActionC 세 가지 입력을 사용하며, 난이도별로 서로 다른 채보 Asset을 참조합니다.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBLCRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()
};
