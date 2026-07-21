#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PTBLCBeatPulseComponent.generated.h"

class APTBLCMiniGame;
class USceneComponent;

/**
 * LC 미니게임 배경/소품에 박자 기준 둠칫 스케일을 적용하는 컴포넌트입니다.
 */
UCLASS(ClassGroup=(PTB), meta=(BlueprintSpawnableComponent))
class PARTTIMEBEAT_API UPTBLCBeatPulseComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBLCBeatPulseComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 박자를 가져올 LC 미니게임. 비워두면 BeginPlay에서 월드의 LC 미니게임을 찾습니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse")
	TObjectPtr<APTBLCMiniGame> SourceMiniGame;

	/** 스케일을 적용할 컴포넌트. 비워두면 Owner의 RootComponent를 사용합니다. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse")
	TObjectPtr<USceneComponent> TargetComponent;

	/** 자동으로 월드의 LC 미니게임을 찾을지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse")
	bool bAutoFindSourceMiniGame = true;

	/** 둠칫 효과 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse")
	bool bEnableBeatPulse = true;

	/** 둠칫 시 최소 스케일 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.0", UIMin = "0.0", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseMinScale = 0.88f;

	/** 줄어드는 데 사용하는 Beat 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseShrinkBeatRatio = 0.10f;

	/** 복원하는 데 사용하는 Beat 비율 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.01", UIMin = "0.01", ClampMax = "1.0", UIMax = "1.0"))
	float BeatPulseRecoverBeatRatio = 0.30f;

	/** 줄어드는 구간 Ease 지수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float BeatPulseShrinkEaseExponent = 2.5f;

	/** 복원 구간 Ease 지수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|LC|Beat Pulse", meta = (ClampMin = "0.1", UIMin = "0.1"))
	float BeatPulseRecoverEaseExponent = 3.0f;

private:
	APTBLCMiniGame* FindSourceMiniGame() const;
	USceneComponent* ResolveTargetComponent() const;
	float CalculateBeatPulseScale(float BeatPhase) const;
	bool TryGetBeatPhase(float& OutBeatPhase) const;

	FVector BaseRelativeScale = FVector::OneVector;
};
