#include "MiniGames/Dodge/PTBDodgeMiniGame.h"

APTBDodgeMiniGame::APTBDodgeMiniGame()
{
    PrimaryActorTick.bCanEverTick = true;
    MiniGameId = FName("Dodge");
    MiniGameCode = FName("DG");
}

void APTBDodgeMiniGame::BeginPlay()
{
    Super::BeginPlay();
}

void APTBDodgeMiniGame::BuildRuntimeState()
{
    Super::BuildRuntimeState();

    // 상태 초기화만 (로드/BPM 접근 금지)
    CurrentScore = 0;
    Health = 100;
    DodgeCount = 0;
    HitCount = 0;
    bIsJumping = false;
    ObstacleFallSpeed = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] BuildRuntimeState 완료"));
}

void APTBDodgeMiniGame::StartMiniGame()
{
    // StartMiniGame 이후 BPM 접근 안전
    float BPM = GameContext.ChartData.BPM;
    ObstacleFallSpeed = CalculateObstacleFallSpeed(BPM);

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] StartMiniGame. BPM: %.1f, 장애물 속도: %.1f"), BPM, ObstacleFallSpeed);

    Super::StartMiniGame();
}

void APTBDodgeMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    Super::HandleNoteCue(Note);

    // Blueprint 에 장애물 스폰 신호 전달
    OnObstacleSpawn(ObstacleFallSpeed, Note.BeatTime);

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 예고: BeatTime=%.2f, 속도=%.1f"), Note.BeatTime, ObstacleFallSpeed);
}

void APTBDodgeMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    // 반드시 Super 호출 (JudgementSystem.RegisterNoteEvent 실행)
    Super::HandleNoteArm(Note);

    // Blueprint 에 판정 진입 신호 전달
    OnObstacleArmed(Note.BeatTime);

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 판정 진입: BeatTime=%.2f"), Note.BeatTime);
}

void APTBDodgeMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    Super::HandleJudgementResult(Result);

    if (Result.JudgementType == EPTBJudgementType::Miss)
    {
        // 장애물 맞음 → 체력 감소
        Health -= 10;
        HitCount++;
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 맞음! 체력: %d"), Health);

        // 체력 0 → 게임 종료
        if (Health <= 0)
        {
            Health = 0;
            UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 체력 소진! 최종 점수: %d"), CurrentScore);
            FinishMiniGame(EPTBRoundEndReason::Failed);
        }
    }
    else
    {
        // 장애물 피함 → 난이도 배율만큼 점수 증가
        DodgeCount++;
        CurrentScore += GetScoreMultiplier();
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 피함! 점수: %d"), CurrentScore);
    }
}

void APTBDodgeMiniGame::OnJumpInput()
{
    if (!CanAcceptInput()) return;

    bIsJumping = true;
    HandleRhythmInput(EPTBActionType::ActionA);
    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 점프 입력"));
}

int32 APTBDodgeMiniGame::GetScoreMultiplier() const
{
    switch (GameContext.SessionRequest.Difficulty)
    {
    case EPTBDifficulty::Easy:     return 1; // Easy: +1점
    case EPTBDifficulty::Standard: return 2; // Standard: +2점
    case EPTBDifficulty::Insane:   return 3; // Insane: +3점
    default:                       return 1;
    }
}

float APTBDodgeMiniGame::CalculateObstacleFallSpeed(float BPM) const
{
    float LookAheadMs = GetLookAheadMsByDifficulty();

    // 안전장치: 최소 반응 시간 보장
    LookAheadMs = FMath::Max(LookAheadMs, MinReactionTimeMs);

    // 속도 = 화면 높이 기준값 / LookAhead 시간(초)
    float Speed = 1000.0f / (LookAheadMs / 1000.0f);

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] BPM: %.1f, LookAheadMs: %.1f, 속도: %.1f"), BPM, LookAheadMs, Speed);

    return Speed;
}

float APTBDodgeMiniGame::GetLookAheadMsByDifficulty() const
{
    switch (GameContext.SessionRequest.Difficulty)
    {
    case EPTBDifficulty::Easy:     return 1000.0f; // 여유로운 반응 시간
    case EPTBDifficulty::Standard: return 500.0f;  // 평균 반응 시간
    case EPTBDifficulty::Insane:   return 250.0f;  // 빠른 반응 시간
    default:                       return 500.0f;
    }
}

FPTBMiniGameResultPayload APTBDodgeMiniGame::BuildResultPayload() const
{
    FPTBMiniGameResultPayload Payload;
    Payload.IntValues.Add(FName("FinalScore"), CurrentScore);
    Payload.IntValues.Add(FName("HitCount"), HitCount);
    Payload.IntValues.Add(FName("DodgeCount"), DodgeCount);
    Payload.IntValues.Add(FName("FinalHealth"), Health);
    return Payload;
}