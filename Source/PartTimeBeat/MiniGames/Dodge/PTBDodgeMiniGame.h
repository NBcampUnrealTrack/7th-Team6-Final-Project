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
    int32 Health = 100;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 DodgeCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 HitCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    bool bIsJumping = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float ObstacleFallSpeed = 0.0f;

    static constexpr float MinReactionTimeMs = 150.0f;

    virtual void BuildRuntimeState() override;
    virtual void StartMiniGame() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
    virtual float ResolveInputOffsetMs(const FPTBMiniGameContext& Context) const override;

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
};