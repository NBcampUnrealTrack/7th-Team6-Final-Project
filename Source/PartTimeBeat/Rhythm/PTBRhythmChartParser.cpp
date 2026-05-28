#include "Rhythm/PTBRhythmChartParser.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace PTBRhythmChartParserInternal
{
	struct FParsedChartNote
	{
		FPTBNoteEvent Note;
		int32 SourceIndex = INDEX_NONE;
	};

	void SortNotesByTime(TArray<FParsedChartNote>& Notes)
	{
		Notes.Sort([](const FParsedChartNote& Left, const FParsedChartNote& Right)
		{
			if (FMath::IsNearlyEqual(Left.Note.TimeMs, Right.Note.TimeMs))
			{
				return Left.Note.NoteId < Right.Note.NoteId;
			}

			return Left.Note.TimeMs < Right.Note.TimeMs;
		});
	}

	bool TryParseNoteType(const FString& InType, EPTBNoteType& OutNoteType)
	{
		if (InType.Equals(TEXT("Tap"), ESearchCase::IgnoreCase))
		{
			OutNoteType = EPTBNoteType::Tap;
			return true;
		}

		if (InType.Equals(TEXT("Hold"), ESearchCase::IgnoreCase))
		{
			OutNoteType = EPTBNoteType::Hold;
			return true;
		}

		if (InType.Equals(TEXT("Release"), ESearchCase::IgnoreCase))
		{
			OutNoteType = EPTBNoteType::Release;
			return true;
		}

		return false;
	}

	void AddTempoEvent(FPTBChartData& ChartData, float Beat, float Bpm)
	{
		ChartData.TempoChangeBeats.Add(Beat);
		ChartData.TempoChangeBpms.Add(Bpm);
	}

	void AddTimeSignatureEvent(FPTBChartData& ChartData, float Beat, int32 Numerator, int32 Denominator)
	{
		ChartData.TimeSignatureChangeBeats.Add(Beat);
		ChartData.TimeSignatureChangeNumerators.Add(Numerator);
		ChartData.TimeSignatureChangeDenominators.Add(Denominator);
	}

	bool ReadArrayNumber(
		const TMap<FString, int32>& NoteFormatIndex,
		const TArray<TSharedPtr<FJsonValue>>& Values,
		const FString& FieldName,
		double& OutValue)
	{
		const int32* FieldIndex = NoteFormatIndex.Find(FieldName);
		if (!FieldIndex || !Values.IsValidIndex(*FieldIndex) || !Values[*FieldIndex].IsValid())
		{
			return false;
		}

		return Values[*FieldIndex]->TryGetNumber(OutValue);
	}

	bool ReadArrayString(
		const TMap<FString, int32>& NoteFormatIndex,
		const TArray<TSharedPtr<FJsonValue>>& Values,
		const FString& FieldName,
		FString& OutValue)
	{
		const int32* FieldIndex = NoteFormatIndex.Find(FieldName);
		if (!FieldIndex || !Values.IsValidIndex(*FieldIndex) || !Values[*FieldIndex].IsValid())
		{
			return false;
		}

		return Values[*FieldIndex]->TryGetString(OutValue);
	}

	int32 GenerateMissingNoteId(TSet<int32>& AssignedNoteIds, int32& NextGeneratedNoteId)
	{
		while (AssignedNoteIds.Contains(NextGeneratedNoteId))
		{
			++NextGeneratedNoteId;
		}

		const int32 GeneratedNoteId = NextGeneratedNoteId;
		AssignedNoteIds.Add(GeneratedNoteId);
		++NextGeneratedNoteId;
		return GeneratedNoteId;
	}
}

bool UPTBRhythmChartParser::ParseChartFile(const FString& FilePath, FPTBChartData& OutChartData, TArray<FPTBNoteEvent>& OutNoteEvents, TArray<FText>& OutErrors)
{
	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		OutErrors.Add(FText::FromString(FString::Printf(TEXT("Failed to read chart file: %s"), *FilePath)));
		return false;
	}

	return ParseChartString(JsonString, OutChartData, OutNoteEvents, OutErrors);
}

