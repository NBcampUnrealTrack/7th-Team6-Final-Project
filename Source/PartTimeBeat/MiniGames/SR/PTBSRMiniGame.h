#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBSRMiniGame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnSushiPlateSignature, int32, ToppingType, int32, NoteId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSushiSuccessSignature, int32, NoteId, EPTBJudgementType, JudgementType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSushiMissSignature, int32, NoteId);

UCLASS()
class PARTTIMEBEAT_API APTBSRMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
	
public:
	APTBSRMiniGame();

	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi") FOnSpawnSushiPlateSignature OnSushiPlateSpawn;
	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi") FOnSushiSuccessSignature OnSushiSuccessDelegate;
	UPROPERTY(BlueprintAssignable, Category = "PTB|Sushi") FOnSushiMissSignature OnSushiMissDelegate;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MiniGame|Setup") class UDataTable* ToppingDataTable;

	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	virtual void PlayJudgementFeedback(const FPTBJudgementResult& Result) override;
	virtual void BuildRuntimeState() override;
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const override;
private:
	UPROPERTY() TArray<int32> ToppingQueue;
	

	
};
