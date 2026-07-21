#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InputCoreTypes.h"
#include "PTBLatencyCalibrationWidget.generated.h"

class UCanvasPanel;
class UBorder;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCalibrationFinished, float, AverageOffsetMs);

USTRUCT()
struct FPTBCalibrationNote
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UBorder> Widget = nullptr;

	double SpawnTime = 0.0;
};

UCLASS()
class PARTTIMEBEAT_API UPTBLatencyCalibrationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float BPM = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	FKey TargetKey = EKeys::SpaceBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	int32 RequiredPressCount = 8;

	// 노트가 판정선까지 도달하는 데 걸리는 박자 수 (클수록 화면 오른쪽에서 천천히 옴)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	int32 NoteLeadBeats = 2;

	// 판정선 X 위치 (0=왼쪽 끝, 1=오른쪽 끝) - 화면 비율 기준이라 반응형
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float JudgmentLineAnchorX = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	float NoteSizePx = 40.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calibration")
	TObjectPtr<USoundBase> MetronomeSound;

	UPROPERTY(BlueprintAssignable, Category = "Calibration")
	FOnCalibrationFinished OnCalibrationFinished;

	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void StartCalibration();

	// 위젯을 닫을 때 호출 - 노트/카운트는 초기화되지만 GameInstance에 저장된 값은 건드리지 않음
	UFUNCTION(BlueprintCallable, Category = "Calibration")
	void CloseCalibration();

	// GameInstance(CachedSettings.JudgementOffsetMs)에 저장된 마지막 값을 조회
	UFUNCTION(BlueprintPure, Category = "Calibration")
	float GetSavedJudgementOffsetMs() const;

protected:
	// WBP Designer에 이 이름과 타입(Canvas Panel)으로 위젯을 만들어야 바인딩됨
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> NoteTrackCanvas;

	// 선택: 판정선 표시용 위젯 (Border/Image), 만들어두면 위치 자동 동기화
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidget> JudgmentLineWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnMetronomeTick();

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnJudgementFlash(bool bGoodHit);

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnOffsetMsUpdated(const FText& FormattedOffset);

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnPressCountUpdated(const FText& FormattedCount);

	UFUNCTION(BlueprintImplementableEvent, Category = "Calibration")
	void OnCalibrationFinishedDisplay(const FText& FormattedAverage);

private:
	void MetronomeTick();
	void SpawnNote();
	void FinishCalibration();

	float BeatIntervalSec = 0.f;
	float NoteTravelTimeSec = 0.f;
	int32 CurrentPressCount = 0;
	double SessionStartTime = 0.0;
	TArray<float> BeatOffsetsMs;
	TArray<FPTBCalibrationNote> ActiveNotes;
	FTimerHandle MetronomeTimerHandle;

	bool bIsCalibrationActive = false;
};