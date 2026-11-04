#include "MiniGames/Dodge/PTBDodgeMiniGame.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Components/InputComponent.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Blueprint/UserWidget.h"

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

    // 튜토리얼 연습은 StartMiniGame을 거치지 않아서 낙하 속도가 0인 채로 노트가 스폰됨(안 떨어짐) → 여기서 보정
    if (ObstacleFallSpeed <= 0.0f)
    {
        ObstacleFallSpeed = CalculateObstacleFallSpeed(GameContext.ChartData.BPM);
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 낙하 속도 미설정 상태에서 노트 큐 → 속도 보정: %.1f"), ObstacleFallSpeed);
    }

    OnObstacleSpawn(ObstacleFallSpeed, Note.BeatTime, Note.Lane);
}

void APTBDodgeMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
    Super::HandleNoteArm(Note);
    OnObstacleArmed(Note.BeatTime);
}

void APTBDodgeMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
    // 1. 먼저 피버 상태 업데이트
    if (Result.JudgementType == EPTBJudgementType::Miss)
    {
        HighPerfectStreak = 0;
        if (bFeverMode)
        {
            bFeverMode = false;
            ScoreMultiplier = 1.0f;
            OnFeverModeChanged(false);
        }
    }
    else if (Result.JudgementType == EPTBJudgementType::HighPerfect)
    {
        HighPerfectStreak++;
        if (HighPerfectStreak >= 5 && !bFeverMode)
        {
            bFeverMode = true;
            ScoreMultiplier = 2.0f;
            OnFeverModeChanged(true);
        }
    }
    else if (Result.JudgementType == EPTBJudgementType::Good)
    {
        HighPerfectStreak = 0;
        if (bFeverMode)
        {
            bFeverMode = false;
            ScoreMultiplier = 1.0f;
            OnFeverModeChanged(false);
        }
    }

    // 2. 업데이트된 배율로 ScoreDelta 적용
    if (Result.JudgementType != EPTBJudgementType::Miss)
    {
        Result.ScoreDelta = FMath::RoundToInt(Result.ScoreDelta * ScoreMultiplier * GetScoreMultiplier());
    }

    Super::HandleJudgementResult(Result);

    // 3. HP/DodgeCount 처리 (튜토리얼 연습에서는 HP를 깎지 않음 → 연습 통과 여부는 튜토리얼 위젯이 판단)
    if (Result.JudgementType == EPTBJudgementType::Miss && IsTutorialPracticeActive())
    {
        HitCount++;
    }
    else if (Result.JudgementType == EPTBJudgementType::Miss)
    {
        Health -= 10;
        HitCount++;
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 맞음! 체력: %f"), Health);
        if (Health <= 0)
        {
            Health = 0;
            FinishMiniGame(EPTBRoundEndReason::Failed);
        }
    }
    else
    {
        DodgeCount++;
        UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 장애물 피함! DodgeCount: %d"), DodgeCount);
    }

    int32 Score = ScoreCalculator ? ScoreCalculator->CurrentScore : 0;
    OnScoreUpdated(Score);
    OnJudgementUpdated(Result.JudgementType);
}

void APTBDodgeMiniGame::HandleRoundChartChanged()
{
    Super::HandleRoundChartChanged();

    Health = 100;
    DodgeCount = 0;
    HitCount = 0;
    HighPerfectStreak = 0;
    ScoreMultiplier = 1.0f;
    if (bFeverMode)
    {
        bFeverMode = false;
        OnFeverModeChanged(false);
    }

    OnScoreUpdated(0);
}

EPTBDifficulty APTBDodgeMiniGame::ResolvePracticeUIDifficulty() const
{
    int32 MaxAction = 0;
    if (TutorialPracticeChart)
    {
        for (const FPTBNoteEvent& Note : TutorialPracticeChart->NoteEvents)
        {
            MaxAction = FMath::Max(MaxAction, static_cast<int32>(Note.ActionType));
        }
    }

    if (MaxAction >= static_cast<int32>(EPTBActionType::ActionE))
    {
        return EPTBDifficulty::Insane;
    }
    if (MaxAction >= static_cast<int32>(EPTBActionType::ActionC))
    {
        return EPTBDifficulty::Standard;
    }
    return EPTBDifficulty::Easy;
}

void APTBDodgeMiniGame::RemoveFODGameWidget()
{
    // BP 변수 이름 끝에 공백이 들어가 있어서 둘 다 찾아봄
    FObjectProperty* WidgetProp = FindFProperty<FObjectProperty>(GetClass(), TEXT("FODGameWidget "));
    if (!WidgetProp)
    {
        WidgetProp = FindFProperty<FObjectProperty>(GetClass(), TEXT("FODGameWidget"));
    }
    if (!WidgetProp)
    {
        return;
    }

    if (UUserWidget* Widget = Cast<UUserWidget>(WidgetProp->GetObjectPropertyValue_InContainer(this)))
    {
        Widget->RemoveFromParent();
    }
    WidgetProp->SetObjectPropertyValue_InContainer(this, nullptr);
}

void APTBDodgeMiniGame::ReceiveTutorialPracticeStarted_Implementation()
{
    Super::ReceiveTutorialPracticeStarted_Implementation();

    if (!bPracticeDifficultyOverridden)
    {
        SavedSessionDifficulty = GameContext.SessionRequest.Difficulty;
        bPracticeDifficultyOverridden = true;
    }

    const EPTBDifficulty PracticeDifficulty = ResolvePracticeUIDifficulty();
    GameContext.SessionRequest.Difficulty = PracticeDifficulty;
    ObstacleFallSpeed = CalculateObstacleFallSpeed(GameContext.ChartData.BPM);

    // 이전 연습 위젯이 남아 있으면 내리고, BP의 위젯 생성 이벤트(OnMiniGameReady)를 연습 난이도로 호출
    RemoveFODGameWidget();
    if (UFunction* ReadyFunc = FindFunction(TEXT("OnMiniGameReady")))
    {
        if (ReadyFunc->NumParms == 0)
        {
            ProcessEvent(ReadyFunc, nullptr);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[DodgeMiniGame] BP 이벤트 OnMiniGameReady를 찾지 못해 연습용 위젯을 만들지 못함"));
    }

    UE_LOG(LogTemp, Log, TEXT("[DodgeMiniGame] 튜토리얼 연습 UI 난이도=%d (원래 난이도=%d), 낙하 속도=%.1f"),
        static_cast<int32>(PracticeDifficulty), static_cast<int32>(SavedSessionDifficulty), ObstacleFallSpeed);
}

void APTBDodgeMiniGame::ReceiveTutorialPracticeFinished_Implementation(bool bPassed)
{
    Super::ReceiveTutorialPracticeFinished_Implementation(bPassed);

    RemoveFODGameWidget();

    if (bPracticeDifficultyOverridden)
    {
        GameContext.SessionRequest.Difficulty = SavedSessionDifficulty;
        bPracticeDifficultyOverridden = false;
    }
    // 본게임 StartMiniGame에서 원래 난이도로 다시 계산
    ObstacleFallSpeed = 0.0f;
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