bool UPTBRhythmChartParser::ParseChartString(const FString& JsonString, FPTBChartData& OutChartData, TArray<FPTBNoteEvent>& OutNoteEvents, TArray<FText>& OutErrors)
{
	OutErrors.Reset();
	OutChartData = FPTBChartData();
	OutNoteEvents.Reset();

	TSharedPtr<FJsonObject> RootObject;
	const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

	if (!FJsonSerializer::Deserialize(Reader, RootObject) || !RootObject.IsValid())
	{
		OutErrors.Add(FText::FromString(TEXT("Invalid chart JSON.")));
		return false;
	}

	OutChartData.ChartId = ReadNameField(RootObject, TEXT("chartId"));
	OutChartData.SongId = ReadNameField(RootObject, TEXT("songId"));
	OutChartData.MiniGameId = ReadNameField(RootObject, TEXT("miniGameId"));

	double NumberValue = 0.0;
	int32 IntValue = 0;
	if (RootObject->TryGetNumberField(TEXT("chartVersion"), NumberValue))
	{
		OutChartData.ChartVersion = FMath::Max(1, static_cast<int32>(NumberValue));
	}

	FString DifficultyString;
	if (RootObject->TryGetStringField(TEXT("difficulty"), DifficultyString))
	{
		OutChartData.Difficulty = ParseDifficulty(DifficultyString);
	}

	if (RootObject->TryGetNumberField(TEXT("bpm"), NumberValue))
	{
		OutChartData.BPM = static_cast<float>(NumberValue);
	}

	if (RootObject->TryGetNumberField(TEXT("offsetMs"), NumberValue))
	{
		OutChartData.OffsetMs = static_cast<float>(NumberValue);
	}

	if (RootObject->TryGetNumberField(TEXT("songLengthMs"), NumberValue))
	{
		OutChartData.SongLengthMs = static_cast<float>(NumberValue);
	}

	const TSharedPtr<FJsonObject>* TimeSignatureObject = nullptr;
	if (RootObject->TryGetObjectField(TEXT("timeSignature"), TimeSignatureObject) && TimeSignatureObject && TimeSignatureObject->IsValid())
	{
		if ((*TimeSignatureObject)->TryGetNumberField(TEXT("numerator"), NumberValue))
		{
			OutChartData.TimeSignatureNumerator = FMath::Max(1, static_cast<int32>(NumberValue));
		}

		if ((*TimeSignatureObject)->TryGetNumberField(TEXT("denominator"), NumberValue))
		{
			OutChartData.TimeSignatureDenominator = FMath::Max(1, static_cast<int32>(NumberValue));
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* TempoEventsArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("tempoEvents"), TempoEventsArray) && TempoEventsArray)
	{
		for (const TSharedPtr<FJsonValue>& TempoValue : *TempoEventsArray)
		{
			const TSharedPtr<FJsonObject> TempoObject = TempoValue->AsObject();
			if (!TempoObject.IsValid())
			{
				OutErrors.Add(FText::FromString(TEXT("tempoEvents must contain objects.")));
				continue;
			}

			double BeatValue = 0.0;
			double BpmValue = 0.0;
			const bool bHasBeat = TempoObject->TryGetNumberField(TEXT("beat"), BeatValue);
			const bool bHasBpm = TempoObject->TryGetNumberField(TEXT("bpm"), BpmValue);
			if (!bHasBeat || !bHasBpm || BpmValue <= 0.0)
			{
				OutErrors.Add(FText::FromString(TEXT("tempoEvents entries must contain valid beat and bpm.")));
				continue;
			}

			PTBRhythmChartParserInternal::AddTempoEvent(OutChartData, static_cast<float>(BeatValue), static_cast<float>(BpmValue));
		}
	}

	const TArray<TSharedPtr<FJsonValue>>* TimeSignatureEventsArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("timeSignatureEvents"), TimeSignatureEventsArray) && TimeSignatureEventsArray)
	{
		for (const TSharedPtr<FJsonValue>& SignatureValue : *TimeSignatureEventsArray)
		{
			const TSharedPtr<FJsonObject> SignatureObject = SignatureValue->AsObject();
			if (!SignatureObject.IsValid())
			{
				OutErrors.Add(FText::FromString(TEXT("timeSignatureEvents must contain objects.")));
				continue;
			}

			double BeatValue = 0.0;
			double NumeratorValue = 0.0;
			double DenominatorValue = 0.0;
			const bool bHasBeat = SignatureObject->TryGetNumberField(TEXT("beat"), BeatValue);
			const bool bHasNumerator = SignatureObject->TryGetNumberField(TEXT("numerator"), NumeratorValue);
			const bool bHasDenominator = SignatureObject->TryGetNumberField(TEXT("denominator"), DenominatorValue);
			if (!bHasBeat || !bHasNumerator || !bHasDenominator || NumeratorValue <= 0.0 || DenominatorValue <= 0.0)
			{
				OutErrors.Add(FText::FromString(TEXT("timeSignatureEvents entries must contain valid beat, numerator and denominator.")));
				continue;
			}

			PTBRhythmChartParserInternal::AddTimeSignatureEvent(
				OutChartData,
				static_cast<float>(BeatValue),
				static_cast<int32>(NumeratorValue),
				static_cast<int32>(DenominatorValue));
		}
	}

	if (OutChartData.TempoChangeBeats.IsEmpty())
	{
		PTBRhythmChartParserInternal::AddTempoEvent(OutChartData, 0.0f, OutChartData.BPM);
	}

	if (OutChartData.TimeSignatureChangeBeats.IsEmpty())
	{
		PTBRhythmChartParserInternal::AddTimeSignatureEvent(
			OutChartData,
			0.0f,
			OutChartData.TimeSignatureNumerator,
			OutChartData.TimeSignatureDenominator);
	}

	const TSharedPtr<FJsonObject>* WwiseObject = nullptr;
	if (RootObject->TryGetObjectField(TEXT("wwise"), WwiseObject) && WwiseObject && WwiseObject->IsValid())
	{
		OutChartData.WwiseBankName = ReadNameField(*WwiseObject, TEXT("bankName"));
		OutChartData.WwiseEventName = ReadNameField(*WwiseObject, TEXT("bgmEventName"));
	}

	const TArray<TSharedPtr<FJsonValue>>* NotesArray = nullptr;
	if (!RootObject->TryGetArrayField(TEXT("notes"), NotesArray) || !NotesArray)
	{
		OutErrors.Add(FText::FromString(TEXT("Chart JSON must contain a notes array.")));
		return false;
	}

	TMap<FString, int32> NoteFormatIndex;
	const TArray<TSharedPtr<FJsonValue>>* NoteFormatArray = nullptr;
	if (RootObject->TryGetArrayField(TEXT("noteFormat"), NoteFormatArray) && NoteFormatArray)
	{
		for (int32 FormatIndex = 0; FormatIndex < NoteFormatArray->Num(); ++FormatIndex)
		{
			FString FieldName;
			if ((*NoteFormatArray)[FormatIndex]->TryGetString(FieldName))
			{
				NoteFormatIndex.Add(FieldName, FormatIndex);
			}
		}
	}

	TArray<PTBRhythmChartParserInternal::FParsedChartNote> ParsedNotes;
	TSet<int32> AssignedNoteIds;
	int32 NextGeneratedNoteId = 1;

	for (int32 Index = 0; Index < NotesArray->Num(); ++Index)
	{
		const TSharedPtr<FJsonValue>& RawNote = (*NotesArray)[Index];
		PTBRhythmChartParserInternal::FParsedChartNote ParsedNote;
		FPTBNoteEvent& Note = ParsedNote.Note;
		bool bHasTimeMs = false;
		bool bHasNoteId = false;
		bool bHasAction = false;
		bool bActionSupported = false;
		bool bHasLane = false;
		bool bHasNoteType = false;
		ParsedNote.SourceIndex = Index;

		if (RawNote.IsValid() && RawNote->Type == EJson::Object)
		{
			const TSharedPtr<FJsonObject> NoteObject = RawNote->AsObject();
			if (!NoteObject.IsValid())
			{
				OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] must be a valid object."), Index)));
				continue;
			}

			if (NoteObject->TryGetNumberField(TEXT("id"), NumberValue))
			{
				Note.NoteId = static_cast<int32>(NumberValue);
				bHasNoteId = true;
			}

			if (NoteObject->TryGetNumberField(TEXT("timeMs"), NumberValue) || NoteObject->TryGetNumberField(TEXT("time"), NumberValue))
			{
				Note.TimeMs = static_cast<float>(NumberValue);
				bHasTimeMs = true;
			}

			if (NoteObject->TryGetNumberField(TEXT("beat"), NumberValue) || NoteObject->TryGetNumberField(TEXT("beatTime"), NumberValue))
			{
				Note.BeatTime = static_cast<float>(NumberValue);
			}

			FString ActionString;
			if (NoteObject->TryGetStringField(TEXT("action"), ActionString))
			{
				bHasAction = true;
				Note.ActionType = ParseActionType(ActionString);
				bActionSupported = Note.ActionType != EPTBActionType::None;
			}

			FString NoteTypeString;
			if (NoteObject->TryGetStringField(TEXT("type"), NoteTypeString))
			{
				bHasNoteType = true;
				if (!PTBRhythmChartParserInternal::TryParseNoteType(NoteTypeString, Note.NoteType))
				{
					OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] has unsupported type value."), Index)));
				}
			}

			if (NoteObject->TryGetNumberField(TEXT("durationBeat"), NumberValue) || NoteObject->TryGetNumberField(TEXT("duration"), NumberValue))
			{
				Note.DurationBeat = static_cast<float>(NumberValue);
			}

			if (NoteObject->TryGetNumberField(TEXT("lane"), NumberValue))
			{
				IntValue = static_cast<int32>(NumberValue);
				Note.Lane = IntValue;
				bHasLane = true;
			}

			FString SectionString;
			if (NoteObject->TryGetStringField(TEXT("section"), SectionString) || NoteObject->TryGetStringField(TEXT("sectionName"), SectionString))
			{
				Note.SectionName = FName(*SectionString);
			}
		}
		else if (RawNote.IsValid() && RawNote->Type == EJson::Array)
		{
			const TArray<TSharedPtr<FJsonValue>>& NoteValues = RawNote->AsArray();

			if (PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("id"), NumberValue))
			{
				Note.NoteId = static_cast<int32>(NumberValue);
				bHasNoteId = true;
			}

			if (PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("timeMs"), NumberValue)
				|| PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("time"), NumberValue))
			{
				Note.TimeMs = static_cast<float>(NumberValue);
				bHasTimeMs = true;
			}

			if (PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("beat"), NumberValue)
				|| PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("beatTime"), NumberValue))
			{
				Note.BeatTime = static_cast<float>(NumberValue);
			}

			FString ActionString;
			if (PTBRhythmChartParserInternal::ReadArrayString(NoteFormatIndex, NoteValues, TEXT("action"), ActionString))
			{
				bHasAction = true;
				Note.ActionType = ParseActionType(ActionString);
				bActionSupported = Note.ActionType != EPTBActionType::None;
			}

			FString NoteTypeString;
			if (PTBRhythmChartParserInternal::ReadArrayString(NoteFormatIndex, NoteValues, TEXT("type"), NoteTypeString))
			{
				bHasNoteType = true;
				if (!PTBRhythmChartParserInternal::TryParseNoteType(NoteTypeString, Note.NoteType))
				{
					OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] has unsupported type value."), Index)));
				}
			}

			if (PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("durationBeat"), NumberValue)
				|| PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("duration"), NumberValue))
			{
				Note.DurationBeat = static_cast<float>(NumberValue);
			}

			if (PTBRhythmChartParserInternal::ReadArrayNumber(NoteFormatIndex, NoteValues, TEXT("lane"), NumberValue))
			{
				IntValue = static_cast<int32>(NumberValue);
				Note.Lane = IntValue;
				bHasLane = true;
			}

			FString SectionString;
			if (PTBRhythmChartParserInternal::ReadArrayString(NoteFormatIndex, NoteValues, TEXT("section"), SectionString)
				|| PTBRhythmChartParserInternal::ReadArrayString(NoteFormatIndex, NoteValues, TEXT("sectionName"), SectionString))
			{
				Note.SectionName = FName(*SectionString);
			}
		}
		else
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] must be an object or array."), Index)));
			continue;
		}

		if (!bHasTimeMs)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] is missing timeMs."), Index)));
		}

		if (bHasNoteId && Note.NoteId <= 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] id must be greater than 0."), Index)));
		}

		if (!bHasAction)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] is missing action."), Index)));
		}
		else if (!bActionSupported)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] has unsupported action value."), Index)));
		}

		if (!bHasLane)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] is missing lane."), Index)));
		}
		else if (Note.Lane < 0)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] lane cannot be negative."), Index)));
		}

		if (!bHasNoteType)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] is missing type."), Index)));
		}

		if (!bHasNoteId)
		{
			Note.NoteId = PTBRhythmChartParserInternal::GenerateMissingNoteId(AssignedNoteIds, NextGeneratedNoteId);
		}
		else if (Note.NoteId > 0)
		{
			AssignedNoteIds.Add(Note.NoteId);
		}

		ParsedNotes.Add(ParsedNote);
	}

	PTBRhythmChartParserInternal::SortNotesByTime(ParsedNotes);

	TMap<int32, int32> ActiveHoldIndexByLane;
	for (const PTBRhythmChartParserInternal::FParsedChartNote& ParsedNote : ParsedNotes)
	{
		if (ParsedNote.Note.NoteType == EPTBNoteType::Tap)
		{
			OutNoteEvents.Add(ParsedNote.Note);
			continue;
		}

		if (ParsedNote.Note.NoteType == EPTBNoteType::Hold)
		{
			if (ActiveHoldIndexByLane.Contains(ParsedNote.Note.Lane))
			{
				OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] starts a Hold before the previous Hold on lane %d is released."), ParsedNote.SourceIndex, ParsedNote.Note.Lane)));
				continue;
			}

			FPTBNoteEvent HoldNote = ParsedNote.Note;
			HoldNote.bIsLongNote = true;
			HoldNote.DurationBeat = 0.0f;
			OutNoteEvents.Add(HoldNote);
			ActiveHoldIndexByLane.Add(ParsedNote.Note.Lane, OutNoteEvents.Num() - 1);
			continue;
		}

		const int32* ActiveHoldIndex = ActiveHoldIndexByLane.Find(ParsedNote.Note.Lane);
		if (!ActiveHoldIndex)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] has a Release without a matching Hold on lane %d."), ParsedNote.SourceIndex, ParsedNote.Note.Lane)));
			continue;
		}

		FPTBNoteEvent& HoldNote = OutNoteEvents[*ActiveHoldIndex];
		if (ParsedNote.Note.BeatTime <= HoldNote.BeatTime)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] Release must be after its Hold on lane %d."), ParsedNote.SourceIndex, ParsedNote.Note.Lane)));
			ActiveHoldIndexByLane.Remove(ParsedNote.Note.Lane);
			continue;
		}

		HoldNote.DurationBeat = ParsedNote.Note.BeatTime - HoldNote.BeatTime;
		HoldNote.ReleaseNoteId = ParsedNote.Note.NoteId;
		HoldNote.ReleaseBeatTime = ParsedNote.Note.BeatTime;
		HoldNote.ReleaseTimeMs = ParsedNote.Note.TimeMs;
		ActiveHoldIndexByLane.Remove(ParsedNote.Note.Lane);
	}

	for (const TPair<int32, int32>& ActiveHoldPair : ActiveHoldIndexByLane)
	{
		const FPTBNoteEvent& HoldNote = OutNoteEvents[ActiveHoldPair.Value];
		OutErrors.Add(FText::FromString(FString::Printf(TEXT("Hold note %d on lane %d is missing a matching Release."), HoldNote.NoteId, ActiveHoldPair.Key)));
	}

	OutNoteEvents.Sort([](const FPTBNoteEvent& Left, const FPTBNoteEvent& Right)
	{
		if (FMath::IsNearlyEqual(Left.TimeMs, Right.TimeMs))
		{
			return Left.NoteId < Right.NoteId;
		}

		return Left.TimeMs < Right.TimeMs;
	});

	return OutErrors.IsEmpty();
}

