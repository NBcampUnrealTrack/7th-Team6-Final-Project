#include "MiniGames/Dodge/PTBDodgeMiniGame.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Components/InputComponent.h"

APTBDodgeMiniGame::APTBDodgeMiniGame()
{
    PrimaryActorTick.bCanEverTick = true;
    MiniGameId = FName("FOD");
    MiniGameCode = FName("FOD");
}

void APTBDodgeMiniGame::BeginPlay()
{
    Super::BeginPlay();
}

void APTBDodgeMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();

    Health = 100;
    DodgeCount = 0;
    HitCount = 0;
    bIsJumping = false;
    ObstacleFallSpeed = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] BuildRuntimeState 완료"));
}

void APTBDodgeMiniGame::StartMiniGame()
{
    if (bIsRoundActive) return;

    float BPM = GameContext.ChartData.BPM;
    ObstacleFallSpeed = CalculateObstacleFallSpeed(BPM);
    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] StartMiniGame. BPM: %.1f, 장애물 속도: %.1f"), BPM, ObstacleFallSpeed);

    Super::StartMiniGame();
}

void APTBDodgeMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);
    OnObstacleSpawn(ObstacleFallSpeed, Note.BeatTime, Note.Lane);
}

void APTBDodgeMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);
    OnObstacleArmed(Note.BeatTime);
}

void APTBDodgeMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);

    if (Result.JudgementType == EPTBJudgementType::Miss)
    {
        Health -= 10;
        HitCount++;
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 맞음! 체력: %d"), Health);
        if (Health <= 0)
        {
            Health = 0;
            UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 체력 소진! 게임 종료"));
            FinishMiniGame(EPTBRoundEndReason::Failed);
        }
    }
    else
    {
        DodgeCount++;
        if (ScoreCalculator)
        {
            ScoreCalculator->ComboCount += GetScoreMultiplier();
        }
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 피함! DodgeCount: %d"), DodgeCount);
    }

    int32 Score = ScoreCalculator ? ScoreCalculator->CurrentScore : 0;
    OnScoreUpdated(Score);
    OnJudgementUpdated(Result.JudgementType);
}

void APTBDodgeMiniGame::InitializeMiniGame(const FPTBMiniGameContext& Context)
{
    Super::InitializeMiniGame(Context);
}

int32 APTBDodgeMiniGame::GetScoreMultiplier() const
{
    switch (GameContext.SessionRequest.Difficulty)
    {
    case EPTBDifficulty::Easy:     return 1;
    case EPTBDifficulty::Standard: return 2;
    case EPTBDifficulty::Insane:   return 3;
    default:                       return 1;
    }
}

float APTBDodgeMiniGame::CalculateObstacleFallSpeed(float BPM) const
{
    float LookAheadMs = GetLookAheadMsByDifficulty();
    LookAheadMs = FMath::Max(LookAheadMs, MinReactionTimeMs);
    float Speed = 1000.0f / (LookAheadMs / 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] BPM: %.1f, LookAheadMs: %.1f, 속도: %.1f"), BPM, LookAheadMs, Speed);
    return Speed;
}

float APTBDodgeMiniGame::GetLookAheadMsByDifficulty() const
{
    switch (GameContext.SessionRequest.Difficulty)
    {
    case EPTBDifficulty::Easy:     return 1000.0f;
    case EPTBDifficulty::Standard: return 500.0f;
    case EPTBDifficulty::Insane:   return 250.0f;
    default:                       return 500.0f;
    }
}

FPTBMiniGameResultPayload APTBDodgeMiniGame::BuildResultPayload() const
{
    FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();
    Payload.PayloadType = FName("DodgeMiniGame");
    Payload.IntValues.Add(FName("HitCount"), HitCount);
    Payload.IntValues.Add(FName("DodgeCount"), DodgeCount);
    Payload.IntValues.Add(FName("FinalHealth"), Health);
    return Payload;
}

float APTBDodgeMiniGame::ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const
{
    // 시스템의 250ms 지연을 강제로 보정하여 싱크를 맞춥니다.
 /*   float Result = -250.0f;
    UE_LOG(LogTemp, Warning, TEXT("[DodgeMiniGame] 강제 보정 적용! Result: %.2f"), Result); */

    return Super::ResolveInputOffsetMs(Context);

}