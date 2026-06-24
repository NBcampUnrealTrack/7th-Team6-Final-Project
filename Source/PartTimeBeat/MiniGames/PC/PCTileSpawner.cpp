#include "MiniGames/PC/PCTileSpawner.h"
#include "Kismet/GameplayStatics.h"

APCTileSpawner::APCTileSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

APCTileActor* APCTileSpawner::SpawnTile()
{
    if (!TileClass || !RailCharacter) return nullptr;

    // 판정존 위치를 목표점으로
    FVector TargetLocation = RailCharacter->JudgementZone->GetComponentLocation();

    FVector SpawnLocation = TargetLocation + RailCharacter->GetActorForwardVector() * SpawnDistance + FVector(0.f, 0.f, SpawnHeightOffset);

    APCTileActor* Tile = GetWorld()->SpawnActor<APCTileActor>(TileClass, SpawnLocation, FRotator::ZeroRotator);

    if (Tile)
    {
        FVector Dir = TargetLocation - SpawnLocation; // 캐릭터 대신 판정존으로
        Tile->Launch(Dir);
    }

    return Tile;
}