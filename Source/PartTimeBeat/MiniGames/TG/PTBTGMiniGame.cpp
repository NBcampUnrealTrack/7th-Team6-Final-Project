#include "MiniGames/TG/PTBTGMiniGame.h"

#include "Debug/PTBLogChannels.h"
#include "MiniGames/TG/PTBTGMiniGameRuleSet.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBScoreCalculator.h"

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
	if (Result.Reason == EPTBJudgementReason::EmptyInput && (!RuleSet || !RuleSet->ShouldTreatEmptyInputAsMiss()))
	{
		Super::HandleJudgementResult(Result);
		return;
	}

	FPTBNoteEvent JudgedNote;
	const bool bHasJudgedNote = FindTrackedNote(Result.NoteId, JudgedNote);

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
		TEXT("[%s] TG Judgement NoteId=%d Action=%d Type=%d Reason=%d DeltaMs=%.3f ScoreDelta=%d Count=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		Result.DeltaMs,
		Result.ScoreDelta,
		JudgementCount);
}
