#include "MiniGames/PC/PCTileSpawner.h"
#include "Kismet/GameplayStatics.h"

APCTileSpawner::APCTileSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

APCTileActor* APCTileSpawner::SpawnTile()
{
    if (!TileClass || !RailCharacter) return nullptr;

    // 스폰 위치: 캐릭터 정면 SpawnDistance 앞
    FVector TargetLocation = RailCharacter->JudgementZone->GetComponentLocation();

    // 캐릭터 정면 앞 + 위쪽에서 스폰
    FVector SpawnLocation = TargetLocation + RailCharacter->GetActorForwardVector() * SpawnDistance + FVector(0.f, 0.f, SpawnHeightOffset);

    // 타일 스폰
    APCTileActor* Tile = GetWorld()->SpawnActor<APCTileActor>(TileClass, SpawnLocation, FRotator::ZeroRotator);

    if (Tile)
    {
        // 캐릭터 방향으로 날아오게
        FVector Dir = TargetLocation - SpawnLocation;
        Tile->Launch(Dir);
    }
    return Tile;
}