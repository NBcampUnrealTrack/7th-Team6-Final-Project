#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseRhythmSyncComponent.generated.h"

class UPTBWwiseAudioManager;

/**
 * Wwise 재생 위치를 채보 시간과 Beat로 변환하는 리듬 동기화 컴포넌트입니다.
 *
 * RhythmConductor는 이 컴포넌트의 현재 음악 시간과 Beat를 읽어 노트 발행 시점을 결정합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PARTTIMEBEAT_API UPTBWwiseRhythmSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** 기본값 초기화 */
	UPTBWwiseRhythmSyncComponent();

protected:
	/** 게임 시작 처리 */
	virtual void BeginPlay() override;

public:	
	/** Beat Tick 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Sync")
	FOnBeatTick OnBeatTick;

	/** Bar Tick 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Sync")
	FOnBarTick OnBarTick;

	/** 매니저 참조 */
	UPROPERTY()
	TObjectPtr<UPTBWwiseAudioManager> WwiseManager = nullptr;

	/** 활성 PlayingId */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rhythm|Sync")
	int32 CurrentPlayingId = 0;

	/** 동기화 상태 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rhythm|Sync")
	FPTBWwiseSyncData SyncData;

	/** 현재 차트 메타 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rhythm|Sync")
	FPTBChartData ActiveChartData;

	/** 채보 오프셋 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rhythm|Sync")
	float ChartOffsetMs = 0.0f;

	/** 입력 판정용 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Sync")
	float InputOffsetMs = 0.0f;

	/** 화면 연출용 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Sync")
	float VisualOffsetMs = 0.0f;

	/** 소리 출력용 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Sync")
	float SoundOffsetMs = 0.0f;

	/** Wwise 콜백 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Sync")
	bool bUseMusicCallbacks = true;

	/** 마디당 Beat 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Sync", meta = (ClampMin = "1", UIMin = "1"))
	int32 BeatsPerBar = 4;

	/** 마지막 Beat Tick 인덱스 */
	int32 LastBeatTickIndex = -1;

	/** 마지막 Bar Tick 인덱스 */
	int32 LastBarTickIndex = -1;

	/** Wwise 재생 위치 기준 동기화 갱신 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Wwise 매니저 설정 */
	void SetWwiseManager(UPTBWwiseAudioManager* InWwiseManager);

	/** 유저 장치별 오프셋 설정 */
	void SetUserOffsets(float InInputOffsetMs, float InVisualOffsetMs, float InSoundOffsetMs);

	/** 마디당 Beat 수 설정 */
	void SetBeatsPerBar(int32 InBeatsPerBar);

	/** 동기화 시작 */
	void StartSync(int32 PlayingId, const FPTBChartData& InChartData);

	/** 정지 */
	void StopSync();

	/** Wwise 원본 재생 시간 */
	float GetRawPlaybackTimeMs() const;

	/** 채보 기준 시간 */
	float GetChartTimeMs() const;

	/** 소리 출력 기준 채보 시간 */
	float GetAudibleChartTimeMs() const;

	/** 화면 연출 기준 채보 시간 */
	float GetVisualChartTimeMs() const;

	/** 입력 판정 기준 채보 시간 */
	float GetInputJudgeTimeMs() const;

	/** 채보 기준 Beat */
	float GetChartBeat() const;

	/** 화면 연출 기준 Beat */
	float GetVisualBeat() const;

	/** 소리 출력 기준 Beat */
	float GetAudibleBeat() const;

	/** 현재 마디당 Beat 수 */
	int32 GetCurrentBeatsPerBar() const;

	/** 채보 기준 Beat */
	float GetCurrentBeat() const;

	/** Wwise 원본 재생 시간 */
	float GetCurrentTimeMs() const;

	/** 입력 판정 오프셋 조정 */
	void CalibrateInputOffset(float DeltaMs);
};
