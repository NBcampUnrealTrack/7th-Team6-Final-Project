#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBCHMiniGameRuleSet.generated.h"

/**
 * 커스텀 햄버거 미니게임 규칙을 정의하는 RuleSet DataAsset입니다
 *
 * 손님이 원하는 재료 조합을 박자에 맞춰 선택/제출하는 리듬 게임입니다
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBCHMiniGameRuleSet : public UPTBMiniGameRuleSet
{
    GENERATED_BODY()

public:
    /** Cue 로그 출력 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    bool bLogNoteCue = true;

    /** Arm 로그 출력 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    bool bLogNoteArm = true;

    /** NoteEvent 로그 출력 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    bool bLogNoteEvent = true;

    /** 판정 로그 출력 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    bool bLogJudgement = true;

    /** LongNote 상세 로그 출력 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|CH")
    bool bLogLongNoteDetails = true;
};