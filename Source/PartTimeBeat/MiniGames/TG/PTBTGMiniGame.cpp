#include "MiniGames/TG/PTBTGMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "Engine/Engine.h"
#include "MiniGames/TG/PTBTGMiniGameRuleSet.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"

// ── 화면 타이밍 디버그 헬퍼 ──────────────────────────────────────

namespace PTBTGDebug
{
	static bool bEnabled = true;  // false로 바꾸면 전체 비활성

	static FString KeyLabel(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return TEXT("Z");
		case EPTBActionType::ActionB: return TEXT("X");
		case EPTBActionType::ActionC: return TEXT("C");
		case EPTBActionType::ActionD: return TEXT("V");
		case EPTBActionType::ActionE: return TEXT("B");
		default:                      return TEXT("?");
		}
	}

	static FColor JudgementColor(EPTBJudgementType Type)
	{
		switch (Type)
		{
		case EPTBJudgementType::HighPerfect: return FColor(255, 215, 0);   // 금색
		case EPTBJudgementType::Perfect:     return FColor(255, 105, 180); // 분홍
		case EPTBJudgementType::Good:        return FColor(100, 220, 100); // 초록
		default:                             return FColor::Red;
		}
	}

	static FString JudgementName(EPTBJudgementType Type)
	{
		switch (Type)
		{
		case EPTBJudgementType::HighPerfect: return TEXT("HighPerfect ★");
		case EPTBJudgementType::Perfect:     return TEXT("Perfect");
		case EPTBJudgementType::Good:        return TEXT("Good");
		default:                             return TEXT("Miss");
		}
	}

	static void Screen(int32 Key, float Duration, FColor Color, const FString& Msg)
	{
		if (bEnabled && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(Key, Duration, Color, Msg);
		}
	}
}

// ─────────────────────────────────────────────────────────────────

void APTBTGMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	CueCount = 0;
	ArmCount = 0;
	NoteEventCount = 0;
	LongNoteCueCount = 0;
	JudgementCount = 0;
	CuedNotes.Reset();
	ArmedNotes.Reset();
	ReachedNotes.Reset();

	UE_LOG(LogPTBMiniGames, Log, TEXT("[%s] TG BuildRuntimeState RuleSet=%s Chart=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()));
}

void APTBTGMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);

	++NoteEventCount;
	TrackNote(ReachedNotes, Note);
	OnTGNoteReached.Broadcast(Note);

	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	if (!TGRuleSet || TGRuleSet->bLogNoteEvent)
	{
		LogNoteDebug(TEXT("NoteEvent"), Note);
	}
}

void APTBTGMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);

	++ArmCount;
	TrackNote(ArmedNotes, Note);
	OnTGNoteArm.Broadcast(Note);

	// ── [Arm] 판정 가능 구간 시작 ───────────────────────────────
	{
		const float ArmMs = (RuleSet && RuleSet->bUseArmLeadTimeOverride)
			? RuleSet->ArmLeadTimeMsOverride
			: 120.0f;
		const float WindowSec = (ArmMs + 120.0f) / 1000.0f;
		const FString Type = Note.bIsLongNote ? TEXT("Hold") : TEXT("Tap");
		PTBTGDebug::Screen(
			Note.NoteId,
			WindowSec,
			FColor::Yellow,
			FString::Printf(TEXT("[%s %s] 지금부터 누를 수 있음 | ±120ms 창"),
				*Type, *PTBTGDebug::KeyLabel(Note.ActionType)));

		constexpr float PreNowOffsetMs = 100.0f;
		const float DelayToPreNow = FMath::Max(0.0f, (ArmMs - PreNowOffsetMs) / 1000.0f);
		const EPTBActionType CapturedAction = Note.ActionType;
		if (UWorld* World = GetWorld())
		{
			FTimerHandle NowHandle;
			World->GetTimerManager().SetTimer(
				NowHandle,
				FTimerDelegate::CreateWeakLambda(this, [this, CapturedAction]()
				{
					PTBTGDebug::Screen(3000, 0.4f, FColor::White,
						FString::Printf(TEXT("★ 지금! [%s]  Perfect: ±50ms / Good: ±70ms"),
							*PTBTGDebug::KeyLabel(CapturedAction)));
				}),
				DelayToPreNow, false);
		}
	}

	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	if (!TGRuleSet || TGRuleSet->bLogNoteArm)
	{
		LogNoteDebug(TEXT("NoteArm"), Note);
	}
}

void APTBTGMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	++CueCount;
	if (Note.bIsLongNote)
	{
		++LongNoteCueCount;
	}

	TrackNote(CuedNotes, Note);
	OnTGNoteCue.Broadcast(Note);

	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	if (!TGRuleSet || TGRuleSet->bLogNoteCue)
	{
		LogNoteDebug(TEXT("NoteCue"), Note);
	}
}

void APTBTGMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	FPTBNoteEvent JudgedNote;
	const bool bHasJudgedNote = FindTrackedNote(Result.NoteId, JudgedNote);

	// ── [판정 결과] 색상·오차 표시 ────────────────────────────────
	if (Result.Reason != EPTBJudgementReason::EmptyInput)
	{
		FString ReasonSuffix;
		switch (Result.Reason)
		{
		case EPTBJudgementReason::ExpiredNote:  ReasonSuffix = TEXT(" (놓침)");   break;
		case EPTBJudgementReason::EarlyRelease: ReasonSuffix = TEXT(" (조기해제)"); break;
		default: break;
		}

		const bool bIsHit = (Result.JudgementType != EPTBJudgementType::Miss);
		const FString DeltaSign = (Result.DeltaMs > 0.f) ? TEXT("+") : TEXT("");

		PTBTGDebug::Screen(
			2000,
			2.0f,
			PTBTGDebug::JudgementColor(Result.JudgementType),
			FString::Printf(TEXT("[판정] %s%s | 오차: %s%.1fms"),
				*PTBTGDebug::JudgementName(Result.JudgementType),
				*ReasonSuffix,
				*DeltaSign, Result.DeltaMs));

		// Arm 메시지 지우기 (판정 완료 시)
		if (bIsHit && GEngine)
		{
			GEngine->RemoveOnScreenDebugMessage(Result.NoteId);
			GEngine->RemoveOnScreenDebugMessage(1000 + Result.NoteId);
		}
	}

	Super::HandleJudgementResult(Result);

	++JudgementCount;
	if (bHasJudgedNote)
	{
		RemoveTrackedNote(CuedNotes, Result.NoteId);
		RemoveTrackedNote(ArmedNotes, Result.NoteId);
		RemoveTrackedNote(ReachedNotes, Result.NoteId);

		OnTGJudgement.Broadcast(Result, JudgedNote);
		OnTGNoteCleared.Broadcast(Result.NoteId, Result.JudgementType);
	}
	else if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		FPTBNoteEvent EmptyInputNote;
		OnTGJudgement.Broadcast(Result, EmptyInputNote);
	}

	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	if (!TGRuleSet || TGRuleSet->bLogJudgement)
	{
		LogJudgementDebug(Result);
	}
}

FPTBMiniGameResultPayload APTBTGMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();
	Payload.PayloadType = TEXT("TG");

	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	if (TGRuleSet && !TGRuleSet->bIncludeDebugPayload)
	{
		return Payload;
	}

	Payload.IntValues.Add(TEXT("CueCount"), CueCount);
	Payload.IntValues.Add(TEXT("ArmCount"), ArmCount);
	Payload.IntValues.Add(TEXT("NoteEventCount"), NoteEventCount);
	Payload.IntValues.Add(TEXT("LongNoteCueCount"), LongNoteCueCount);
	Payload.IntValues.Add(TEXT("JudgementCount"), JudgementCount);
	Payload.IntValues.Add(TEXT("CuedNotesRemaining"), CuedNotes.Num());
	Payload.IntValues.Add(TEXT("ArmedNotesRemaining"), ArmedNotes.Num());
	Payload.IntValues.Add(TEXT("ReachedNotesRemaining"), ReachedNotes.Num());

	if (ScoreCalculator)
	{
		Payload.IntValues.Add(TEXT("MissCount"), ScoreCalculator->MissCount);
	}

	return Payload;
}

void APTBTGMiniGame::HandleTGInput(EPTBActionType Action, float TimeMs)
{
	if (Action == EPTBActionType::None)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid TG action input."), *GetNameSafe(this));
		return;
	}

	HandleRhythmInput(Action, TimeMs);
}

void APTBTGMiniGame::HandleActionAInput(float TimeMs)
{
	HandleTGInput(EPTBActionType::ActionA, TimeMs);
}

void APTBTGMiniGame::HandleActionBInput(float TimeMs)
{
	HandleTGInput(EPTBActionType::ActionB, TimeMs);
}

void APTBTGMiniGame::HandleActionCInput(float TimeMs)
{
	HandleTGInput(EPTBActionType::ActionC, TimeMs);
}

void APTBTGMiniGame::HandleActionDInput(float TimeMs)
{
	HandleTGInput(EPTBActionType::ActionD, TimeMs);
}

void APTBTGMiniGame::HandleActionEInput(float TimeMs)
{
	HandleTGInput(EPTBActionType::ActionE, TimeMs);
}

void APTBTGMiniGame::HandleTGInputReleased(EPTBActionType Action, float TimeMs)
{
	if (Action == EPTBActionType::None)
	{
		UE_LOG(LogPTBMiniGames, Warning, TEXT("[%s] Invalid TG action release input."), *GetNameSafe(this));
		return;
	}

	HandleRhythmInputReleased(Action, TimeMs);
}

void APTBTGMiniGame::HandleActionAReleased(float TimeMs)
{
	HandleTGInputReleased(EPTBActionType::ActionA, TimeMs);
}

void APTBTGMiniGame::HandleActionBReleased(float TimeMs)
{
	HandleTGInputReleased(EPTBActionType::ActionB, TimeMs);
}

