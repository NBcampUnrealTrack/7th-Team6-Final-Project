#include "Rhythm/PTBJudgementSystem.h"

namespace PTBJudgementSystemInternal
{
	bool IsSameNote(const FPTBNoteEvent& Left, const FPTBNoteEvent& Right)
	{
		if (Left.NoteId != 0 && Left.NoteId == Right.NoteId)
		{
			return true;
		}

		return Left.ActionType == Right.ActionType
			&& FMath::IsNearlyEqual(Left.TimeMs, Right.TimeMs)
			&& FMath::IsNearlyEqual(Left.BeatTime, Right.BeatTime);
	}

	EPTBJudgementType ResolveJudgementType(float AbsDeltaMs, float HighPerfectMs, float PerfectMs, float GoodMs)
	{
		if (AbsDeltaMs <= HighPerfectMs)
		{
			return EPTBJudgementType::HighPerfect;
		}

		if (AbsDeltaMs <= PerfectMs)
		{
			return EPTBJudgementType::Perfect;
		}

		if (AbsDeltaMs <= GoodMs)
		{
			return EPTBJudgementType::Good;
		}

		return EPTBJudgementType::Miss;
	}

	int32 GetBaseScoreForJudgement(EPTBJudgementType JudgementType)
	{
		switch (JudgementType)
		{
		case EPTBJudgementType::HighPerfect:
			return 1000;
		case EPTBJudgementType::Perfect:
			return 800;
		case EPTBJudgementType::Good:
			return 500;
		case EPTBJudgementType::Miss:
		default:
			return 0;
		}
	}

	FPTBJudgementResult MakeMissResult(
		EPTBActionType Action,
		EPTBJudgementReason Reason,
		int32 NoteId = 0,
		float DeltaMs = 0.0f,
		float ChartTimeMs = 0.0f,
		float InputTimeMs = 0.0f)
	{
		FPTBJudgementResult Result;
		Result.NoteId = NoteId;
		Result.ActionType = Action;
		Result.JudgementType = EPTBJudgementType::Miss;
		Result.Reason = Reason;
		Result.ChartTimeMs = ChartTimeMs;
		Result.InputTimeMs = InputTimeMs;
		Result.DeltaMs = DeltaMs;
		Result.ScoreDelta = 0;
		Result.bBreaksCombo = true;
		return Result;
	}

	FPTBJudgementResult MakeJudgementResult(const FPTBNoteEvent& Note, EPTBJudgementType JudgementType, float DeltaMs)
	{
		FPTBJudgementResult Result;
		Result.NoteId = Note.NoteId;
		Result.ActionType = Note.ActionType;
		Result.JudgementType = JudgementType;
		Result.Reason = EPTBJudgementReason::Note;
		Result.ChartTimeMs = Note.TimeMs;
		Result.InputTimeMs = Note.TimeMs + DeltaMs;
		Result.DeltaMs = DeltaMs;
		Result.ScoreDelta = GetBaseScoreForJudgement(JudgementType);
		Result.bBreaksCombo = JudgementType == EPTBJudgementType::Miss;
		return Result;
	}

	bool ShouldInsertBefore(const FPTBNoteEvent& Left, const FPTBNoteEvent& Right)
	{
		if (FMath::IsNearlyEqual(Left.TimeMs, Right.TimeMs))
		{
			return Left.NoteId < Right.NoteId;
		}

		return Left.TimeMs < Right.TimeMs;
	}

	int32 FindInsertIndex(const TArray<FPTBNoteEvent>& Notes, const FPTBNoteEvent& Note)
	{
		int32 Low = 0;
		int32 High = Notes.Num();

		while (Low < High)
		{
			const int32 Mid = Low + (High - Low) / 2;
			if (ShouldInsertBefore(Note, Notes[Mid]))
			{
				High = Mid;
			}
			else
			{
				Low = Mid + 1;
			}
		}

		return Low;
	}
}

UPTBJudgementSystem::UPTBJudgementSystem()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPTBJudgementSystem::BeginPlay()
{
	Super::BeginPlay();
}

void UPTBJudgementSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPTBJudgementSystem::Initialize(const FPTBChartData& Chart, float UserOffset)
{
	Reset();
	JudgementOffsetMs = UserOffset;
}

void UPTBJudgementSystem::RegisterNoteEvent(const FPTBNoteEvent& Note)
{
	for (const FPTBNoteEvent& PendingNote : PendingNotes)
	{
		if (PTBJudgementSystemInternal::IsSameNote(Note, PendingNote))
		{
			return;
		}
	}

	const int32 InsertIndex = PTBJudgementSystemInternal::FindInsertIndex(PendingNotes, Note);
	PendingNotes.Insert(Note, InsertIndex);
}

FPTBJudgementResult UPTBJudgementSystem::EvaluateInput(EPTBActionType Action, float InputTimeMs)
{
	return EvaluateInput(Action, InputTimeMs, true);
}

