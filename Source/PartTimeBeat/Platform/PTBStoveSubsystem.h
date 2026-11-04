#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "PTBStoveSubsystem.generated.h"

// STOVE SDK 타입 (SDK가 없는 빌드에서도 헤더가 컴파일되도록 전방 선언만)
struct IStoveCallbackResult;
struct IStoveRestartAppIfNecessaryOutcome;

UENUM(BlueprintType)
enum class EPTBStoveState : uint8
{
	Disabled          UMETA(DisplayName = "사용 안 함"),
	CheckingLauncher  UMETA(DisplayName = "런처 확인 중"),
	Ready             UMETA(DisplayName = "연동 완료"),
	Failed            UMETA(DisplayName = "실패")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPTBStoveReady);

/**
 * STOVE PC SDK 수명 관리.
 * 게임 시작 → 런처 실행 확인(Stove_RestartAppIfNecessary) → 초기화(Stove_Initialize) → 매 프레임 Stove_RunCallback → 종료 시 Stove_Uninitialize
 *
 * 설정: Project Settings → PTB STOVE 연동
 * 런처로 실행되지 않았으면 SDK가 런처를 통해 다시 실행하고 현재 프로세스는 종료됨
 * (그래서 에디터에서는 기본적으로 꺼져 있음. 테스트는 패키징 빌드를 STOVE 런처로 실행)
 */
UCLASS()
class PARTTIMEBEAT_API UPTBStoveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category = "PTB|Stove")
	EPTBStoveState GetStoveState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "PTB|Stove")
	bool IsStoveReady() const { return State == EPTBStoveState::Ready; }

	/** 런처에 로그인한 STOVE 닉네임 (연동 완료 후) */
	UFUNCTION(BlueprintPure, Category = "PTB|Stove")
	FString GetStoveNickname() const { return Nickname; }

	/**
	 * 스탯 값 설정 (STOVE 도전과제는 연결된 스탯 값으로 달성됨).
	 * 연동 완료 전이면 모아 뒀다가 완료 직후 전송. STOVE를 안 쓰는 빌드면 false
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Stove")
	bool SetStat(const FString& StatId, int32 Value);

	/** (디버그) 내 도전과제 목록을 Output Log에 출력 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Stove")
	void DebugFetchAchievements();

	/** 연동 완료 */
	UPROPERTY(BlueprintAssignable, Category = "PTB|Stove")
	FOnPTBStoveReady OnStoveReady;

	// ── SDK 콜백에서 호출 (내부용) ──
	void HandleRestartResult(const IStoveCallbackResult* CallbackResult, const IStoveRestartAppIfNecessaryOutcome* Outcome);

private:
	bool ShouldStartStove() const;
	bool LoadSdkDll();
	void StartStove();
	void InitializeSdk();
	void FailStove(const FString& Reason);
	void SendSetStat(const FString& StatId, int32 Value);
	bool TickStove(float DeltaTime);

	EPTBStoveState State = EPTBStoveState::Disabled;
	FString Nickname;
	uint64 UserId = 0;

	/** 연동 완료 전에 들어온 스탯 (ID → 가장 큰 값) */
	TMap<FString, int32> PendingStats;

	FTSTicker::FDelegateHandle TickerHandle;
	void* SdkDllHandle = nullptr;
};
