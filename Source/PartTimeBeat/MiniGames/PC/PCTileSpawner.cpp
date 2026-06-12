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
    FVector SpawnLocation = RailCharacter->GetActorLocation() + RailCharacter->GetActorForwardVector() * SpawnDistance;

    // 타일 스폰
    APCTileActor* Tile = GetWorld()->SpawnActor<APCTileActor>(TileClass, SpawnLocation, FRotator::ZeroRotator);

    if (Tile)
    {
        // 캐릭터 방향으로 날아오게
        FVector Dir = RailCharacter->GetActorLocation() - SpawnLocation;
        Tile->Launch(Dir);
    }
    return Tile;
}