FPTBJudgementResult UPTBJudgementSystem::EvaluateInput(EPTBActionType Action, float InputTimeMs, bool bBroadcastResult)
{
	const float CorrectedInputTimeMs = InputTimeMs + JudgementOffsetMs;

	if (Action == EPTBActionType::None)
	{
		const FPTBJudgementResult Result = PTBJudgementSystemInternal::MakeMissResult(
			Action,
			EPTBJudgementReason::EmptyInput,
			0,
			0.0f,
			0.0f,
			CorrectedInputTimeMs);
		if (bBroadcastResult)
		{
			OnJudgementResult.Broadcast(Result);
		}
		return Result;
	}

	int32 BestNoteIndex = INDEX_NONE;
	float BestAbsDeltaMs = HitWindowMissMs;
	float BestSignedDeltaMs = 0.0f;

	for (int32 Index = 0; Index < PendingNotes.Num(); ++Index)
	{
		const FPTBNoteEvent& Note = PendingNotes[Index];
		if (Note.ActionType != Action)
		{
			continue;
		}

		const float SignedDeltaMs = CorrectedInputTimeMs - Note.TimeMs;
		const float AbsDeltaMs = FMath::Abs(SignedDeltaMs);
		if (AbsDeltaMs <= HitWindowMissMs && (BestNoteIndex == INDEX_NONE || AbsDeltaMs < BestAbsDeltaMs))
		{
			BestNoteIndex = Index;
			BestAbsDeltaMs = AbsDeltaMs;
			BestSignedDeltaMs = SignedDeltaMs;
		}
	}

	if (BestNoteIndex == INDEX_NONE)
	{
		const FPTBJudgementResult Result = PTBJudgementSystemInternal::MakeMissResult(
			Action,
			EPTBJudgementReason::EmptyInput,
			0,
			0.0f,
			0.0f,
			CorrectedInputTimeMs);
		if (bBroadcastResult)
		{
			OnJudgementResult.Broadcast(Result);
		}
		return Result;
	}

	const FPTBNoteEvent MatchedNote = PendingNotes[BestNoteIndex];
	PendingNotes.RemoveAt(BestNoteIndex);

	const EPTBJudgementType JudgementType = PTBJudgementSystemInternal::ResolveJudgementType(
		BestAbsDeltaMs,
		HitWindowHighPerfectMs,
		HitWindowPerfectMs,
		HitWindowGoodMs);

	const FPTBJudgementResult Result = PTBJudgementSystemInternal::MakeJudgementResult(
		MatchedNote,
		JudgementType,
		BestSignedDeltaMs);

	if (bBroadcastResult)
	{
		OnJudgementResult.Broadcast(Result);
	}
	return Result;
}

bool UPTBJudgementSystem::FindBestPendingNote(EPTBActionType Action, float InputTimeMs, FPTBNoteEvent& OutNote) const
{
	if (Action == EPTBActionType::None)
	{
		return false;
	}

	const float CorrectedInputTimeMs = InputTimeMs + JudgementOffsetMs;
	int32 BestNoteIndex = INDEX_NONE;
	float BestAbsDeltaMs = HitWindowMissMs;

	for (int32 Index = 0; Index < PendingNotes.Num(); ++Index)
	{
		const FPTBNoteEvent& Note = PendingNotes[Index];
		if (Note.ActionType != Action)
		{
			continue;
		}

		const float AbsDeltaMs = FMath::Abs(CorrectedInputTimeMs - Note.TimeMs);
		if (AbsDeltaMs <= HitWindowMissMs && (BestNoteIndex == INDEX_NONE || AbsDeltaMs < BestAbsDeltaMs))
		{
			BestNoteIndex = Index;
			BestAbsDeltaMs = AbsDeltaMs;
		}
	}

	if (BestNoteIndex == INDEX_NONE)
	{
		return false;
	}

	OutNote = PendingNotes[BestNoteIndex];
	return true;
}

TArray<FPTBJudgementResult> UPTBJudgementSystem::ForceMissExpiredNotes(float CurrentTimeMs)
{
	TArray<FPTBJudgementResult> MissResults;
	const float CorrectedCurrentTimeMs = CurrentTimeMs + JudgementOffsetMs;

	for (int32 Index = PendingNotes.Num() - 1; Index >= 0; --Index)
	{
		const FPTBNoteEvent& Note = PendingNotes[Index];
		const float SignedDeltaMs = CorrectedCurrentTimeMs - Note.TimeMs;
		if (SignedDeltaMs <= HitWindowMissMs)
		{
			continue;
		}

		MissResults.Insert(
			PTBJudgementSystemInternal::MakeMissResult(
				Note.ActionType,
				EPTBJudgementReason::ExpiredNote,
				Note.NoteId,
				SignedDeltaMs,
				Note.TimeMs,
				CorrectedCurrentTimeMs),
			0);
		PendingNotes.RemoveAt(Index);
	}

	for (const FPTBJudgementResult& Result : MissResults)
	{
		OnJudgementResult.Broadcast(Result);
	}

	return MissResults;
}

void UPTBJudgementSystem::Reset()
{
	PendingNotes.Reset();
	JudgementOffsetMs = 0.0f;
}
