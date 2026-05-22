#include "Rhythm/PTBRhythmChartParser.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

	FString DifficultyString;
	if (RootObject->TryGetStringField(TEXT("difficulty"), DifficultyString))
	{
		OutChartData.Difficulty = ParseDifficulty(DifficultyString);
	}

	double NumberValue = 0.0;
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

	auto ReadArrayNumber = [&NoteFormatIndex](const TArray<TSharedPtr<FJsonValue>>& Values, const FString& FieldName, double& OutValue) -> bool
	{
		const int32* FieldIndex = NoteFormatIndex.Find(FieldName);
		if (!FieldIndex || !Values.IsValidIndex(*FieldIndex))
		{
			return false;
		}

		return Values[*FieldIndex]->TryGetNumber(OutValue);
	};

	auto ReadArrayString = [&NoteFormatIndex](const TArray<TSharedPtr<FJsonValue>>& Values, const FString& FieldName, FString& OutValue) -> bool
	{
		const int32* FieldIndex = NoteFormatIndex.Find(FieldName);
		if (!FieldIndex || !Values.IsValidIndex(*FieldIndex))
		{
			return false;
		}

		return Values[*FieldIndex]->TryGetString(OutValue);
	};

	TSet<int32> AssignedNoteIds;
	int32 NextGeneratedNoteId = 1;
	auto GenerateMissingNoteId = [&AssignedNoteIds, &NextGeneratedNoteId]() -> int32
	{
		while (AssignedNoteIds.Contains(NextGeneratedNoteId))
		{
			++NextGeneratedNoteId;
		}

		const int32 GeneratedNoteId = NextGeneratedNoteId;
		AssignedNoteIds.Add(GeneratedNoteId);
		++NextGeneratedNoteId;
		return GeneratedNoteId;
	};

	for (int32 Index = 0; Index < NotesArray->Num(); ++Index)
	{
		const TSharedPtr<FJsonValue>& RawNote = (*NotesArray)[Index];
		const TSharedPtr<FJsonObject> NoteObject = RawNote->AsObject();
		FPTBNoteEvent Note;
		int32 IntValue = 0;
		bool bHasTimeMs = false;
		bool bHasNoteId = false;
		bool bHasAction = false;
		bool bActionSupported = false;

		if (NoteObject.IsValid())
		{
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
				Note.bIsLongNote = NoteTypeString.Equals(TEXT("Hold"), ESearchCase::IgnoreCase);
				Note.Payload.Add(TEXT("NoteType"), NoteTypeString);
			}

			if (NoteObject->TryGetNumberField(TEXT("durationMs"), NumberValue))
			{
				Note.Payload.Add(TEXT("DurationMs"), FString::SanitizeFloat(static_cast<float>(NumberValue)));
			}

			if (NoteObject->TryGetNumberField(TEXT("durationBeat"), NumberValue) || NoteObject->TryGetNumberField(TEXT("duration"), NumberValue))
			{
				Note.DurationBeat = static_cast<float>(NumberValue);
			}

			if (NoteObject->TryGetNumberField(TEXT("lane"), NumberValue) || NoteObject->TryGetNumberField(TEXT("track"), NumberValue))
			{
				IntValue = static_cast<int32>(NumberValue);
				Note.Payload.Add(TEXT("Lane"), FString::FromInt(IntValue));
			}

			FString SectionString;
			if (NoteObject->TryGetStringField(TEXT("section"), SectionString) || NoteObject->TryGetStringField(TEXT("sectionName"), SectionString))
			{
				Note.SectionName = FName(*SectionString);
			}
		}
		else if (RawNote->Type == EJson::Array)
		{
			const TArray<TSharedPtr<FJsonValue>>& NoteValues = RawNote->AsArray();

			if (ReadArrayNumber(NoteValues, TEXT("id"), NumberValue))
			{
				Note.NoteId = static_cast<int32>(NumberValue);
				bHasNoteId = true;
			}

			if (ReadArrayNumber(NoteValues, TEXT("timeMs"), NumberValue) || ReadArrayNumber(NoteValues, TEXT("time"), NumberValue))
			{
				Note.TimeMs = static_cast<float>(NumberValue);
				bHasTimeMs = true;
			}

			if (ReadArrayNumber(NoteValues, TEXT("beat"), NumberValue) || ReadArrayNumber(NoteValues, TEXT("beatTime"), NumberValue))
			{
				Note.BeatTime = static_cast<float>(NumberValue);
			}

			FString ActionString;
			if (ReadArrayString(NoteValues, TEXT("action"), ActionString))
			{
				bHasAction = true;
				Note.ActionType = ParseActionType(ActionString);
				bActionSupported = Note.ActionType != EPTBActionType::None;
			}

			FString NoteTypeString;
			if (ReadArrayString(NoteValues, TEXT("type"), NoteTypeString))
			{
				Note.bIsLongNote = NoteTypeString.Equals(TEXT("Hold"), ESearchCase::IgnoreCase);
				Note.Payload.Add(TEXT("NoteType"), NoteTypeString);
			}

			if (ReadArrayNumber(NoteValues, TEXT("durationMs"), NumberValue))
			{
				Note.Payload.Add(TEXT("DurationMs"), FString::SanitizeFloat(static_cast<float>(NumberValue)));
			}

			if (ReadArrayNumber(NoteValues, TEXT("durationBeat"), NumberValue) || ReadArrayNumber(NoteValues, TEXT("duration"), NumberValue))
			{
				Note.DurationBeat = static_cast<float>(NumberValue);
			}

			if (ReadArrayNumber(NoteValues, TEXT("lane"), NumberValue) || ReadArrayNumber(NoteValues, TEXT("track"), NumberValue))
			{
				IntValue = static_cast<int32>(NumberValue);
				Note.Payload.Add(TEXT("Lane"), FString::FromInt(IntValue));
			}

			FString SectionString;
			if (ReadArrayString(NoteValues, TEXT("section"), SectionString) || ReadArrayString(NoteValues, TEXT("sectionName"), SectionString))
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

		if (!bHasAction)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] is missing action."), Index)));
		}
		else if (!bActionSupported)
		{
			OutErrors.Add(FText::FromString(FString::Printf(TEXT("notes[%d] has unsupported action value."), Index)));
		}

		if (!bHasNoteId)
		{
			Note.NoteId = GenerateMissingNoteId();
		}
		else if (Note.NoteId > 0)
		{
			AssignedNoteIds.Add(Note.NoteId);
		}

		OutNoteEvents.Add(Note);
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
