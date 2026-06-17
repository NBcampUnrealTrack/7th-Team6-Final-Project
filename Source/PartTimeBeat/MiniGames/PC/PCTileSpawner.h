
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MiniGames/PC/PCTileActor.h"
#include "MiniGames/PC/PCRailCharacter.h"
#include "PCTileSpawner.generated.h"

UCLASS()
class PARTTIMEBEAT_API APCTileSpawner : public AActor
{
    GENERATED_BODY()

public:
    APCTileSpawner();

    // 스폰할 타일 클래스 (에디터에서 BP_PCTile 연결)
    UPROPERTY(EditAnywhere)
    TSubclassOf<APCTileActor> TileClass;

    // 캐릭터 참조 (에디터에서 연결)
    UPROPERTY(EditAnywhere)
    APCRailCharacter* RailCharacter;

    // 캐릭터 정면으로부터 스폰 거리
    UPROPERTY(EditAnywhere)
    float SpawnDistance = 2000.f;
 
   // 스폰 높이 오프셋
    UPROPERTY(EditAnywhere)
    float SpawnHeightOffset = 1000.f;

    // 박자 타이밍에 호출
    APCTileActor* SpawnTile();
};