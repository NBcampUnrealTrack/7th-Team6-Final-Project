#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBTGMiniGameRuleSet.generated.h"

/**
 * 공용 리듬 코어 검증용 TestGame 규칙을 정의하는 RuleSet DataAsset입니다
 *
 * TestGame은 실제 미니게임 연출보다 채보 로드, Cue, Arm, 판정, Hold/Release 페어링, 결과 확인을 목적으로 사용합니다
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBTGMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/** Cue 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	bool bLogNoteCue = true;

	/** Arm 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	bool bLogNoteArm = true;

	/** NoteEvent 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	bool bLogNoteEvent = true;

	/** 판정 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	bool bLogJudgement = true;

	/** LongNote 상세 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|TestGame")
	bool bLogLongNoteDetails = true;

};