void APTBTGMiniGame::HandleActionCReleased(float TimeMs)
{
	HandleTGInputReleased(EPTBActionType::ActionC, TimeMs);
}

void APTBTGMiniGame::HandleActionDReleased(float TimeMs)
{
	HandleTGInputReleased(EPTBActionType::ActionD, TimeMs);
}

void APTBTGMiniGame::HandleActionEReleased(float TimeMs)
{
	HandleTGInputReleased(EPTBActionType::ActionE, TimeMs);
}

FPTBJudgementResult APTBTGMiniGame::EvaluateHoldInput(EPTBActionType Action, float TimeMs)
{
	FPTBNoteEvent HoldNote;
	const bool bHasHoldNote = JudgementSystem
		&& JudgementSystem->FindBestPendingNote(Action, TimeMs, HoldNote)
		&& HoldNote.NoteType == EPTBNoteType::Hold;

	const FPTBJudgementResult Result = Super::EvaluateHoldInput(Action, TimeMs);
	if (bHasHoldNote && Result.Reason == EPTBJudgementReason::Note && Result.JudgementType != EPTBJudgementType::Miss)
	{
		OnTGHoldStarted.Broadcast(Result, HoldNote);

		// ── [Hold 시작] 얼마나 눌러야 하는지 표시 ───────────────
		const float RequiredMs = (HoldNote.ReleaseTimeMs - HoldNote.TimeMs) * 0.75f;
		PTBTGDebug::Screen(
			1000 + HoldNote.NoteId,
			RequiredMs / 1000.0f + 1.0f,
			FColor::Cyan,
			FString::Printf(TEXT("[Hold 시작] %.0fms 더 누르세요 (75%% 기준) → 그 후 떼세요"),
				RequiredMs));
	}

	return Result;
}

const UPTBTGMiniGameRuleSet* APTBTGMiniGame::GetTGRuleSet() const
{
	return Cast<UPTBTGMiniGameRuleSet>(RuleSet.Get());
}

void APTBTGMiniGame::TrackNote(TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note)
{
	for (const FPTBNoteEvent& TrackedNote : Notes)
	{
		if (TrackedNote.NoteId == Note.NoteId)
		{
			return;
		}
	}

	Notes.Add(Note);
}

bool APTBTGMiniGame::RemoveTrackedNote(TArray<FPTBNoteEvent>& Notes, int32 NoteId)
{
	for (int32 Index = 0; Index < Notes.Num(); ++Index)
	{
		if (Notes[Index].NoteId == NoteId)
		{
			Notes.RemoveAt(Index);
			return true;
		}
	}

	return false;
}

bool APTBTGMiniGame::FindTrackedNote(int32 NoteId, FPTBNoteEvent& OutNote) const
{
	for (const FPTBNoteEvent& Note : ArmedNotes)
	{
		if (Note.NoteId == NoteId)
		{
			OutNote = Note;
			return true;
		}
	}

	for (const FPTBNoteEvent& Note : CuedNotes)
	{
		if (Note.NoteId == NoteId)
		{
			OutNote = Note;
			return true;
		}
	}

	for (const FPTBNoteEvent& Note : ReachedNotes)
	{
		if (Note.NoteId == NoteId)
		{
			OutNote = Note;
			return true;
		}
	}

	return false;
}

void APTBTGMiniGame::LogNoteDebug(const TCHAR* EventName, const FPTBNoteEvent& Note) const
{
	const UPTBTGMiniGameRuleSet* TGRuleSet = GetTGRuleSet();
	const bool bLogLongNoteDetails = !TGRuleSet || TGRuleSet->bLogLongNoteDetails;

	UE_LOG(LogPTBMiniGames, Log,
		TEXT("[%s] TG %s NoteId=%d Action=%d NoteType=%d Lane=%d Beat=%.3f TimeMs=%.3f Long=%d DurationBeat=%.3f ReleaseBeat=%.3f ReleaseTimeMs=%.3f"),
		*GetNameSafe(this),
		EventName,
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		static_cast<int32>(Note.NoteType),
		Note.Lane,
		Note.BeatTime,
		Note.TimeMs,
		Note.bIsLongNote ? 1 : 0,
		Note.DurationBeat,
		bLogLongNoteDetails ? Note.ReleaseBeatTime : 0.0f,
		bLogLongNoteDetails ? Note.ReleaseTimeMs : 0.0f);
}

void APTBTGMiniGame::LogJudgementDebug(const FPTBJudgementResult& Result) const
{
	UE_LOG(LogPTBMiniGames, Log,
		TEXT("[%s] TG Judgement NoteId=%d Action=%d Type=%d Reason=%d ChartMs=%.3f InputMs=%.3f DeltaMs=%.3f ScoreDelta=%d Count=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		Result.ChartTimeMs,
		Result.InputTimeMs,
		Result.DeltaMs,
		Result.ScoreDelta,
		JudgementCount);
}
