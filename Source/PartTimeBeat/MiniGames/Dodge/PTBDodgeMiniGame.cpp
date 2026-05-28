#include "MiniGames/Dodge/PTBDodgeMiniGame.h"
#include "Rhythm/PTBScoreCalculator.h"

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
    OnObstacleSpawn(ObstacleFallSpeed, Note.BeatTime);
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
    // 반드시 Super 호출 (ScoreCalculator + SFX 처리)
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
            UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 체력 소진! 게임 종료"));
            FinishMiniGame(EPTBRoundEndReason::Failed);
        }
    }
    else
    {
        // 장애물 피함 → 난이도 배율만큼 점수 증가
        DodgeCount++;

        // ScoreCalculator 통해 점수 추가
        if (ScoreCalculator)
        {
            ScoreCalculator->ComboCount += GetScoreMultiplier();
        }

        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 피함! DodgeCount: %d"), DodgeCount);
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