#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "HAL/Runnable.h"
#include "Containers/Ticker.h"
#include <atomic>
#include "Core/PTBStructEnums.h"
#include "PTBInputPollSubsystem.generated.h"

class FRunnableThread;
class FPTBInputPollRunnable;

/**
 * 리듬 키 입력 폴링 (Windows 전용).
 *
 * 별도 스레드가 설정한 주기(125~8000Hz)로 리듬 키(A~E 바인딩) 상태를 확인해
 * 눌림/뗌이 일어난 "정확한 시각"만 기록한다. 실제 입력 처리는 기존 언리얼 입력 경로 그대로이고,
 * PlayerController가 판정할 때 이 시각으로 판정 시간을 보정한다(프레임 단위 오차 감소).
 * 기록이 없거나 오래됐으면 보정하지 않으므로, 실패해도 기존 동작과 같다.
 *
 * 폴링 레이트 0 = 끔(스레드 없음, 프레임 기준 판정).
 */
UCLASS()
class PARTTIMEBEAT_API UPTBInputPollSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 폴링 레이트(Hz, 0=끔)와 키 바인딩 적용. 설정이 바뀔 때마다 호출 */
	void Configure(int32 InRateHz, const FPTBRhythmKeyBindings& InKeys);

	/**
	 * 해당 액션 키의 가장 최근 눌림(bPressed) 또는 뗌 시각을 가져오고 사용 처리.
	 * 아직 사용하지 않았고, 최근(MaxEdgeAgeSec 이내) 기록이 있을 때만 true.
	 */
	bool ConsumeEdge(EPTBActionType Action, bool bPressed, double& OutRealSeconds);

	/** 보정량 통계 (로그용) */
	void ReportCorrection(float CorrectionMs);

	UFUNCTION(BlueprintPure, Category = "PTB|Input")
	int32 GetPollingRateHz() const { return RateHz; }

	/** 기록이 이보다 오래되면 보정에 쓰지 않음(초) */
	static constexpr double MaxEdgeAgeSec = 0.1;

private:
	friend class FPTBInputPollRunnable;

	void StartThread();
	void StopThread();
	bool FocusTick(float DeltaTime);

	int32 RateHz = 0;

	/** 액션 A~E의 가상 키 코드 (0 = 추적 안 함). 스레드에서 읽음 */
	std::atomic<uint32> VirtualKeys[5];

	/** 마지막 눌림/뗌 시각 (스레드가 씀). 사용 여부는 Consumed 시각으로 판단 */
	std::atomic<double> LastDownSeconds[5];
	std::atomic<double> LastUpSeconds[5];
	double ConsumedDownSeconds[5] = { 0, 0, 0, 0, 0 };
	double ConsumedUpSeconds[5] = { 0, 0, 0, 0, 0 };

	/** 게임 창이 포커스를 가졌는지 (게임 스레드가 갱신, 폴링 스레드가 읽음) */
	std::atomic<bool> bAppFocused{ true };

	FPTBInputPollRunnable* Runnable = nullptr;
	FRunnableThread* Thread = nullptr;
	FTSTicker::FDelegateHandle FocusTickerHandle;

	// 로그 통계
	int32 CorrectionCount = 0;
	double CorrectionSumMs = 0.0;
	float CorrectionMaxMs = 0.0f;
};
