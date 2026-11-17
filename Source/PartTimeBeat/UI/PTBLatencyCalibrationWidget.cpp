#include "PTBLatencyCalibrationWidget.h"
#include "Components/Widget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "HAL/PlatformTime.h"
#include "Core/PTBGameInstance.h"
#include "Core/PTBInputPollSubsystem.h"

#define LOCTEXT_NAMESPACE "PTBLatencyCalibration"

namespace PTBCalibrationInternal
{
	// 판정선을 지난 뒤 이 비율만큼 더 이동하며 사라짐
	constexpr float NoteOvershootRatio = 0.3f;
}

void UPTBLatencyCalibrationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetIsFocusable(true);
	SetKeyboardFocus();

	PlaceJudgmentLine();

	if (ButtonRestart) ButtonRestart->OnClicked.AddUniqueDynamic(this, &UPTBLatencyCalibrationWidget::HandleRestartClicked);
	if (ButtonApply)   ButtonApply->OnClicked.AddUniqueDynamic(this, &UPTBLatencyCalibrationWidget::HandleApplyClicked);
	if (ButtonCancel)  ButtonCancel->OnClicked.AddUniqueDynamic(this, &UPTBLatencyCalibrationWidget::HandleCancelClicked);

	RefreshKeyText();
	SetState(State);
}

// ── 상태 ─────────────────────────────────────────────────────────

void UPTBLatencyCalibrationWidget::SetState(EPTBCalibrationState NewState)
{
	State = NewState;

	if (ButtonApply)
	{
		// Hidden: 자리는 유지 → 결과가 나와도 다른 버튼 위치가 흔들리지 않음
		ButtonApply->SetVisibility(State == EPTBCalibrationState::Result ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}

	LastPulseTime = -1.0;
	ApplyBeatPulse(0.f);

	OnCalibrationStateChanged(State);
}

void UPTBLatencyCalibrationWidget::StartCalibration()
{
	ClearNotes();

	ActiveKey = ResolveTargetKey();
	RefreshKeyText();

	BeatIntervalSec = 60.0 / FMath::Max(BPM, 30.f);
	NoteTravelTimeSec = NoteSpeed > 0.f
		? static_cast<double>(NoteTravelDistance) / NoteSpeed
		: BeatIntervalSec * FMath::Max(NoteLeadBeats, 1);

	// 준비(카운트인) 박자 수: 첫 노트가 판정선까지 오는 시간을 박자 단위로 올림
	const int32 CountInBeats = FMath::Max(1, FMath::CeilToInt(NoteTravelTimeSec / BeatIntervalSec - KINDA_SMALL_NUMBER));

	// 첫 노트가 화면 오른쪽에서 들어와 0번 박자에 판정선에 닿도록 준비 시간 = 노트 이동 시간.
	// 준비 시간 동안에도 소리는 나게 해서(카운트인) 박자를 미리 잡을 수 있게 함.
	const double Now = FPlatformTime::Seconds();
	FirstBeatTime = Now + CountInBeats * BeatIntervalSec;
	NextSoundBeat = -CountInBeats;
	NextNoteBeat = 0;

	BeatOffsetsMs.Reset();
	CurrentPressCount = 0;
	ResultOffsetMs = 0.f;

	// 이전 측정 결과가 화면에 남지 않도록 표시 초기화
	OnOffsetMsUpdated(FText::GetEmpty());
	OnCalibrationFinishedDisplay(FText::GetEmpty());
	OnPressCountUpdated(FText::FromString(FString::Printf(TEXT("%d / %d"), 0, RequiredPressCount)));

	SetState(EPTBCalibrationState::Running);
	SetKeyboardFocus();
}

void UPTBLatencyCalibrationWidget::ApplyCalibrationResult()
{
	if (State != EPTBCalibrationState::Result)
	{
		return;
	}

	if (UPTBGameInstance* PTBGameInstance = GetGameInstance<UPTBGameInstance>())
	{
		PTBGameInstance->SetJudgementOffsetMs(ResultOffsetMs);
		PTBGameInstance->SaveGame();
		UE_LOG(LogTemp, Log, TEXT("[Calibration] 적용: JudgementOffsetMs = %.1f"), ResultOffsetMs);
	}

	OnCalibrationFinished.Broadcast(ResultOffsetMs);

	if (bCloseOnApply)
	{
		CloseCalibration();
	}
	else
	{
		SetState(EPTBCalibrationState::Idle);
	}
}

void UPTBLatencyCalibrationWidget::CloseCalibration()
{
	ClearNotes();
	BeatOffsetsMs.Reset();
	CurrentPressCount = 0;
	SetState(EPTBCalibrationState::Idle);

	// 저장된 JudgementOffsetMs는 건드리지 않음 (적용 버튼으로만 저장)
	RemoveFromParent();
}

float UPTBLatencyCalibrationWidget::GetSavedJudgementOffsetMs() const
{
	if (const UPTBGameInstance* PTBGameInstance = GetGameInstance<UPTBGameInstance>())
	{
		return PTBGameInstance->CachedSettings.JudgementOffsetMs;
	}
	return 0.f;
}

void UPTBLatencyCalibrationWidget::HandleRestartClicked() { StartCalibration(); }
void UPTBLatencyCalibrationWidget::HandleApplyClicked()   { ApplyCalibrationResult(); }
void UPTBLatencyCalibrationWidget::HandleCancelClicked()  { CloseCalibration(); }

// ── 키 ───────────────────────────────────────────────────────────

FKey UPTBLatencyCalibrationWidget::ResolveTargetKey() const
{
	if (bUseActionAKey)
	{
		if (const UPTBGameInstance* PTBGameInstance = GetGameInstance<UPTBGameInstance>())
		{
			const FKey Key = PTBGameInstance->CachedSettings.RhythmKeys.ActionA;
			if (Key.IsValid())
			{
				return Key;
			}
		}
	}
	return TargetKey;
}

FText UPTBLatencyCalibrationWidget::GetTargetKeyDisplayName() const
{
	const FKey Key = ActiveKey.IsValid() ? ActiveKey : ResolveTargetKey();
	return Key.GetDisplayName();
}

void UPTBLatencyCalibrationWidget::RefreshKeyText()
{
	if (!ActiveKey.IsValid())
	{
		ActiveKey = ResolveTargetKey();
	}
	const FText KeyName = ActiveKey.GetDisplayName();
	if (TXT_TargetKey)
	{
		TXT_TargetKey->SetText(KeyName);
	}
	if (TXT_Instruction)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Key"), KeyName);
		Args.Add(TEXT("ActionA"), KeyName);
		TXT_Instruction->SetText(FText::Format(InstructionFormat, Args));
	}
}

