#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBDodgeMiniGame.generated.h"

/**
 * 리듬 피하기 미니게임
 * 위에서 내려오는 장애물을 점프로 피하는 게임
 * 맞추면 점수 증가, 맞으면 체력 감소
 */
UCLASS()
class PARTTIMEBEAT_API APTBDodgeMiniGame : public APTBBaseMiniGame
{
    GENERATED_BODY()

public:
    APTBDodgeMiniGame();

protected:
    virtual void BeginPlay() override;

public:
    // 현재 점수 (최대 제한 없음)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 CurrentScore = 0;

    // 현재 체력 (100에서 시작, 0이면 사망)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 Health = 100;

    // 피한 장애물 수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 DodgeCount = 0;

    // 맞은 장애물 수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    int32 HitCount = 0;

    // 점프 중 여부
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    bool bIsJumping = false;

    // 장애물 낙하 속도 (StartMiniGame 이후 계산)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Dodge")
    float ObstacleFallSpeed = 0.0f;

    // 최소 반응 시간 (150ms 이하로 내려가지 않음)
    static constexpr float MinReactionTimeMs = 150.0f;

    // 베이스 클래스 오버라이드
    virtual void BuildRuntimeState() override;
    virtual void StartMiniGame() override;
    virtual void HandleNoteCue(FPTBNoteEvent Note) override;
    virtual void HandleNoteArm(FPTBNoteEvent Note) override;
    virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
    virtual FPTBMiniGameResultPayload BuildResultPayload() const override;

    // 점프 입력
    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    void OnJumpInput();

    // 난이도별 점수 배율
    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    int32 GetScoreMultiplier() const;

    // BPM 기반 장애물 속도 계산
    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    float CalculateObstacleFallSpeed(float BPM) const;

    // 난이도별 LookAhead 시간
    UFUNCTION(BlueprintCallable, Category = "PTB|Dodge")
    float GetLookAheadMsByDifficulty() const;

    // Blueprint 에서 장애물 스폰 연출 구현
    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnObstacleSpawn(float FallSpeed, float BeatTime);

    // Blueprint 에서 장애물 판정 진입 연출 구현
    UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Dodge")
    void OnObstacleArmed(float BeatTime);
};