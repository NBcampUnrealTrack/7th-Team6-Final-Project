#pragma once
#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "PTBDodgeMiniGame.generated.h"

UCLASS()
class PARTTIMEBEAT_API APTBDodgeMiniGame : public APTBBaseMiniGame
{
    GENERATED_BODY()
public:
    APTBDodgeMiniGame();
protected:
    virtual void BeginPlay() override;
public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 CurrentScore = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float Health = 100.0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 DodgeCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 HitCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    bool bIsJumping = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float ObstacleFallSpeed = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 HighPerfectStreak = 0;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    bool bFeverMode = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float ScoreMultiplier = 1.0f;

    static constexpr float MinReactionTimeMs = 150.0f;

    virtual void BuildRuntimeState() override;
    virtual void StartMiniGame() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
    virtual float ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const override;

    /** 튜토리얼 연습 시작/종료로 채보가 바뀔 때 HP·카운트·피버 초기화 (연습 결과가 본게임에 남지 않게) */
    virtual void HandleRoundChartChanged() override;

    /**
     * 튜토리얼 연습 시작: 연습 패턴에 쓰인 키로 연습용 UI 난이도를 고름
     * (A·B만 → Easy, C·D 포함 → Standard, E 포함 → Insane).
     * 연습 동안만 SessionRequest.Difficulty를 바꾼 뒤 BP의 OnMiniGameReady를 호출해 그 난이도의 위젯을 만듦.
     */
    virtual void ReceiveTutorialPracticeStarted_Implementation() override;

    /** 튜토리얼 연습 종료: 연습용 위젯 제거 + 원래 난이도 복구 → 본게임 시작 때 BP가 원래 난이도 위젯을 새로 만듦 */
    virtual void ReceiveTutorialPracticeFinished_Implementation(bool bPassed) override;

private:
    /** 연습 채보에 쓰인 가장 높은 키 기준으로 연습용 UI 난이도 결정 */
    EPTBDifficulty ResolvePracticeUIDifficulty() const;

    /** BP 변수 FODGameWidget(현재 게임 위젯) 프로퍼티 (변수 이름 끝 공백 포함/미포함 둘 다 찾음) */
    FObjectProperty* FindFODGameWidgetProperty() const;

    /** BP 변수 FODGameWidget(현재 게임 위젯)을 화면에서 내리고 비움 */
    void RemoveFODGameWidget();

    /** FODGameWidget이 비어 있으면 BP의 OnMiniGameReady를 호출해 현재 난이도 위젯을 만듦 */
    void EnsureFODGameWidget(const TCHAR* Reason);

    /** 연습 동안 바꿔둔 원래 난이도 */
    EPTBDifficulty SavedSessionDifficulty = EPTBDifficulty::Easy;
    bool bPracticeDifficultyOverridden = false;
public:

    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    virtual void InitializeMiniGame(const FPTBMiniGameContext& Context) override;

    virtual FPTBMiniGameResultPayload BuildResultPayload() const override;

    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    int32 GetScoreMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    float CalculateObstacleFallSpeed(float BPM) const;

    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    float GetLookAheadMsByDifficulty() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnObstacleSpawn(float FallSpeed, float BeatTime, int32 LaneIndex);

    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnObstacleArmed(float BeatTime);

    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnScoreUpdated(int32 NewScore);

    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnJudgementUpdated(EPTBJudgementType JudgementType);

    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnFeverModeChanged(bool bFever);
};