double UPTBLatencyCalibrationWidget::ConsumePressTime() const
{
	// 폴링 스레드가 기록한 정확한 눌림 시각이 있으면 사용 (같은 FPlatformTime 시계)
	if (bUseActionAKey)
	{
		if (UPTBInputPollSubsystem* InputPoll = GetGameInstance() ? GetGameInstance()->GetSubsystem<UPTBInputPollSubsystem>() : nullptr)
		{
			double EdgeSeconds = 0.0;
			if (InputPoll->ConsumeEdge(EPTBActionType::ActionA, true, EdgeSeconds))
			{
				return EdgeSeconds;
			}
		}
	}
	return FPlatformTime::Seconds();
}

// ── 입력 ─────────────────────────────────────────────────────────

FReply UPTBLatencyCalibrationWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 버튼에 포커스가 있어도 여기서 먼저 받음 → 스페이스/엔터가 버튼을 눌러버리지 않게 측정 키는 항상 소비
	const FKey Key = InKeyEvent.GetKey();
	if (Key == ActiveKey)
	{
		if (!InKeyEvent.IsRepeat() && State == EPTBCalibrationState::Running)
		{
			RegisterPress(ConsumePressTime());
		}
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UPTBLatencyCalibrationWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CloseCalibration();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPTBLatencyCalibrationWidget::RegisterPress(double PressTime)
{
	const double SinceFirst = PressTime - FirstBeatTime;
	const int32 NearestBeat = FMath::RoundToInt(SinceFirst / BeatIntervalSec);
	if (NearestBeat < 0)
	{
		return; // 카운트인 구간 입력은 무시
	}

	const double NearestBeatTime = FirstBeatTime + NearestBeat * BeatIntervalSec;
	const float OffsetMs = static_cast<float>((PressTime - NearestBeatTime) * 1000.0); // + = 늦게 누름

	BeatOffsetsMs.Add(OffsetMs);
	CurrentPressCount++;

	OnOffsetMsUpdated(FText::FromString(FString::Printf(TEXT("%+d ms"), FMath::RoundToInt(OffsetMs))));
	OnPressCountUpdated(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPressCount, RequiredPressCount)));
	OnJudgementFlash(FMath::Abs(OffsetMs) < 100.f);

	// 그 박자의 노트 제거
	int32 ClosestIndex = INDEX_NONE;
	double ClosestDist = TNumericLimits<double>::Max();
	for (int32 i = 0; i < ActiveNotes.Num(); ++i)
	{
		const double Dist = FMath::Abs(ActiveNotes[i].HitTime - NearestBeatTime);
		if (Dist < ClosestDist)
		{
			ClosestDist = Dist;
			ClosestIndex = i;
		}
	}
	if (ClosestIndex != INDEX_NONE && ClosestDist < BeatIntervalSec * 0.5)
	{
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
}

