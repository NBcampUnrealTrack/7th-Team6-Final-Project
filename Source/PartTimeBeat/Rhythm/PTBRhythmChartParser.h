#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBRhythmChartParser.generated.h"

class FJsonObject;

/**
 * 외부 채보 파일 또는 JSON 문자열을 공통 리듬 채보 데이터로 변환하는 Parser입니다.
 *
 * 파일 확장자는 검사하지 않고, 내부 문자열이 JSON 형식이면 파싱합니다.
 * 미니게임별 연출 정보는 해석하지 않으며, FPTBChartData와 FPTBNoteEvent 배열로 표현되는 공통 리듬 정보만 변환합니다.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBRhythmChartParser : public UObject
{
	GENERATED_BODY()

public:
	/** 외부 파일 파싱 */
	static bool ParseChartFile(const FString& FilePath, FPTBChartData& OutChartData, TArray<FPTBNoteEvent>& OutNoteEvents, TArray<FText>& OutErrors);

	/** JSON 문자열 파싱 */
	static bool ParseChartString(const FString& JsonString, FPTBChartData& OutChartData, TArray<FPTBNoteEvent>& OutNoteEvents, TArray<FText>& OutErrors);

private:
	/** ActionType 변환 */
	static EPTBActionType ParseActionType(const FString& Value);

	/** Difficulty 변환 */
	static EPTBDifficulty ParseDifficulty(const FString& Value);

	/** FName 필드 읽기 */
	static FName ReadNameField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName);
};
