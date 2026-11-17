#include "Core/PTBInputPollSubsystem.h"

#include "HAL/RunnableThread.h"
#include "HAL/PlatformProcess.h"
#include "HAL/PlatformTime.h"
#include "Misc/App.h"
#include "Containers/Ticker.h"
#include "InputCoreTypes.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogPTBInputPoll, Log, All);

// ─────────────────────────────────────────────────────────────
// 폴링 스레드
// ─────────────────────────────────────────────────────────────

class FPTBInputPollRunnable : public FRunnable
{
public:
	FPTBInputPollRunnable(UPTBInputPollSubsystem* InOwner, int32 InRateHz)
		: Owner(InOwner)
		, PeriodSec(1.0 / FMath::Clamp(InRateHz, 1, 8000))
	{
	}

	virtual uint32 Run() override
	{
#if PLATFORM_WINDOWS
		bool bWasDown[5] = { false, false, false, false, false };
		double NextTime = FPlatformTime::Seconds();

		while (!bStop.load())
		{
			const double Now = FPlatformTime::Seconds();

			if (Owner->bAppFocused.load(std::memory_order_relaxed))
			{
				for (int32 i = 0; i < 5; ++i)
				{
					const uint32 VK = Owner->VirtualKeys[i].load(std::memory_order_relaxed);
					if (VK == 0)
					{
						bWasDown[i] = false;
						continue;
					}
					const bool bDown = (::GetAsyncKeyState(static_cast<int>(VK)) & 0x8000) != 0;
					if (bDown && !bWasDown[i])
					{
						Owner->LastDownSeconds[i].store(Now, std::memory_order_release);
					}
					else if (!bDown && bWasDown[i])
					{
						Owner->LastUpSeconds[i].store(Now, std::memory_order_release);
					}
					bWasDown[i] = bDown;
				}
			}
			else
			{
				// 포커스가 없으면 기록하지 않음 (다른 프로그램에서 누른 키 무시)
				for (bool& b : bWasDown) { b = false; }
			}

			// 다음 확인 시각까지 대기: 여유가 크면 Sleep, 짧으면 양보, 아주 짧으면 바쁜 대기
			NextTime += PeriodSec;
			double Remaining = NextTime - FPlatformTime::Seconds();
			if (Remaining < -0.05)
			{
				NextTime = FPlatformTime::Seconds(); // 크게 밀렸으면(절전·히치) 재정렬
				continue;
			}
			while (Remaining > 0.0 && !bStop.load(std::memory_order_relaxed))
			{
				if (Remaining > 0.0015)
				{
					FPlatformProcess::Sleep(0.001f);
				}
				else if (Remaining > 0.0002)
				{
					FPlatformProcess::YieldThread();
				}
				else
				{
					FPlatformProcess::YieldCycles(64);
				}
				Remaining = NextTime - FPlatformTime::Seconds();
			}
		}
#endif
		return 0;
	}

	virtual void Stop() override { bStop.store(true); }

private:
	UPTBInputPollSubsystem* Owner;
	double PeriodSec;
	std::atomic<bool> bStop{ false };
};

// ─────────────────────────────────────────────────────────────
// 서브시스템
// ─────────────────────────────────────────────────────────────

void UPTBInputPollSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	for (int32 i = 0; i < 5; ++i)
	{
		VirtualKeys[i].store(0);
		LastDownSeconds[i].store(0.0);
		LastUpSeconds[i].store(0.0);
	}
	FocusTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &UPTBInputPollSubsystem::FocusTick));
}

void UPTBInputPollSubsystem::Deinitialize()
{
	StopThread();
	if (FocusTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(FocusTickerHandle);
		FocusTickerHandle.Reset();
	}
	if (CorrectionCount > 0)
	{
		UE_LOG(LogPTBInputPoll, Log, TEXT("[InputPoll] 종료: 보정 %d회, 평균 %.2fms, 최대 %.2fms (Rate=%dHz)"),
			CorrectionCount, CorrectionSumMs / CorrectionCount, CorrectionMaxMs, RateHz);
	}
	Super::Deinitialize();
}

bool UPTBInputPollSubsystem::FocusTick(float DeltaTime)
{
	bAppFocused.store(FApp::HasFocus(), std::memory_order_relaxed);
	return true;
}

