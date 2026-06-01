#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBPCMiniGame.generated.h"

class UPTBPCMiniGameRuleSet;

/**
 * 
 */
UCLASS()
class PARTTIMEBEAT_API APTBPCMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
	
public:
	APTBPCMiniGame();

protected:

	/** 오브젝트 / 상태 구성(하위 override) */
	void BuildRuntimeState() override;

	/** 선행 비주얼 큐(하위 override) */
	UFUNCTION()
	void HandleNoteCue(FPTBNoteEvent Note);

	/** 판정 등록 가능 상태 진입(하위 override) */
	UFUNCTION()
	void HandleNoteArm(FPTBNoteEvent Note) override;

	/** 채보 이벤트 처리(하위 override) */
	UFUNCTION()
	void HandleChartEvent(FPTBNoteEvent Note) override;

	/** 점수 / HUD / SFX 반영 */
	UFUNCTION()
	void HandleJudgementResult(FPTBJudgementResult Result) override;

	/** 미니게임 전용 피드백(하위 override) */
	void PlayJudgementFeedback(const FPTBJudgementResult& Result) override;

	/** 전용 결과(하위 override) */
	virtual FPTBMiniGameResultPayload BuildResultPayload() const override;

	/** 라운드 시작 전 에셋 준비(하위 override 권장) */
	void PreloadAssets() override;
	
	/** 오디오 에셋 준비 */
	void PreloadAudioAssets() override;

};
