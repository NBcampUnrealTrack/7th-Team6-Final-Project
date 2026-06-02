#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBRhythmConductorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteCue, FPTBNoteEvent, NoteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteArm, FPTBNoteEvent, NoteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteEvent, FPTBNoteEvent, NoteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAllNotesPassed);

class UPTBRhythmChartAsset;
class UPTBWwiseAudioManager;
class UPTBWwiseRhythmSyncComponent;

/**
 * Wwise 재생 시간을 기준으로 공통 채보 노트를 발행하는 Conductor 컴포넌트입니다.
 *
 * 이 컴포넌트는 미니게임 연출을 직접 실행하지 않고, 채보의 노트 시점에 맞춰 공통 이벤트만 발행합니다.
 * 실제 라운드 종료는 BGM 종료 기준으로 처리하고, 이 컴포넌트는 모든 노트 발행 완료만 알립니다.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBRhythmConductorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** 기본값 초기화 */
	UPTBRhythmConductorComponent();

protected:
	/** 게임 시작 처리 */
	virtual void BeginPlay() override;

public:	
	/** Wwise 재생 시간 기준 이벤트 발행 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 메타 데이터 기반 시작 */
	void StartConductor(const FPTBChartData& Data, int32 PlayingId);

	/** ChartAsset 기반 시작 */
	void StartConductor(UPTBRhythmChartAsset* InChartAsset, int32 PlayingId, UPTBWwiseAudioManager* InAudioManager);

	/** 일시정지 */
	void PauseConductor();

	/** 재개 */
	void ResumeConductor();

	/** 정지 및 초기화 */
	void StopConductor();

	/** 현재 음악 시간 */
	float GetCurrentMusicTimeMs() const;

	/** 현재 Beat */
	float GetCurrentBeat() const;

	/** 판정 선행 등록 시간(ms) 설정 */
	void SetArmLeadTimeMs(float InArmLeadTimeMs);

	/** 비주얼 큐 선행 시간 기준 설정 */
	void SetCueLeadTimeMode(EPTBCueLeadTimeMode InCueLeadTimeMode);

	/** 비주얼 큐 선행 Beat 수 설정 */
	void SetLookAheadBeats(float InLookAheadBeats);

	/** 비주얼 큐 선행 시간(ms) 설정 */
	void SetCueLeadTimeMs(float InCueLeadTimeMs);

	/** 마디당 Beat 수 설정 */
	void SetBeatsPerBar(int32 InBeatsPerBar);

	/** 리듬 동기화 컴포넌트 설정 */
	void SetRhythmSyncComponent(UPTBWwiseRhythmSyncComponent* InRhythmSyncComponent);

	/** 현재 채보 Asset */
	UPROPERTY()
	TObjectPtr<UPTBRhythmChartAsset> ChartAsset;

	/** Wwise 시간 공급자 */
	UPROPERTY()
	TObjectPtr<UPTBWwiseAudioManager> AudioManager;

	/** Wwise 재생 위치 기반 동기화 컴포넌트 */
	UPROPERTY()
	TObjectPtr<UPTBWwiseRhythmSyncComponent> RhythmSyncComponent;

	/**	현재채보 */
	FPTBChartData ChartData;
	/**	현재 음악 위치 (Beat) */
	float CurrentBeat;
	/**	현재 음악 시간 (ms) */
	float CurrentTimeMs;
	/**	BGM Wwise 재생 ID */
	int32 WwisePlayingId;
	/**	다음 발행 노트 인덱스 */
	int32 NextNoteIndex;
	/** 다음 선행 Cue 노트 인덱스 */
	int32 NextCueIndex;
	/** 다음 판정 등록 노트 인덱스 */
	int32 NextArmIndex;
	/** 마지막 Beat Tick 인덱스 */
	int32 LastBeatTickIndex;
	/** 마지막 Bar Tick 인덱스 */
	int32 LastBarTickIndex;
	/** 마디당 Beat 수 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm|Time", meta = (ClampMin = "1", UIMin = "1"))
	int32 BeatsPerBar = 4;
	/**	비주얼 큐 선행 Beat 수 */
	float LookAheadBeats = 2.0;
	/** 비주얼 큐 선행 시간 기준 */
	EPTBCueLeadTimeMode CueLeadTimeMode = EPTBCueLeadTimeMode::Beat;
	/** 비주얼 큐 선행 시간(ms) */
	float CueLeadTimeMs = 2000.0f;
	/** 판정 등록 선행 시간(ms) */
	float ArmLeadTimeMs = 120.0f;
	/**	채보 오프셋 */
	float ChartOffsetMs;
	/**	Wwise 뮤직 콜백 사용 */
	bool bUseMusicCallbacks = true;
	/** 재생 중 여부 */
	bool bIsPlaying;
	/** 일시정지 여부 */
	bool bIsPaused;
	/** 모든 노트 발행 완료 여부 */
	bool bAllNotesPassed;
		

	/** 판정 등록 가능 상태 진입 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Note")
	FOnNoteArm OnNoteArm;

	/** 선행 연출용 노트 Cue */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Note")
	FOnNoteCue OnNoteCue;

	/** 노트 판정선 도달 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Note")
	FOnNoteEvent OnNoteEvent;

	/** Beat Tick 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Time")
	FOnBeatTick OnBeatTick;

	/** Bar Tick 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Time")
	FOnBarTick OnBarTick;

	/** 모든 노트 발행 완료 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnAllNotesPassed OnAllNotesPassed;
};
