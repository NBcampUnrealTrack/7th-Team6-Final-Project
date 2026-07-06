#include "PTBLatencyCalibrationWidget.h"
#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelSlot.h"
#include "Components/Border.h"
#include "Blueprint/WidgetTree.h"

void UPTBLatencyCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetKeyboardFocus();

	if (JudgmentLineWidget)
	{
		if (UCanvasPanelSlot* LineSlot = Cast<UCanvasPanelSlot>(JudgmentLineWidget->Slot))
		{
			LineSlot->SetAnchors(FAnchors(JudgmentLineAnchorX, 0.f, JudgmentLineAnchorX, 1.f));
			LineSlot->SetAlignment(FVector2D(0.5f, 0.f));
		}
	}
}

void UPTBLatencyCalibrationWidget::StartCalibration()
{
	bIsCalibrationActive = true;

	BeatIntervalSec = 60.0f / BPM;
	NoteTravelTimeSec = BeatIntervalSec * NoteLeadBeats;

	BeatOffsetsMs.Reset();
	CurrentPressCount = 0;
	SessionStartTime = GetWorld()->GetTimeSeconds();

	for (FPTBCalibrationNote& Note : ActiveNotes)
	{
		if (Note.Widget)
		{
			Note.Widget->RemoveFromParent();
		}
	}
	ActiveNotes.Reset();

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
	SpawnNote();
	OnMetronomeTick();
}

void UPTBLatencyCalibrationWidget::SpawnNote()
{
	if (!NoteTrackCanvas)
	{
		return;
	}

	UBorder* NoteWidget = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	NoteWidget->SetBrushColor(FLinearColor(0.1f, 0.8f, 1.0f, 1.0f));

	if (UCanvasPanelSlot* NoteSlot = NoteTrackCanvas->AddChildToCanvas(NoteWidget))
	{
		// 오른쪽 끝(anchorX=1.0)에서 스폰
		NoteSlot->SetAnchors(FAnchors(1.0f, 0.5f, 1.0f, 0.5f));
		NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NoteSlot->SetPosition(FVector2D(0.f, 0.f));
		NoteSlot->SetSize(FVector2D(NoteSizePx, NoteSizePx));
	}

	FPTBCalibrationNote NewNote;
	NewNote.Widget = NoteWidget;
	NewNote.SpawnTime = GetWorld()->GetTimeSeconds();
	ActiveNotes.Add(NewNote);
}

void UPTBLatencyCalibrationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!NoteTrackCanvas || NoteTravelTimeSec <= 0.f)
	{
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();

	for (int32 i = ActiveNotes.Num() - 1; i >= 0; --i)
	{
		FPTBCalibrationNote& Note = ActiveNotes[i];
		if (!Note.Widget)
		{
			ActiveNotes.RemoveAt(i);
			continue;
		}

		const float Elapsed = static_cast<float>(Now - Note.SpawnTime);
		const float Alpha = Elapsed / NoteTravelTimeSec;

		// 스폰 지점(1.0) → 판정선(JudgmentLineAnchorX)로 선형 보간
		const float CurrentAnchorX = FMath::Lerp(1.0f, JudgmentLineAnchorX, FMath::Clamp(Alpha, 0.f, 1.5f));

		if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note.Widget->Slot))
		{
			NoteSlot->SetAnchors(FAnchors(CurrentAnchorX, 0.5f, CurrentAnchorX, 0.5f));
		}

		// 판정선을 한참 지나치면 제거 (놓친 노트)
		if (Alpha > 1.5f)
		{
			Note.Widget->RemoveFromParent();
			ActiveNotes.RemoveAt(i);
		}
	}
}

FReply UPTBLatencyCalibrationWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!bIsCalibrationActive)
	{
		return FReply::Unhandled();
	}

	if (InKeyEvent.GetKey() == TargetKey && !InKeyEvent.IsRepeat())
	{
		const double CurrentTime = GetWorld()->GetTimeSeconds();
		const double ElapsedSec = CurrentTime - SessionStartTime;

		const int32 NearestBeatIndex = FMath::RoundToInt(ElapsedSec / BeatIntervalSec);
		const double NearestBeatTimeSec = NearestBeatIndex * BeatIntervalSec;
		const float OffsetMs = static_cast<float>((ElapsedSec - NearestBeatTimeSec) * 1000.0);

		BeatOffsetsMs.Add(OffsetMs);
		CurrentPressCount++;

		const FString OffsetStr = FString::Printf(TEXT("%+.0fms"), OffsetMs);
		OnOffsetMsUpdated(FText::FromString(OffsetStr));

		OnJudgementFlash(FMath::Abs(OffsetMs) < 100.f);

		// 판정선에 가장 가까운 노트를 소모
		if (ActiveNotes.Num() > 0)
		{
			int32 ClosestIndex = 0;
			float ClosestDist = TNumericLimits<float>::Max();
			for (int32 i = 0; i < ActiveNotes.Num(); ++i)
			{
				const float Elapsed = static_cast<float>(CurrentTime - ActiveNotes[i].SpawnTime);
				const float Alpha = Elapsed / NoteTravelTimeSec;
				const float Dist = FMath::Abs(Alpha - 1.0f);
				if (Dist < ClosestDist)
				{
					ClosestDist = Dist;
					ClosestIndex = i;
				}
			}

			if (ActiveNotes[ClosestIndex].Widget)
			{
				ActiveNotes[ClosestIndex].Widget->RemoveFromParent();
			}
			ActiveNotes.RemoveAt(ClosestIndex);
		}

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
	bIsCalibrationActive = false; 

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

	const FString AverageStr = FString::Printf(TEXT("%+.1fms"), AverageOffsetMs);
	OnCalibrationFinishedDisplay(FText::FromString(AverageStr));

	OnCalibrationFinished.Broadcast(AverageOffsetMs);
}