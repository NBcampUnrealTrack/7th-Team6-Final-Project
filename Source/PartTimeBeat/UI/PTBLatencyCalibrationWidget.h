#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "PTBLatencyCalibrationWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalibrationFinished, float, AverageOffsetMs);

UCLASS()
class PARTTIMEBEAT_API UPTBLatencyCalibrationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float BPM = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	FKey TargetKey = EKeys::SpaceBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	int32 RequiredPressCount = 8;

	UPROPERTY(BlueprintAssignable, Category = "Calibration")
	FOnCalibrationFinished OnCalibrationFinished;

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void StartCalibration();

protected:
	// WBP에서 구현: 비주얼/오디오 큐(메트로놈 플래시 등)
	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnMetronomeTick();

private:
	void MetronomeTick();
	void FinishCalibration();

	float BeatIntervalSec = 0.f;
	int32 CurrentPressCount = 0;
	double SessionStartTime = 0.0;
	TArray<float> BeatOffsetsMs;
	FTimerHandle MetronomeTimerHandle;
};