void UPTBLatencyCalibrationWidget::FinishCalibration()
{
	ClearNotes();

	float AverageMs = 0.f;
	if (BeatOffsetsMs.Num() > 0)
	{
		// 가장 많이 벗어난 값 1개씩(앞/뒤) 제외 → 실수로 한 번 크게 틀려도 결과가 덜 흔들림
		TArray<float> Sorted = BeatOffsetsMs;
		Sorted.Sort();
		int32 Begin = 0;
		int32 End = Sorted.Num();
		if (Sorted.Num() >= 6)
		{
			++Begin;
			--End;
		}
		double Sum = 0.0;
		for (int32 i = Begin; i < End; ++i)
		{
			Sum += Sorted[i];
		}
		AverageMs = static_cast<float>(Sum / FMath::Max(End - Begin, 1));
	}

	// 판정 시각 = 입력 시각 + JudgementOffsetMs 이므로,
	// 평균적으로 늦게 누르면(+) 그만큼 빼줘야(-) 정확해짐 → 부호 반대로 저장
	ResultOffsetMs = FMath::Clamp(static_cast<float>(FMath::RoundToInt(-AverageMs)), -MaxOffsetMs, MaxOffsetMs);

	UE_LOG(LogTemp, Log, TEXT("[Calibration] 측정 완료: 평균 %+.1fms → 보정값 %+.0fms (아직 저장 안 함, 현재 %+.0fms)"),
		AverageMs, ResultOffsetMs, GetSavedJudgementOffsetMs());

	SetState(EPTBCalibrationState::Result);

	OnCalibrationFinishedDisplay(FText::FromString(FString::Printf(TEXT("%+d ms"), FMath::RoundToInt(ResultOffsetMs))));
}

// ── 노트 / 메트로놈 ──────────────────────────────────────────────

void UPTBLatencyCalibrationWidget::SpawnNote(double HitTime)
{
	if (!NoteTrackCanvas || !WidgetTree)
	{
		return;
	}

	UBorder* NoteWidget = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	if (NoteBrush.GetResourceObject())
	{
		NoteWidget->SetBrush(NoteBrush);
		NoteWidget->SetBrushColor(FLinearColor::White);
	}
	else
	{
		NoteWidget->SetBrushColor(NoteColor);
	}
	NoteWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	NoteWidget->SetRenderOpacity(NoteFadeInRatio > 0.f ? 0.f : 1.f);

	if (UCanvasPanelSlot* NoteSlot = NoteTrackCanvas->AddChildToCanvas(NoteWidget))
	{
		NoteSlot->SetAnchors(FAnchors(0.f, 0.5f, 0.f, 0.5f));
		NoteSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NoteSlot->SetSize(FVector2D(NoteSizePx, NoteSizePx));
		NoteSlot->SetPosition(FVector2D(-10000.f, 0.f)); // 첫 Tick에서 위치 계산
	}

	FPTBCalibrationNote NewNote;
	NewNote.Widget = NoteWidget;
	NewNote.HitTime = HitTime;
	ActiveNotes.Add(NewNote);
}

bool UPTBLatencyCalibrationWidget::IsSameOrDescendant(const UWidget* Child, const UWidget* Ancestor)
{
	for (const UWidget* It = Child; It; It = It->GetParent())
	{
		if (It == Ancestor)
		{
			return true;
		}
	}
	return false;
}

