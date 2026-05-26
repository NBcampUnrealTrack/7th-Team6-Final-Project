#include "Rhythm/PTBRhythmChartAsset.h"

#include "Debug/PTBLogChannels.h"
#include "Misc/Paths.h"
#include "Rhythm/PTBRhythmChartParser.h"

bool UPTBRhythmChartAsset::ValidateChart(TArray<FText>& OutErrors) const
{
	OutErrors.Reset();

	if (ChartId.IsNone())
	{
		OutErrors.Add(FText::FromString(TEXT("ChartId is empty.")));
	}

	const bool bHasValidBaseBpm = ChartData.BPM > 0.0f;
	const bool bHasValidFirstTempoBpm = ChartData.TempoChangeBpms.IsValidIndex(0) && ChartData.TempoChangeBpms[0] > 0.0f;
	if (!bHasValidBaseBpm && !bHasValidFirstTempoBpm)
	{
		OutErrors.Add(FText::FromString(TEXT("BPM or first tempo event BPM must be greater than 0.")));
	}

	if (ChartData.SongLengthMs < 0.0f)
	{
		OutErrors.Add(FText::FromString(TEXT("SongLengthMs cannot be negative.")));
	}

	if (ChartData.TimeSignatureNumerator <= 0)
	{
		OutErrors.Add(FText::FromString(TEXT("TimeSignatureNumerator must be greater than 0.")));
	}

	if (ChartData.TimeSignatureDenominator <= 0)
	{
		OutErrors.Add(FText::FromString(TEXT("TimeSignatureDenominator must be greater than 0.")));
	}

	if (ChartData.TempoChangeBeats.Num() != ChartData.TempoChangeBpms.Num())
	{
		OutErrors.Add(FText::FromString(TEXT("Tempo change arrays must have the same length.")));
	}

	if (ChartData.TimeSignatureChangeBeats.Num() != ChartData.TimeSignatureChangeNumerators.Num()
		|| ChartData.TimeSignatureChangeBeats.Num() != ChartData.TimeSignatureChangeDenominators.Num())
	{
		OutErrors.Add(FText::FromString(TEXT("Time signature change arrays must have the same length.")));
	}

	float PreviousTempoBeat = -FLT_MAX;
	for (int32 Index = 0; Index < ChartData.TempoChangeBeats.Num(); ++Index)
	{
		const float TempoBeat = ChartData.TempoChangeBeats[Index];
		const float TempoBpm = ChartData.TempoChangeBpms[Index];

		if (TempoBeat < 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("TempoChangeBeats[%d] cannot be negative."), Index)));
		}

		if (TempoBpm <= 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("TempoChangeBpms[%d] must be greater than 0."), Index)));
		}

		if (TempoBeat < PreviousTempoBeat)
		{
			OutErrors.Add(FText::FromString(TEXT("Tempo change events must be sorted by beat.")));
			break;
		}

		if (FMath::IsNearlyEqual(TempoBeat, PreviousTempoBeat))
		{
			OutErrors.Add(FText::FromString(TEXT("Tempo change events cannot share the same beat.")));
			break;
		}

		PreviousTempoBeat = TempoBeat;
	}

	float PreviousTimeSignatureBeat = -FLT_MAX;
	for (int32 Index = 0; Index < ChartData.TimeSignatureChangeBeats.Num(); ++Index)
	{
		const float SignatureBeat = ChartData.TimeSignatureChangeBeats[Index];
		const int32 SignatureNumerator = ChartData.TimeSignatureChangeNumerators[Index];
		const int32 SignatureDenominator = ChartData.TimeSignatureChangeDenominators[Index];

		if (SignatureBeat < 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("TimeSignatureChangeBeats[%d] cannot be negative."), Index)));
		}

		if (SignatureNumerator <= 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("TimeSignatureChangeNumerators[%d] must be greater than 0."), Index)));
		}

		if (SignatureDenominator <= 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("TimeSignatureChangeDenominators[%d] must be greater than 0."), Index)));
		}

		if (SignatureBeat < PreviousTimeSignatureBeat)
		{
			OutErrors.Add(FText::FromString(TEXT("Time signature change events must be sorted by beat.")));
			break;
		}

		if (FMath::IsNearlyEqual(SignatureBeat, PreviousTimeSignatureBeat))
		{
			OutErrors.Add(FText::FromString(TEXT("Time signature change events cannot share the same beat.")));
			break;
		}

		PreviousTimeSignatureBeat = SignatureBeat;
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

		if (Note.NoteType == EPTBNoteType::Release)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].NoteType cannot be Release after chart parsing."), Index)));
		}

		if (Note.Lane < 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].Lane cannot be negative."), Index)));
		}

		if (Note.bIsLongNote && Note.DurationBeat <= 0.0f)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].DurationBeat must be greater than 0 for long notes."), Index)));
		}

		if (Note.bIsLongNote && Note.ReleaseBeatTime <= Note.BeatTime)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].ReleaseBeatTime must be after BeatTime for long notes."), Index)));
		}

		if (Note.bIsLongNote && Note.ReleaseTimeMs <= Note.TimeMs)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("NoteEvents[%d].ReleaseTimeMs must be after TimeMs for long notes."), Index)));
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

bool UPTBRhythmChartAsset::LoadFromSourceJson(TArray<FText>& OutErrors)
{
	OutErrors.Reset();

	if (SourceJsonFilePath.IsEmpty())
	{
		OutErrors.Add(FText::FromString(TEXT("SourceJsonFilePath is empty.")));
		return false;
	}

	FString ResolvedPath = SourceJsonFilePath;
	if (FPaths::IsRelative(ResolvedPath))
	{
		ResolvedPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectDir(), ResolvedPath));
	}

	return LoadFromJson(ResolvedPath, OutErrors);
}
