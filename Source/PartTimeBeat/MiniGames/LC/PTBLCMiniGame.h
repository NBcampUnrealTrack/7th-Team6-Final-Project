#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBLCMiniGame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBLCJudgementEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);

UCLASS()
class PARTTIMEBEAT_API APTBLCMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	APTBLCMiniGame();

	/** 오브젝트 / 상태 구성 */
	virtual void BuildRuntimeState() override;

	/** 채보 이벤트 처리 */
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;

	/** 판정 등록 가능 상태 진입 */
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;

	/** 선행 비주얼 큐 */
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;

	/** 점수 / HUD / SFX 반영 */
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	
	/** 판정 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|LC|Events")
	FPTBLCJudgementEvent OnJudgement;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|CollisionBox")
	TObjectPtr<UBoxComponent> CollisionBox;

public:
};
