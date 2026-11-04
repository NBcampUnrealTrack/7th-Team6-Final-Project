#include "Platform/PTBStoveSubsystem.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "Misc/Paths.h"
#include "Platform/PTBStoveSettings.h"

#if WITH_STOVE_SDK
THIRD_PARTY_INCLUDES_START
#include "stove_api.h"
#include "base_guard.h"
THIRD_PARTY_INCLUDES_END

namespace PTBStoveInternal
{
	FString CallbackError(const IStoveCallbackResult* CallbackResult)
	{
		if (!CallbackResult)
		{
			return TEXT("(콜백 결과 없음)");
		}
		const IStoveResult* Result = CallbackResult->GetResult();
		const wchar_t* Message = CallbackResult->GetErrorMessage();
		return FString::Printf(TEXT("code=%u method=%u external=%d msg=%s"),
			Result ? Result->GetResultCode() : 0u,
			Result ? Result->GetMethodCode() : 0u,
			CallbackResult->GetExternalError(),
			Message ? Message : TEXT(""));
	}

	bool IsCallbackSuccess(const IStoveCallbackResult* CallbackResult)
	{
		const IStoveResult* Result = CallbackResult ? CallbackResult->GetResult() : nullptr;
		return Result && Result->IsSuccessful();
	}

	// ── SDK 콜백 (__cdecl, userData = UPTBStoveSubsystem*) ──
	// 콜백은 TickStove의 Stove_RunCallback 안에서만 호출되고, 서브시스템 종료 시 Ticker를 먼저 지우므로 userData는 항상 유효

	void __cdecl OnRestartAppIfNecessary(const IStoveCallbackResult* CallbackResult, const IStoveRestartAppIfNecessaryOutcome* Outcome)
	{
		if (UPTBStoveSubsystem* Self = static_cast<UPTBStoveSubsystem*>(CallbackResult ? CallbackResult->GetUserData() : nullptr))
		{
			Self->HandleRestartResult(CallbackResult, Outcome);
		}
	}

	void __cdecl OnSetStat(const IStoveCallbackResult* CallbackResult, const IStoveSetStatOutcome* Outcome)
	{
		if (!IsCallbackSuccess(CallbackResult))
		{
			PTB_WARNING(LogPTBProfile, TEXT("[Stove] SetStat 실패: %s"), *CallbackError(CallbackResult));
			return;
		}
		const wchar_t* Message = Outcome ? Outcome->GetErrorMessage() : nullptr;
		PTB_RECORD(LogPTBProfile, TEXT("[Stove] SetStat 완료: current=%d updated=%d %s"),
			Outcome ? Outcome->GetCurrentValue() : -1,
			Outcome && Outcome->IsUpdated() ? 1 : 0,
			Message ? Message : TEXT(""));
	}

	void __cdecl OnFetchAchievements(const IStoveCallbackResult* CallbackResult, const IStoveAchievementList* List)
	{
		if (!IsCallbackSuccess(CallbackResult) || !List)
		{
			PTB_WARNING(LogPTBProfile, TEXT("[Stove] FetchAchievements 실패: %s"), *CallbackError(CallbackResult));
			return;
		}
		PTB_RECORD(LogPTBProfile, TEXT("[Stove] 도전과제 %u개"), List->GetCount());
		for (uint32 Index = 0; Index < List->GetCount(); ++Index)
		{
			const IStoveAchievement* Achievement = List->GetAt(Index);
			if (!Achievement)
			{
				continue;
			}
			const IStoveAchievementCondition* Condition = Achievement->GetCondition();
			PTB_RECORD(LogPTBProfile, TEXT("[Stove]  %s | %s | %s | %d / %d"),
				Achievement->GetAchievementId(),
				Achievement->GetName(),
				Achievement->GetStatus(),
				Achievement->GetValue(),
				Condition ? Condition->GetGoalValue() : 0);
		}
	}
}
#endif

void UPTBStoveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	StartStove();
}

void UPTBStoveSubsystem::Deinitialize()
{
	// 콜백이 더 이상 오지 않도록 Ticker 먼저 제거
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}

#if WITH_STOVE_SDK
	if (State == EPTBStoveState::Ready)
	{
		StoveResultGuard Result(Stove_Uninitialize());
		PTB_RECORD(LogPTBProfile, TEXT("[Stove] Uninitialize (code=%u)"), Result.GetResultCode());
	}
#endif
	State = EPTBStoveState::Disabled;

	Super::Deinitialize();
}