void UPTBLatencyCalibrationWidget::PlaceJudgmentLine()
{
	if (!JudgmentLineWidget || !NoteTrackCanvas || !bAutoPlaceJudgmentLine)
	{
		return;
	}

	// 판정선이 Overlay 등으로 묶여 있으면, NoteTrackCanvas 바로 아래의 묶음 위젯을 배치
	UWidget* Root = JudgmentLineWidget;
	while (Root && Root->GetParent() && Root->GetParent() != NoteTrackCanvas)
	{
		Root = Root->GetParent();
	}

	UCanvasPanelSlot* RootSlot = Root ? Cast<UCanvasPanelSlot>(Root->Slot) : nullptr;
	if (!RootSlot || Root->GetParent() != NoteTrackCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Calibration] JudgmentLineWidget이 NoteTrackCanvas 안에 없어서 자동 배치를 건너뜀"));
		return;
	}

	if (bJudgmentLineMatchesNoteSize)
	{
		// 노트와 같은 네모: 노트와 같은 기준(트랙 세로 중앙)으로 배치
		RootSlot->SetAutoSize(false);
		RootSlot->SetAnchors(FAnchors(JudgmentLineAnchorX, 0.5f, JudgmentLineAnchorX, 0.5f));
		RootSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		RootSlot->SetPosition(FVector2D::ZeroVector);
		RootSlot->SetSize(FVector2D(NoteSizePx, NoteSizePx));
	}
	else
	{
		// 세로로 꽉 채우고, 가로 중앙이 정확히 AnchorX에 오도록 (Designer의 X 오프셋 제거)
		const FVector2D RootSize = RootSlot->GetSize();
		RootSlot->SetAnchors(FAnchors(JudgmentLineAnchorX, 0.f, JudgmentLineAnchorX, 1.f));
		RootSlot->SetAlignment(FVector2D(0.5f, 0.f));
		RootSlot->SetOffsets(FMargin(0.f, 0.f, RootSize.X, 0.f));
	}

	JudgmentLineWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	Root->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
}

void UPTBLatencyCalibrationWidget::ApplyBeatPulse(float Pulse)
{
	auto ScaleOf = [Pulse](float MaxScale) { return FMath::Lerp(1.f, MaxScale, Pulse); };

	if (UWidget* KeyWidget = KeyPulseTarget ? KeyPulseTarget.Get() : static_cast<UWidget*>(TXT_TargetKey.Get()))
	{
		KeyWidget->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
		const float S = ScaleOf(KeyPulseScale);
		KeyWidget->SetRenderScale(FVector2D(S, S));
	}

	// 판정선이 KeyPulseTarget 묶음 안에 있으면 묶음이 이미 커지므로 따로 키우지 않음 (두 번 커짐 방지)
	if (JudgmentLineWidget && !IsSameOrDescendant(JudgmentLineWidget, KeyPulseTarget))
	{
		const float S = bPulseJudgmentLine ? ScaleOf(JudgmentLinePulseScale) : 1.f;
		JudgmentLineWidget->SetRenderScale(FVector2D(S, S));
	}

	const float NoteScale = bPulseNotes ? ScaleOf(NotePulseScale) : 1.f;
	for (const FPTBCalibrationNote& Note : ActiveNotes)
	{
		if (Note.Widget)
		{
			Note.Widget->SetRenderScale(FVector2D(NoteScale, NoteScale));
		}
	}
}

float UPTBLatencyCalibrationWidget::ResolveJudgmentLineX() const
{
	const FGeometry& CanvasGeometry = NoteTrackCanvas->GetCachedGeometry();

	// 판정선 위젯이 있으면 실제로 그려진 위치의 가로 중앙을 사용
	// (다른 패널 안에 있거나 Designer 오프셋이 있어도 화면에 보이는 선과 정확히 맞음)
	if (JudgmentLineWidget && JudgmentLineWidget->IsVisible())
	{
		const FGeometry& LineGeometry = JudgmentLineWidget->GetCachedGeometry();
		if (LineGeometry.GetLocalSize().X > 0.f || LineGeometry.GetLocalSize().Y > 0.f)
		{
			const FVector2D LineCenterAbs = LineGeometry.LocalToAbsolute(LineGeometry.GetLocalSize() * 0.5f);
			return static_cast<float>(CanvasGeometry.AbsoluteToLocal(LineCenterAbs).X);
		}
	}

	return static_cast<float>(CanvasGeometry.GetLocalSize().X) * JudgmentLineAnchorX;
}

