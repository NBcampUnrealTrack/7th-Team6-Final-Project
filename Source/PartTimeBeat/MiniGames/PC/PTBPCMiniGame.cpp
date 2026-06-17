#include "MiniGames/PC/PTBPCMiniGame.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
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
        0.5f,  // 딜레이 초
        false
    );
}

void APTBPCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);


}