void UPTBInputPollSubsystem::Configure(int32 InRateHz, const FPTBRhythmKeyBindings& InKeys)
{
#if PLATFORM_WINDOWS
	// 키 바인딩 → Windows 가상 키 코드
	const EPTBActionType Actions[5] = {
		EPTBActionType::ActionA, EPTBActionType::ActionB, EPTBActionType::ActionC,
		EPTBActionType::ActionD, EPTBActionType::ActionE };
	for (int32 i = 0; i < 5; ++i)
	{
		const FKey Key = InKeys.ResolveAction(Actions[i]);
		uint32 VK = 0;
		const uint32* KeyCode = nullptr;
		const uint32* CharCode = nullptr;
		FInputKeyManager::Get().GetCodesFromKey(Key, KeyCode, CharCode);
		if (KeyCode && *KeyCode != 0)
		{
			VK = *KeyCode;
		}
		else if (CharCode && *CharCode != 0)
		{
			VK = static_cast<uint32>(FChar::ToUpper(static_cast<TCHAR>(*CharCode))); // 글자 키: VK = 대문자 코드
		}
		VirtualKeys[i].store(VK);
	}

	const int32 NewRate = FMath::Clamp(InRateHz, 0, 8000);
	if (NewRate != RateHz || (NewRate > 0 && !Thread))
	{
		StopThread();
		RateHz = NewRate;
		if (RateHz > 0)
		{
			StartThread();
		}
		UE_LOG(LogPTBInputPoll, Log, TEXT("[InputPoll] 폴링 레이트 %s"),
			RateHz > 0 ? *FString::Printf(TEXT("%dHz"), RateHz) : TEXT("끔 (프레임 기준)"));
	}
#else
	RateHz = 0;
#endif
}

void UPTBInputPollSubsystem::StartThread()
{
	Runnable = new FPTBInputPollRunnable(this, RateHz);
	Thread = FRunnableThread::Create(Runnable, TEXT("PTBInputPoll"), 0, TPri_AboveNormal);
	if (!Thread)
	{
		delete Runnable;
		Runnable = nullptr;
		RateHz = 0;
		UE_LOG(LogPTBInputPoll, Warning, TEXT("[InputPoll] 스레드 생성 실패 → 프레임 기준 판정"));
	}
}

void UPTBInputPollSubsystem::StopThread()
{
	if (Thread)
	{
		Thread->Kill(true); // Stop() 호출 후 종료 대기
		delete Thread;
		Thread = nullptr;
	}
	if (Runnable)
	{
		delete Runnable;
		Runnable = nullptr;
	}
}

bool UPTBInputPollSubsystem::ConsumeEdge(EPTBActionType Action, bool bPressed, double& OutRealSeconds)
{
	if (!Thread)
	{
		return false;
	}

	int32 Index = INDEX_NONE;
	switch (Action)
	{
	case EPTBActionType::ActionA: Index = 0; break;
	case EPTBActionType::ActionB: Index = 1; break;
	case EPTBActionType::ActionC: Index = 2; break;
	case EPTBActionType::ActionD: Index = 3; break;
	case EPTBActionType::ActionE: Index = 4; break;
	default: return false;
	}

	const double Edge = bPressed
		? LastDownSeconds[Index].load(std::memory_order_acquire)
		: LastUpSeconds[Index].load(std::memory_order_acquire);
	double& Consumed = bPressed ? ConsumedDownSeconds[Index] : ConsumedUpSeconds[Index];

	const double Now = FPlatformTime::Seconds();
	if (Edge <= 0.0 || Edge <= Consumed || Edge > Now || Now - Edge > MaxEdgeAgeSec)
	{
		return false; // 기록 없음 / 이미 사용 / 너무 오래됨 → 프레임 기준
	}

	Consumed = Edge;
	OutRealSeconds = Edge;
	return true;
}

void UPTBInputPollSubsystem::ReportCorrection(float CorrectionMs)
{
	++CorrectionCount;
	CorrectionSumMs += CorrectionMs;
	CorrectionMaxMs = FMath::Max(CorrectionMaxMs, FMath::Abs(CorrectionMs));
	UE_LOG(LogPTBInputPoll, Verbose, TEXT("[InputPoll] 판정 보정 %+.2fms"), CorrectionMs);
}
