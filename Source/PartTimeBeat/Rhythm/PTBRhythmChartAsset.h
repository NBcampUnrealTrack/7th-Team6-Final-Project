#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBRhythmChartAsset.generated.h"

/**
 * 리듬 게임의 채보 데이터를 담는 Primary Data Asset 예시
 */
UCLASS(BlueprintType) // 블루프린트에서 변수 타입 등으로 사용할 수 있게 합니다.
class PARTTIMEBEAT_API UPTBRhythmChartAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	FName ChartId;
	//FPTBChartMeta Meta;
	TArray<FPTBNoteEvent> NoteEvents;

	// 유효성 검사
    bool ValidateChart(TArray<FText>& OutErrors) const;
	//	Conductor 최적화
	TArray<FPTBNoteEvent> GetNotesInBeatRange(float Start, float End) const;
	//	외부.rhythmchart 파싱
	bool LoadFromJson(const FString& JsonPath);
};