bool UPTBStoveSubsystem::ShouldStartStove() const
{
	const UPTBStoveSettings* Settings = GetDefault<UPTBStoveSettings>();
	if (!Settings || !Settings->bEnableStove)
	{
		return false;
	}
	if (GIsEditor && !Settings->bEnableInEditor)
	{
		return false;
	}
	if (Settings->GetActiveGameId().IsEmpty() || Settings->GetActiveAppKey().IsEmpty())
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Stove] Game ID / App Key 미입력 (Project Settings → PTB STOVE 연동, 환경=%s)"), *Settings->GetActiveEnvironmentString());
		return false;
	}
	return true;
}

bool UPTBStoveSubsystem::LoadSdkDll()
{
	if (SdkDllHandle)
	{
		return true;
	}

	// 빌드 시 실행 파일 폴더(Binaries/Win64)로 복사됨
	const FString Candidates[] = {
		FPaths::Combine(FPlatformProcess::GetModulesDirectory(), TEXT("BaseSDK.dll")),
		FPaths::Combine(FPlatformProcess::BaseDir(), TEXT("BaseSDK.dll")),
		FPaths::Combine(FPaths::ProjectDir(), TEXT("Source/ThirdParty/StovePCSDK/dll/x64/BaseSDK.dll"))
	};
	for (const FString& Path : Candidates)
	{
		if (FPaths::FileExists(Path))
		{
			SdkDllHandle = FPlatformProcess::GetDllHandle(*Path);
			if (SdkDllHandle)
			{
				PTB_RECORD(LogPTBProfile, TEXT("[Stove] BaseSDK.dll 로드: %s"), *Path);
				return true;
			}
		}
	}
	return false;
}

void UPTBStoveSubsystem::StartStove()
{
#if WITH_STOVE_SDK
	if (!ShouldStartStove())
	{
		State = EPTBStoveState::Disabled;
		PTB_RECORD(LogPTBProfile, TEXT("[Stove] 사용 안 함 (설정 꺼짐 또는 에디터)"));
		return;
	}

	if (!LoadSdkDll())
	{
		FailStove(TEXT("BaseSDK.dll을 찾지 못함"));
		return;
	}

	const UPTBStoveSettings* Settings = GetDefault<UPTBStoveSettings>();

	// 콜백은 Stove_RunCallback을 돌려야 들어옴 → 런처 확인 전에 Ticker부터 시작
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UPTBStoveSubsystem::TickStove));

	StoveGuard<IStoveRestartAppIfNecessaryParam> Param = StoveMakeParam<IStoveRestartAppIfNecessaryParam>();
	if (!Param.IsValid())
	{
		FailStove(TEXT("RestartAppIfNecessaryParam 생성 실패"));
		return;
	}

	const FString Environment = Settings->GetActiveEnvironmentString();
	Param->SetEnvironment(*Environment);
	Param->SetGameId(*Settings->GetActiveGameId());
	Param->SetAppKey(*Settings->GetActiveAppKey());
	Param->SetWaitTimeMilliSec(static_cast<uint32_t>(FMath::Max(1000, Settings->RestartCheckWaitMs)));
	Param->SetLaunchStoveLauncher(Settings->bLaunchStoveLauncher);

	State = EPTBStoveState::CheckingLauncher;
	PTB_RECORD(LogPTBProfile, TEXT("[Stove] 런처 확인 시작 (env=%s, gameId=%s)"), *Environment, *Settings->GetActiveGameId());

	Stove_RestartAppIfNecessary(Param.Get(), &PTBStoveInternal::OnRestartAppIfNecessary, this);
#else
	State = EPTBStoveState::Disabled;
#endif
}

void UPTBStoveSubsystem::HandleRestartResult(const IStoveCallbackResult* CallbackResult, const IStoveRestartAppIfNecessaryOutcome* Outcome)
{
#if WITH_STOVE_SDK
	if (!PTBStoveInternal::IsCallbackSuccess(CallbackResult))
	{
		FailStove(FString::Printf(TEXT("런처 확인 실패: %s"), *PTBStoveInternal::CallbackError(CallbackResult)));
		return;
	}

	if (Outcome && Outcome->IsRestartRequired())
	{
		// 런처를 거치지 않고 실행됨 → SDK가 런처로 다시 실행하므로 현재 프로세스 종료
		PTB_RECORD(LogPTBProfile, TEXT("[Stove] 런처로 재실행 필요 → 종료"));
		FPlatformMisc::RequestExit(false);
		return;
	}

	InitializeSdk();
#endif
}

