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

    virtual void BuildRuntimeState() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleChartEvent(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

private:
    void SpawnCueForNote(const FPTBNoteEvent& Note);
    void ClearCueForJudgement(const FPTBJudgementResult& Result);
};
