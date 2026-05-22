#include "Rhythm/PTBRhythmChartAsset.h"

#include "Debug/PTBLogChannels.h"
#include "Rhythm/PTBRhythmChartParser.h"

bool UPTBRhythmChartAsset::ValidateChart(TArray<FText>& OutErrors) const
{
	OutErrors.Reset();

	if (ChartId.IsNone())
	{
		OutErrors.Add(FText::FromString(TEXT("ChartId is empty.")));
	}

	if (ChartData.BPM <= 0.0f)
	{
		OutErrors.Add(FText::FromString(TEXT("BPM must be greater than 0.")));
	}

	if (ChartData.SongLengthMs < 0.0f)
	{
		OutErrors.Add(FText::FromString(TEXT("SongLengthMs cannot be negative.")));
	}

	if (NoteEvents.IsEmpty())
	{
		OutErrors.Add(FText::FromString(TEXT("NoteEvents is empty.")));
	}

	TSet<int32> SeenNoteIds;
	float PreviousTimeMs = -FLT_MAX;

	for (int32 Index = 0; Index < NoteEvents.Num(); ++Index)
	{
		const FPTBNoteEvent& Note = NoteEvents[Index];

		if (Note.NoteId <= 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].NoteId must be greater than 0."), Index)));
		}
		else
		{
			if (SeenNoteIds.Contains(Note.NoteId))
			{
				OutErrors.Add(FText::FromString(FString::Printf(TEXT("Duplicate NoteId found: %d."), Note.NoteId)));
			}

			SeenNoteIds.Add(Note.NoteId);
		}

		if (Note.TimeMs < 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].TimeMs cannot be negative."), Index)));
		}

		if (Note.BeatTime < 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].BeatTime cannot be negative."), Index)));
		}

		if (Note.ActionType == EPTBActionType::None)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].ActionType cannot be None."), Index)));
		}

		if (Note.bIsLongNote && Note.DurationBeat <= 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].DurationBeat must be greater than 0 for long notes."), Index)));
		}

		if (Note.TimeMs < PreviousTimeMs)
		{
			OutErrors.Add(FText::FromString(TEXT("NoteEvents must be sorted by TimeMs.")));
			break;
		}

		PreviousTimeMs = Note.TimeMs;
	}

	return OutErrors.IsEmpty();
}

TArray<FPTBNoteEvent> UPTBRhythmChartAsset::GetNotesInBeatRange(float Start, float End) const
{
	TArray<FPTBNoteEvent> Result;

	if (Start > End)
	{
		Swap(Start, End);
	}

	for (const FPTBNoteEvent& Note : NoteEvents)
	{
		if (Note.BeatTime >= Start && Note.BeatTime <= End)
		{
			Result.Add(Note);
		}

		if (Note.BeatTime > End)
		{
			break;
		}
	}

	return Result;
}

TArray<FPTBNoteEvent> UPTBRhythmChartAsset::GetNotesInTimeRange(float StartMs, float EndMs) const
{
	TArray<FPTBNoteEvent> Result;

	if (StartMs > EndMs)
	{
		Swap(StartMs, EndMs);
	}

	for (const FPTBNoteEvent& Note : NoteEvents)
	{
		if (Note.TimeMs >= StartMs && Note.TimeMs <= EndMs)
		{
			Result.Add(Note);
		}

		if (Note.TimeMs > EndMs)
		{
			break;
		}
	}

	return Result;
}

void UPTBRhythmChartAsset::SortNotesByTime()
{
	NoteEvents.Sort([](const FPTBNoteEvent& Left, const FPTBNoteEvent& Right)
	{
		if (FMath::IsNearlyEqual(Left.TimeMs, Right.TimeMs))
		{
			return Left.NoteId < Right.NoteId;
		}

		return Left.TimeMs < Right.TimeMs;
	});
}

bool UPTBRhythmChartAsset::LoadFromJson(const FString& JsonPath, TArray<FText>& OutErrors)
{
	FPTBChartData ParsedChartData;
	TArray<FPTBNoteEvent> ParsedNoteEvents;
	OutErrors.Reset();

	if (!UPTBRhythmChartParser::ParseChartFile(JsonPath, ParsedChartData, ParsedNoteEvents, OutErrors))
	{
		for (const FText& Error : OutErrors)
		{
			UE_LOG(LogRhythm, Error, TEXT("LoadFromJson parse failed [%s]: %s"), *JsonPath, *Error.ToString());
		}

		return false;
	}

	const FName PreviousChartId = ChartId;
	const FPTBChartData PreviousChartData = ChartData;
	const TArray<FPTBNoteEvent> PreviousNoteEvents = NoteEvents;

	ChartId = ParsedChartData.ChartId;
	ChartData = ParsedChartData;
	NoteEvents = MoveTemp(ParsedNoteEvents);
	SortNotesByTime();

	if (!ValidateChart(OutErrors))
	{
		ChartId = PreviousChartId;
		ChartData = PreviousChartData;
		NoteEvents = PreviousNoteEvents;

		for (const FText& Error : OutErrors)
		{
			UE_LOG(LogRhythm, Error, TEXT("LoadFromJson validation failed [%s]: %s"), *JsonPath, *Error.ToString());
		}

		return false;
	}

	return true;
}
