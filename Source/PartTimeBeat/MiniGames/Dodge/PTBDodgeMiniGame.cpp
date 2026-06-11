#include "MiniGames/Dodge/PTBDodgeMiniGame.h"
#include "Rhythm/PTBScoreCalculator.h"

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

    // 상태 초기화만 (로드/BPM 접근 금지)
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

    // 반드시 Super 호출 (BGM + Conductor 시작)
    Super::StartMiniGame();
}

void APTBDodgeMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
    // 반드시 Super 호출
    Super::HandleNoteCue(Note);

    // Blueprint 에 장애물 스폰 신호 전달
    OnObstacleSpawn(ObstacleFallSpeed, Note.BeatTime, Note.Lane);
}

void APTBDodgeMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    // 반드시 Super 호출 (JudgementSystem.RegisterNoteEvent 실행)
    Super::HandleNoteArm(Note);

    // Blueprint 에 판정 진입 신호 전달
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

void APTBDodgeMiniGame::OnJumpInput()
{
    if (!CanAcceptInput()) return;

    FPTBNoteEvent TargetNote;
    bool bHasNote = JudgementSystem && JudgementSystem->FindBestPendingNote(EPTBActionType::ActionA, GetCurrentInputJudgeTimeMs(), TargetNote);

    bIsJumping = true;
    HandleRhythmInput(EPTBActionType::ActionA);

    if (!bHasNote)
    {
        if (ScoreCalculator)
        {
            ScoreCalculator->CurrentScore = FMath::Max(0, ScoreCalculator->CurrentScore - 100);
        }
        OnScoreUpdated(ScoreCalculator ? ScoreCalculator->CurrentScore : 0);
    }

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 점프 입력"));
}

void APTBDodgeMiniGame::OnJumpInputB()
{
    if (!CanAcceptInput()) return;

    FPTBNoteEvent TargetNote;
    bool bHasNote = JudgementSystem && JudgementSystem->FindBestPendingNote(EPTBActionType::ActionB, GetCurrentInputJudgeTimeMs(), TargetNote);

    bIsJumping = true;
    HandleRhythmInput(EPTBActionType::ActionB);

    if (!bHasNote)
    {
        if (ScoreCalculator)
        {
            ScoreCalculator->CurrentScore = FMath::Max(0, ScoreCalculator->CurrentScore - 100);
        }
        OnScoreUpdated(ScoreCalculator ? ScoreCalculator->CurrentScore : 0);
    }

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] B 점프 입력"));
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

    // 안전장치: 최소 반응 시간 보장 (150ms)
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
    case EPTBDifficulty::Easy:     return 1000.0f;
    case EPTBDifficulty::Standard: return 500.0f;
    case EPTBDifficulty::Insane:   return 250.0f;
    default:                       return 500.0f;
    }
}

FPTBMiniGameResultPayload APTBDodgeMiniGame::BuildResultPayload() const
{
    // 반드시 Super 호출 (기본 결과 포함)
    FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();

    Payload.PayloadType = FName("DodgeMiniGame");
    Payload.IntValues.Add(FName("HitCount"), HitCount);
    Payload.IntValues.Add(FName("DodgeCount"), DodgeCount);
    Payload.IntValues.Add(FName("FinalHealth"), Health);

    return Payload;
}