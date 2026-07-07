#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Engine/DataTable.h"
#include "PTBMDSMiniGame.generated.h"

USTRUCT(BlueprintType)
struct FPTBMDS_FoodRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomFood")
	FName MDS_FoodName; 

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomFood")
	float MDS_Money;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomFood")
	int32 MDS_Score;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "RandomFood")
	TSubclassOf<AActor> MDS_FoodMesh;
};

// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpawnFoodSignature, int32, RandomFood, int32, NoteId);
// DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDiscountSuccessSignature, int32, NoteId, EPTBJudgementType, JudgementType);

UCLASS()
class PARTTIMEBEAT_API APTBMDSMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()
	
public:
	APTBMDSMiniGame();

	virtual void BeginPlay() override;
	virtual void BuildRuntimeState() override;
	virtual void PreloadAudioAssets() override;
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	virtual TMap<FKey, EPTBActionType> GetActionMapping() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "PTB|MDS")
	TObjectPtr<UDataTable> FoodDataTable;
	UFUNCTION(BlueprintCallable, Category = "PTB|MDS")
	FPTBMDS_FoodRow GetRandomFoodData();
};
