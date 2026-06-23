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
{}

void APTBPCMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();
    ActiveTiles.Empty();

    // 레벨에서 자동으로 찾기
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

    // 화면에 출력 (5.f = 표시 시간(초))
    GEngine->AddOnScreenDebugMessage(-1, 2.f, Color, Message);

}

