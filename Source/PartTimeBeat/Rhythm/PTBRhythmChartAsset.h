#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBRhythmChartAsset.generated.h"

/**
 * 리듬 코어가 사용하는 공통 채보 DataAsset.
 * 미니게임별 연출 정보는 포함하지 않고, 메타와 순수 노트 배열만 관리합니다.
 *
 * 이 Asset은 "언제 어떤 공통 입력이 발생해야 하는지"만 표현합니다.
 * 예를 들어 ActionA, Lane 0, TimeMs 1200 같은 정보는 저장하지만,
 * 특정 미니게임의 에셋 등장, 특정 캐릭터 인덱스, SFX 키 같은 View 전용 정보는 저장하지 않습니다.
 *
 * RhythmConductor는 이 Asset의 NoteEvents를 Wwise 재생 시간 기준으로 읽어
 * PreCue, NoteReached, ChartEnd 같은 공통 이벤트를 발행합니다.
 * 각 미니게임은 RuleSet 또는 자체 View 로직을 통해 같은 노트를 자기 방식으로 해석합니다.
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBRhythmChartAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 파싱된 채보 식별자 */
	UPROPERTY()
	FName ChartId;

	/** 파싱된 곡, 난이도, BPM, Offset, Wwise 이벤트 등 공통 메타 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm|Parsed", AdvancedDisplay)
	FPTBChartData ChartData;

	/** 원본 JSON 파일 경로 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm|Source")
	FString SourceJsonFilePath;

	/** 파싱된 시간순 공통 노트 배열 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Rhythm|Parsed", AdvancedDisplay)
	TArray<FPTBNoteEvent> NoteEvents;

	/** 유효성 검사 */
	bool ValidateChart(TArray<FText>& OutErrors) const;
	/** Conductor 최적화 */
	TArray<FPTBNoteEvent> GetNotesInBeatRange(float Start, float End) const;
	/**	판정 / PreCue용 시간 범위 조회 */
	TArray<FPTBNoteEvent> GetNotesInTimeRange(float StartMs, float EndMs) const;
	/**	TimeMs 기준 정렬 */
	void SortNotesByTime();
	/**	외부.rhythmchart 파싱 */
	bool LoadFromJson(const FString& JsonPath, TArray<FText>& OutErrors);
	/** 원본 JSON 파일 경로 기준 파싱 */
	bool LoadFromSourceJson(TArray<FText>& OutErrors);
};
