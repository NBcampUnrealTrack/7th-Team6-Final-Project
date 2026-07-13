#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "Components/BoxComponent.h"
#include "PTBLCLogisticBox.h"
#include "PTBLCMiniGame.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPTBLCJudgementEvent, FPTBJudgementResult, Result, FPTBNoteEvent, Note);

UCLASS()
class PARTTIMEBEAT_API APTBLCMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	APTBLCMiniGame();
	
	/** 프레임별 리듬 연출 처리 */
	virtual void Tick(float DeltaTime) override;

	/** 오브젝트 / 상태 구성 */
	virtual void BuildRuntimeState() override;

	/** 채보 이벤트 처리 */
	virtual void HandleChartEvent(FPTBNoteEvent Note) override;

	/** 판정 등록 가능 상태 진입 */
	virtual void HandleNoteArm(FPTBNoteEvent Note) override;

	/** 선행 비주얼 큐 */
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;

	/** 점수 / HUD / SFX 반영 */
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	
	/** 판정 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|LC|Events")
	FPTBLCJudgementEvent OnJudgement;

protected:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|CollisionBox")
	TObjectPtr<UBoxComponent> CollisionBox;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Spawn")
	FVector BoxSpawnLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Spawn")
	TSubclassOf<APTBLCLogisticBox> LogisticBoxClass;
	
	/** 지연 발행된 Cue의 스폰 위치 보정 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Spawn")
	bool bUseCueSpawnDelayCompensation = true;
	
	/** 한 박스당 최대 스폰 지연 보정 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Spawn", meta = (ClampMin = "0", UIMin = "0"))
	float MaxCueSpawnCompensationMs = 250.0f;
	
	/** Beat 기준 둠칫 효과 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse")
	bool bEnableBeatPulse = true;
	
	/** 둠칫 시 최소 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseMinScale = 0.88f;
	
	/** 줄어드는 데 사용하는 Beat 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseShrinkBeatRatio = 0.10f;
	
	/** 복원하는 데 사용하는 Beat 비율. 기본값은 줄어드는 시간의 3배입니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseRecoverBeatRatio = 0.30f;
	
	/** 줄어드는 구간 Ease 지수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float BeatPulseShrinkEaseExponent = 2.5f;
	
	/** 복원 구간 Ease 지수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float BeatPulseRecoverEaseExponent = 3.0f;

private:
	bool IsLogisticBoxAction(EPTBActionType ActionType) const;

	float CalculateScheduledCueTimeMs(const FPTBNoteEvent& Note) const;
	
	void ApplyCueSpawnDelayCompensation(APTBLCLogisticBox* LogisticBox, const FPTBNoteEvent& Note) const;
	
	float CalculateBeatPulseScale(float BeatPhase) const;
	
	void ApplyBeatPulseToBoxes(float PulseScale);
	
	UFUNCTION()
	void HandleLogisticBoxDestroyed(AActor* DestroyedActor);
	
	void PrepareLogisticBoxPool();

	APTBLCLogisticBox* AcquireLogisticBoxFromPool();

	FVector GetLogisticBoxStandbyLocation() const;

	TArray<TWeakObjectPtr<APTBLCLogisticBox>> ActiveLogisticBoxes;

	UPROPERTY()
	TArray<TObjectPtr<APTBLCLogisticBox>> LogisticBoxPool;

	int32 NextLogisticBoxPoolIndex = 0;
};
