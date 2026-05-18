#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBRhythmConductorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteCue, FPTBNoteEvent, NoteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNoteEvent, FPTBNoteEvent, NoteEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChartEnd);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PARTTIMEBEAT_API UPTBRhythmConductorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPTBRhythmConductorComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	// 채보 + BGM 연결 시작
	void StartConductor(const FPTBChartData& Data, int32 PlayingId);
	// 재개
	void PauseConductor();
	//	일시정지 
	void ResumeConductor();
	//	정지 및 초기화
	void StopConductor();
	//	현재 음악 시간
	float GetCurrentMusicTimeMs() const;
	//	현재 Beat
	float GetCurrentBeat() const;
	//	Wwise 콜백 수신
	//void OnWwiseMusicCallback(const FAkMusicCallbackInfo& Info);


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
	/**	비주얼 큐 선행 Beat 수 */
	float LookAheadBeats = 2.0;
	/**	채보 오프셋 */
	float ChartOffsetMs;
	/**	Wwise 뮤직 콜백 사용 */
	bool bUseMusicCallbacks = true;
		

	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Note")
	FOnNoteCue OnNoteCue;

	// 2) 노트 판정선 통과 등 이벤트 발생 시 호출
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Note")
	FOnNoteEvent OnNoteEvent;

	// 3) 매 박자(Beat)마다 현재 박자 수와 함께 호출
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Time")
	FOnBeatTick OnBeatTick;

	// 4) 매 마디(Bar)마다 현재 마디 수와 함께 호출
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Time")
	FOnBarTick OnBarTick;

	// 5) 채보가 완전히 끝났을 때 호출
	UPROPERTY(BlueprintAssignable, Category = "Rhythm")
	FOnChartEnd OnChartEnd;
};