void UPTBStoveSubsystem::InitializeSdk()
{
#if WITH_STOVE_SDK
	// BaseSDK만 초기화 (View/IAP는 사용 안 함 → 파라미터 없음). 환경·Game ID·App Key는 RestartAppIfNecessary 값을 재사용
	StoveResultGuard Result(Stove_Initialize(nullptr));
	if (!Result)
	{
		FailStove(FString::Printf(TEXT("Stove_Initialize 실패 (code=%u)"), Result.GetResultCode()));
		return;
	}

	State = EPTBStoveState::Ready;

	StoveGuard<IStoveUser> User;
	StoveResultGuard UserResult(Stove_GetUser(User.Put()));
	if (UserResult && User.IsValid())
	{
		const wchar_t* Nick = User->GetNickName();
		Nickname = Nick ? FString(Nick) : FString();
		UserId = User->GetUserId();
	}
	PTB_RECORD(LogPTBProfile, TEXT("[Stove] 연동 완료: %s (userId=%llu)"), *Nickname, UserId);

	// 연동 전에 들어온 스탯 전송
	for (const TPair<FString, int32>& Pair : PendingStats)
	{
		SendSetStat(Pair.Key, Pair.Value);
	}
	PendingStats.Reset();

	OnStoveReady.Broadcast();
#endif
}

void UPTBStoveSubsystem::FailStove(const FString& Reason)
{
	State = EPTBStoveState::Failed;
	PendingStats.Reset();
	PTB_ERROR(LogPTBProfile, TEXT("[Stove] %s"), *Reason);

#if UE_BUILD_SHIPPING
	const UPTBStoveSettings* Settings = GetDefault<UPTBStoveSettings>();
	if (Settings && Settings->bQuitOnInitFailureInShipping)
	{
		FPlatformMisc::RequestExit(false);
	}
#endif
}

bool UPTBStoveSubsystem::TickStove(float DeltaTime)
{
#if WITH_STOVE_SDK
	Stove_RunCallback();
#endif
	return true;
}

bool UPTBStoveSubsystem::SetStat(const FString& StatId, int32 Value)
{
	if (StatId.IsEmpty())
	{
		return false;
	}

	switch (State)
	{
	case EPTBStoveState::Ready:
		SendSetStat(StatId, Value);
		return true;
	case EPTBStoveState::CheckingLauncher:
	{
		int32& Pending = PendingStats.FindOrAdd(StatId, Value);
		Pending = FMath::Max(Pending, Value);
		return true;
	}
	default:
		return false;
	}
}

void UPTBStoveSubsystem::SendSetStat(const FString& StatId, int32 Value)
{
#if WITH_STOVE_SDK
	StoveGuard<IStoveSetStatParam> Param = StoveMakeParam<IStoveSetStatParam>();
	if (!Param.IsValid())
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Stove] SetStatParam 생성 실패"));
		return;
	}
	Param->SetStatsId(*StatId);
	Param->SetStatValue(Value);
	PTB_RECORD(LogPTBProfile, TEXT("[Stove] SetStat 요청: %s = %d"), *StatId, Value);
	Stove_SetStat(Param.Get(), &PTBStoveInternal::OnSetStat, this);
#endif
}

void UPTBStoveSubsystem::DebugFetchAchievements()
{
#if WITH_STOVE_SDK
	if (State != EPTBStoveState::Ready)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Stove] 연동 전이라 도전과제 조회 불가 (state=%d)"), static_cast<int32>(State));
		return;
	}
	StoveGuard<IStoveFetchAchievementsParam> Param = StoveMakeParam<IStoveFetchAchievementsParam>();
	if (!Param.IsValid())
	{
		return;
	}
	Stove_FetchAchievements(Param.Get(), &PTBStoveInternal::OnFetchAchievements, this);
#endif
}

// ── 콘솔 명령어 (Shipping 제외) ──────────────────────────────────

#if WITH_STOVE_SDK && !UE_BUILD_SHIPPING
namespace PTBStoveConsole
{
	UPTBStoveSubsystem* Get(UWorld* World)
	{
		UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UPTBStoveSubsystem>() : nullptr;
	}

	static FAutoConsoleCommandWithWorldAndArgs GStoveStatus(TEXT("PTB.Stove.Status"), TEXT("STOVE 연동 상태 출력"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UPTBStoveSubsystem* Stove = Get(World))
			{
				PTB_RECORD(LogPTBProfile, TEXT("[Stove] state=%s nickname=%s"),
					*UEnum::GetValueAsString(Stove->GetStoveState()), *Stove->GetStoveNickname());
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GStoveAchievements(TEXT("PTB.Stove.Achievements"), TEXT("STOVE 도전과제 목록 출력"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			if (UPTBStoveSubsystem* Stove = Get(World))
			{
				Stove->DebugFetchAchievements();
			}
		}));

	static FAutoConsoleCommandWithWorldAndArgs GStoveSetStat(TEXT("PTB.Stove.SetStat"), TEXT("스탯 직접 설정: PTB.Stove.SetStat ACH_CLEAR_EASY 1"),
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
		{
			UPTBStoveSubsystem* Stove = Get(World);
			if (Stove && Args.Num() >= 2)
			{
				Stove->SetStat(Args[0], FCString::Atoi(*Args[1]));
			}
		}));
}
#endif