void UPTBLatencyCalibrationWidget::ClearNotes()
{
	for (FPTBCalibrationNote& Note : ActiveNotes)
	{
		if (Note.Widget)
		{
			Note.Widget->RemoveFromParent();
		}
	}
	ActiveNotes.Reset();
}

void UPTBLatencyCalibrationWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const double Now = FPlatformTime::Seconds();

	// 박자 펄스 (키 표시 / 판정선 / 노트): 박자 순간 1 → KeyPulseDuration 동안 0으로
	float Pulse = 0.f;
	if (LastPulseTime > 0.0 && KeyPulseDuration > 0.f)
	{
		const float T = FMath::Clamp(static_cast<float>((Now - LastPulseTime) / KeyPulseDuration), 0.f, 1.f);
		Pulse = 1.f - FMath::InterpEaseOut(0.f, 1.f, T, 2.f);
		if (T >= 1.f)
		{
			LastPulseTime = -1.0;
		}
	}
	ApplyBeatPulse(Pulse);

	if (State != EPTBCalibrationState::Running || BeatIntervalSec <= 0.0)
	{
		return;
	}

	// 메트로놈: 박자 시각이 지나면 소리 (일시정지 중에도 나도록 UI 사운드로 재생)
	while (Now >= FirstBeatTime + NextSoundBeat * BeatIntervalSec)
	{
		if (MetronomeSound)
		{
			UGameplayStatics::PlaySound2D(this, MetronomeSound, 1.f, 1.f, 0.f, nullptr, nullptr, /*bIsUISound*/ true);
		}
		LastPulseTime = Now;
		OnMetronomeTick();
		++NextSoundBeat;
	}

	// 노트 생성: 판정선 도착(HitTime)보다 이동 시간만큼 먼저
	while (Now >= FirstBeatTime + NextNoteBeat * BeatIntervalSec - NoteTravelTimeSec)
	{
		SpawnNote(FirstBeatTime + NextNoteBeat * BeatIntervalSec);
		++NextNoteBeat;
	}

	if (!NoteTrackCanvas || NoteTravelTimeSec <= 0.0)
	{
		return;
	}

	const float LineX = ResolveJudgmentLineX();
	// 노트 중앙 기준 위치 보정: HitPoint가 판정선에 올 때 중앙은 이만큼 오른쪽
	const float HitPointShift = NoteSizePx * (0.5f - NoteHitPointX);

	for (int32 i = ActiveNotes.Num() - 1; i >= 0; --i)
	{
		FPTBCalibrationNote& Note = ActiveNotes[i];
		if (!Note.Widget)
		{
			ActiveNotes.RemoveAt(i);
			continue;
		}

		// 0 = 생성 지점, 1 = 판정선
		const float Alpha = 1.f - static_cast<float>((Note.HitTime - Now) / NoteTravelTimeSec);

		if (Alpha > 1.f + PTBCalibrationInternal::NoteOvershootRatio)
		{
			Note.Widget->RemoveFromParent();
			ActiveNotes.RemoveAt(i);
			continue;
		}

		const float X = LineX + NoteTravelDistance * (1.f - Alpha) + HitPointShift;
		if (UCanvasPanelSlot* NoteSlot = Cast<UCanvasPanelSlot>(Note.Widget->Slot))
		{
			NoteSlot->SetPosition(FVector2D(X, 0.f));
		}

		float Opacity = 1.f;
		if (NoteFadeInRatio > 0.f && Alpha < NoteFadeInRatio)
		{
			Opacity = FMath::Clamp(Alpha / NoteFadeInRatio, 0.f, 1.f);
		}
		else if (Alpha > 1.f)
		{
			Opacity = 1.f - (Alpha - 1.f) / PTBCalibrationInternal::NoteOvershootRatio;
		}
		Note.Widget->SetRenderOpacity(Opacity);
	}
}

#undef LOCTEXT_NAMESPACE
