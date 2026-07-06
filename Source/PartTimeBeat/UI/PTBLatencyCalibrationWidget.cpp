#include "PTBLatencyCalibrationWidget.h"

void UPTBLatencyCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetKeyboardFocus();
}

void UPTBLatencyCalibrationWidget::StartCalibration()
{
	BeatIntervalSec = 60.0f / BPM;
	BeatOffsetsMs.Reset();
	CurrentPressCount = 0;
	SessionStartTime = GetWorld()->GetTimeSeconds();

	GetWorld()->GetTimerManager().SetTimer(
		MetronomeTimerHandle,
		this,
		&UPTBLatencyCalibrationWidget::MetronomeTick,
		BeatIntervalSec,
		true
	);
}

void UPTBLatencyCalibrationWidget::MetronomeTick()
{
	OnMetronomeTick();
}

FReply UPTBLatencyCalibrationWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == TargetKey && !InKeyEvent.IsRepeat())
	{
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		const double ElapsedSec = CurrentTime - SessionStartTime;

		const int32 NearestBeatIndex = FMath::RoundToInt(ElapsedSec / BeatIntervalSec);
		const double NearestBeatTimeSec = NearestBeatIndex * BeatIntervalSec;
		const float OffsetMs = static_cast<float>((ElapsedSec - NearestBeatTimeSec) * 1000.0);

		BeatOffsetsMs.Add(OffsetMs);
		CurrentPressCount++;

		if (CurrentPressCount >= RequiredPressCount)
		{
			FinishCalibration();
		}

		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UPTBLatencyCalibrationWidget::FinishCalibration()
{
	GetWorld()->GetTimerManager().ClearTimer(MetronomeTimerHandle);

	float AverageOffsetMs = 0.f;
	if (BeatOffsetsMs.Num() > 0)
	{
		float Sum = 0.f;
		for (const float Offset : BeatOffsetsMs)
		{
			Sum += Offset;
		}
		AverageOffsetMs = Sum / BeatOffsetsMs.Num();
	}

	OnCalibrationFinished.Broadcast(AverageOffsetMs);
}