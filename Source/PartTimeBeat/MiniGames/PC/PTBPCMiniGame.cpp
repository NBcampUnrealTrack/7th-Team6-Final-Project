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

  /*  switch (Difficulty)
    {
    case EPTBDifficulty::Easy:
        TilePrefix = TEXT("EasyTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_EasyTiles");
        break;
    case EPTBDifficulty::Insane:
        TilePrefix = TEXT("InsaneTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_InsaneTiles");
        break;
    default:
        TilePrefix = TEXT("StandardTile");
        TextureFolderPath = TEXT("/Game/FreeAssets/PC/PC_StandardTiles");
        break;
    }*/

    //for (int32 i = 1; i <= 108; i++)
    //{
    //    FString AssetName = FString::Printf(TEXT("%s__%d_"), *TilePrefix, i);
    //    FString Path = FString::Printf(TEXT("%s/%s.%s"), *TextureFolderPath, *AssetName, *AssetName);

    //    UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Path);
    //    TileTextures.Add(Texture);

    //    if (!Texture)
    //    {
    //        UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *Path);
    //    }
    //}

        // 108개 텍스처 자동 로드
    for (int32 i = 1; i <= 108; i++)
    {
        FString Path = FString::Printf(
            TEXT("/Game/FreeAssets/PC/PC_StandardTiles/StandardTile__%d_.StandardTile__%d_"), i, i);

        UTexture2D* Texture = LoadObject<UTexture2D>(nullptr, *Path);

        if (Texture)
        {
            TileTextures.Add(Texture);
        }
        else
        {
            // 로드 실패 시 빈 자리 유지
            TileTextures.Add(nullptr);
            UE_LOG(LogTemp, Warning, TEXT("텍스처 로드 실패: %s"), *Path);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("로드된 텍스처 수: %d"), TileTextures.Num());

   
    RailCharacter = Cast<APCRailCharacter>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCRailCharacter::StaticClass()));

    TileSpawner = Cast<APCTileSpawner>(
        UGameplayStatics::GetActorOfClass(GetWorld(), APCTileSpawner::StaticClass()));
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

    if (AudioManager && !GameContext.ChartData.WwiseBankName.IsNone())
    {
        AudioManager->LoadSoundBank(GameContext.ChartData.WwiseBankName);
    }

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
        0.1f,  // 딜레이 초
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


    if (Result.JudgementType == EPTBJudgementType::Miss) return;

    UE_LOG(LogTemp, Warning, TEXT("TileImageClass: %s"), TileImageClass ? TEXT("있음") : TEXT("없음"));
    UE_LOG(LogTemp, Warning, TEXT("RailCharacter: %s"), RailCharacter ? TEXT("있음") : TEXT("없음"));
    UE_LOG(LogTemp, Warning, TEXT("NoteId: %d, 텍스처: %s"), Result.NoteId,
        TileTextures.IsValidIndex(Result.NoteId - 1) && TileTextures[Result.NoteId - 1] ? TEXT("있음") : TEXT("없음"));

    if (!RailCharacter || !RailCharacter->PCRailPath) return;

    int32 PointIndex = Result.NoteId;

    if (!TileTextures.IsValidIndex(PointIndex - 1)) return;

    FVector SpawnLocation = RailCharacter->PCRailPath->GetPointLocation(PointIndex);
    SpawnLocation.Z += 1.f;

    UE_LOG(LogTemp, Warning, TEXT("SpawnLocation: %s"), *SpawnLocation.ToString());

    APCTileImageActor* ImageActor = GetWorld()->SpawnActor<APCTileImageActor>(
        TileImageClass, SpawnLocation, FRotator(0.f, 0.f, 0.f));

    UE_LOG(LogTemp, Warning, TEXT("ImageActor: %s"), ImageActor ? TEXT("스폰성공") : TEXT("스폰실패"));

    if (ImageActor)
    {
        ImageActor->SetTexture(TileTextures[PointIndex - 1]);
    }

}

