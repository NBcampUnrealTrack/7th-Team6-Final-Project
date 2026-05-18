#include "Rhythm/PTBRhythmChartAsset.h"

bool UPTBRhythmChartAsset::ValidateChart(TArray<FText>& OutErrors) const
{
	return true;
}
TArray<FPTBNoteEvent> UPTBRhythmChartAsset::GetNotesInBeatRange(float Start, float End) const
{
	return TArray<FPTBNoteEvent>();
}
bool UPTBRhythmChartAsset::LoadFromJson(const FString& JsonPath)
{
	return true;
}