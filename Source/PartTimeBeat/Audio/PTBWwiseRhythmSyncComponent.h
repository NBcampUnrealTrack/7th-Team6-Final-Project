#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBWwiseRhythmSyncComponent.generated.h"

class UPTBWwiseAudioManager;

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatTick, float, Beat);
//DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarTick, int32, Bar);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBWwiseRhythmSyncComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPTBWwiseRhythmSyncComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	
	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBeatTick OnBeatTick;

	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnBarTick OnBarTick;
	/** 매니저 참조 */
	UPTBWwiseAudioManager* WwiseManager;
	/** 활성 PlayingId */
	int32 CurrentPlayingId;
	/** 동기화 상태 */
	FPTBWwiseSyncData SyncData;
	/** 채보 오프셋 */
	float ChartOffsetMs;
	/** 유저 레이턴시 보상 */
	float LatencyCompensationMs;
	/** Wwise 콜백 사용 여부 */
	bool bUseMusicCallbacks;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 동기화 시작 */
	void StartSync(int32 PlayingId, float BPM, float OffsetMs);
	/** 정지 */
	void StopSync();
	/** 현재 Beat */
	float GetCurrentBeat() const;
	/** 현재 시간 (ms) */
	float GetCurrentTimeMs() const;
	/** 런타임 오프셋 조정 */
	void CalibrateOffset(float DeltaMs);
};