EPTBActionType UPTBRhythmChartParser::ParseActionType(const FString& Value)
{
	if (Value.Equals(TEXT("A"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ActionA"), ESearchCase::IgnoreCase))
	{
		return EPTBActionType::ActionA;
	}

	if (Value.Equals(TEXT("B"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ActionB"), ESearchCase::IgnoreCase))
	{
		return EPTBActionType::ActionB;
	}

	if (Value.Equals(TEXT("C"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ActionC"), ESearchCase::IgnoreCase))
	{
		return EPTBActionType::ActionC;
	}

	if (Value.Equals(TEXT("D"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ActionD"), ESearchCase::IgnoreCase))
	{
		return EPTBActionType::ActionD;
	}

	if (Value.Equals(TEXT("E"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("ActionE"), ESearchCase::IgnoreCase))
	{
		return EPTBActionType::ActionE;
	}

	return EPTBActionType::None;
}

EPTBDifficulty UPTBRhythmChartParser::ParseDifficulty(const FString& Value)
{
	if (Value.Equals(TEXT("Easy"), ESearchCase::IgnoreCase))
	{
		return EPTBDifficulty::Easy;
	}

	if (Value.Equals(TEXT("Normal"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Standard"), ESearchCase::IgnoreCase))
	{
		return EPTBDifficulty::Standard;
	}

	if (Value.Equals(TEXT("Hard"), ESearchCase::IgnoreCase) || Value.Equals(TEXT("Expert"), ESearchCase::IgnoreCase))
	{
		return EPTBDifficulty::Insane;
	}

	if (Value.Equals(TEXT("Insane"), ESearchCase::IgnoreCase))
	{
		return EPTBDifficulty::Insane;
	}

	return EPTBDifficulty::Standard;
}

FName UPTBRhythmChartParser::ReadNameField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName)
{
	if (!JsonObject.IsValid())
	{
		return NAME_None;
	}

	FString StringValue;
	if (!JsonObject->TryGetStringField(FieldName, StringValue))
	{
		return NAME_None;
	}

	return FName(*StringValue);
}
