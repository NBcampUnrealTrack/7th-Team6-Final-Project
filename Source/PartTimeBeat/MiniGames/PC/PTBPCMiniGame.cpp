#include "MiniGames/PC/PTBPCMiniGame.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "MiniGames/PC/PTBPCMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"

APTBPCMiniGame::APTBPCMiniGame()
{
    static ConstructorHelpers::FClassFinder<APCTileImageActor> ImageActorClass(
        TEXT("/Game/PTB/MiniGames/PC/Blueprints/BP_PCTileImageActor"));
    if (ImageActorClass.Succeeded())
        TileImageClass = ImageActorClass.Class;
}

void APTBPCMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();
    ActiveTiles.Empty();
    TileTextures.Empty();

    FString TilePrefix;
    FString TextureFolderPath;

    int32 TileCount = ChartAsset ? ChartAsset->NoteEvents.Num() : 0;

    UE_LOG(LogTemp, Warning, TEXT("현재 Difficulty: %d"), (int32)Difficulty);

    switch (Difficulty)
    {
    case EPTBDifficulty::Easy:
        TilePrefix = TEXT("EasyTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_EasyTiles");
        break;
    case EPTBDifficulty::Insane:
        TilePrefix = TEXT("InsaneTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_InsaneTiles");
        break;
    case EPTBDifficulty::Standard:
        TilePrefix = TEXT("StandardTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_StandardTiles");
        break;
    }

    for (int32 i = 1; i <= TileCount; i++)
    {
        FString AssetName = FString::Printf(TEXT("%s__%d_"), *TilePrefix, i);
        FString Path = FString::Printf(TEXT("%s/%s.%s"), *TextureFolderPath, *AssetName, *AssetName);

        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Path);
        TileTextures.Add(Texture);

        if (!Texture)
        {
            UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *Path);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("로드된 텍스처 수: %d, 총 노트 수: %d"), TileTextures.Num(), TileCount);
   
    RailCharacter = Cast<APCRailCharacter>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCRailCharacter::StaticClass()));

    TileSpawner = Cast<APCTileSpawner>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCTileSpawner::StaticClass()));

    if (RailCharacter)
    {
        RailCharacter->SelectRailByDifficulty(Difficulty);
    }
}


void APTBPCMiniGame::HandleActionAInput()
{
    HandleRhythmInput(EPTBActionType::ActionA);
}

void APTBPCMiniGame::HandleActionAInputReleased()
{
    HandleRhythmInputReleased(EPTBActionType::ActionA);
}

void APTBPCMiniGame::PreloadAudioAssets()
{
    Super::PreloadAudioAssets();
}

void APTBPCMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    if (!TileSpawner) return;

    APCTileActor* Tile = TileSpawner->SpawnTile();
    if (Tile)
    {
        ActiveTiles.Add(Note.NoteId, Tile);
    }
}

void APTBPCMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);

}

void APTBPCMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
    Super::HandleChartEvent(Note);
    if (!RailCharacter) return;

    FTimerHandle MoveDelayHandle;
    GetWorldTimerManager().SetTimer(
        MoveDelayHandle,
        RailCharacter,
        &APCRailCharacter::MoveOneStep,
        0.02f,  // 딜레이 초
        false
    );
}

void APTBPCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);

    FString Message;
    FColor Color;

    switch (Result.JudgementType)
    {
    case EPTBJudgementType::HighPerfect:
        Message = TEXT("HIGHPERFECT");
        Color = FColor::Cyan;
        break;
    case EPTBJudgementType::Perfect:
        Message = TEXT("PERFECT");
        Color = FColor::Green;
        break;
    case EPTBJudgementType::Good:
        Message = TEXT("GOOD");
        Color = FColor::Yellow;
        break;
    case EPTBJudgementType::Miss:
        Message = TEXT("MISS");
        Color = FColor::Red;
        break;
    default:
        Message = TEXT("?");
        Color = FColor::White;
        break;
    }

    GEngine->AddOnScreenDebugMessage(-1, 2.f, Color, Message);

    if (RailCharacter)
    {
        RailCharacter->PlayHandMontage(Result.JudgementType == EPTBJudgementType::Miss
            ? RailCharacter->HandFailMontage
            : RailCharacter->HandSuccessMontage);
    }

    if (Result.JudgementType == EPTBJudgementType::Miss) return;

    if (!RailCharacter || !RailCharacter->PCRailPath) return;

    int32 PointIndex = Result.NoteId;

    if (!TileTextures.IsValidIndex(PointIndex - 1)) return;

    FVector SpawnLocation = RailCharacter->PCRailPath->GetPointLocation(PointIndex);
    SpawnLocation.Z += -75.f;

    APCTileImageActor* ImageActor = GetWorld()->SpawnActor<APCTileImageActor>(
        TileImageClass, SpawnLocation, FRotator(0.f, 0.f, 0.f));

    if (ImageActor)
    {
        ImageActor->SetTexture(TileTextures[PointIndex - 1]);
    }

}

