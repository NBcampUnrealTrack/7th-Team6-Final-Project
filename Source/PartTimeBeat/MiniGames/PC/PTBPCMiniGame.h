#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "MiniGames/PC/PCRailCharacter.h"
#include "MiniGames/PC/PCTileSpawner.h"
#include "MiniGames/PC/PCTileImageActor.h"
#include "MiniGames/PC/PTBPCJudgementPopupWidget.h"
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

   
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Judgement")
    UPTBPCJudgementPopupWidget* JudgementPopupWidget;

protected:

    virtual void BuildRuntimeState() override;
    virtual void PreloadAudioAssets() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleChartEvent(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

    /** 튜토리얼 연습 ↔ 본게임 채보 전환 시 캐릭터 위치/타일 초기화 */
    virtual void HandleRoundChartChanged() override;

private:

    // NoteId로 타일 찾기용
    UPROPERTY()
    TMap<int32, APCTileActor*> ActiveTiles;

    /** 이번 판에 스폰한 날아오는 타일 / 완성 타일 이미지 (채보 전환 시 정리용) */
    UPROPERTY()
    TArray<TObjectPtr<AActor>> PCSpawnedActors;

    /** 튜토리얼 연습: NoteId → 레일 칸 번호(1부터). 본게임은 NoteId를 그대로 칸 번호로 사용 */
    TMap<int32, int32> PracticePointIndexByNoteId;

    /** 판정된 노트의 레일 칸 번호 */
    int32 ResolvePointIndex(int32 NoteId) const;

    /** 스폰한 타일/이미지 제거 */
    void ClearPCSpawnedActors();

};
