#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/PC/PCRailCharacter.h"
#include "MiniGames/PC/PCTileSpawner.h"
#include "MiniGames/PC/PCTileImageActor.h"
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

    UPROPERTY(EditAnywhere)
    APCRailCharacter* RailCharacter;

    UPROPERTY(EditAnywhere)
    APCTileSpawner* TileSpawner;


    UFUNCTION(BlueprintCallable, Category = "Input")
   void HandleActionAInput();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void HandleActionAInputReleased();

    // 에디터에서 108개 텍스처 배열로 연결
    UPROPERTY(EditAnywhere)
    TArray<UTexture2D*> TileTextures;

    // 이미지 액터 스폰용 클래스
    UPROPERTY(EditAnywhere)
    TSubclassOf<APCTileImageActor> TileImageClass;

    UPROPERTY(EditAnywhere)
    EPTBDifficulty Difficulty;

protected:

    virtual void BuildRuntimeState() override;
    virtual void PreloadAudioAssets() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleChartEvent(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

private:

    // NoteId로 타일 찾기용
    UPROPERTY()
    TMap<int32, APCTileActor*> ActiveTiles;

};
