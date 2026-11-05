#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Containers/Ticker.h"
#include "PTBLoadingScreenSubsystem.generated.h"

class UUserWidget;
class SWidget;

/**
 * 맵 전환 직후 로딩 화면 설정.
 * 패키징 빌드는 처음 보는 머티리얼의 PSO(셰이더 파이프라인)를 그 자리에서 컴파일해서 첫 실행 때 크게 끊김.
 * 맵이 열린 뒤 PSO 컴파일이 끝날 때까지 로딩 화면으로 가려 둠 (두 번째 실행부터는 거의 바로 사라짐).
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "PTB 로딩 화면"))
class PARTTIMEBEAT_API UPTBLoadingScreenSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	/** 맵 전환 후 PSO 컴파일 대기 로딩 화면 사용 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading")
	bool bEnable = true;

	/** 에디터(PIE)에서도 표시할지 (에디터는 셰이더 캐시가 있어 보통 필요 없음) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable"))
	bool bShowInEditor = false;

	/** 로딩 화면 위젯 (비우면 검은 화면 + "Loading..." 기본 화면) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable"))
	TSoftClassPtr<UUserWidget> LoadingWidgetClass;

	/**
	 * 맵이 열릴 때마다 항상 로딩 화면을 띄우고, 첫 프레임들이 끊김 없이 그려질 때까지 유지.
	 * 맵이 열린 직후에는 아직 PSO 요청이 안 들어와서 남은 개수로는 판단할 수 없음
	 * (첫 프레임을 그릴 때 한꺼번에 컴파일되며 끊김). 켜면 ShowThresholdPending은 무시됨.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable"))
	bool bAlwaysShowOnMapLoad = true;

	/** 로딩 화면 뒤에서 최소 이 프레임 수만큼 맵을 그린 뒤에 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "1"))
	int32 MinRenderedFrames = 5;

	/** 이 시간(초) 이하로 그려진 프레임을 '부드러운 프레임'으로 봄 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0.005"))
	float SmoothFrameSeconds = 0.05f;

	/** 부드러운 프레임이 연속으로 이 개수만큼 나와야 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "1"))
	int32 RequiredSmoothFrames = 10;

	/** 남은 PSO 컴파일이 이 개수 이하면 로딩 화면을 띄우지 않음 (bAlwaysShowOnMapLoad가 꺼져 있을 때만 사용) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0"))
	int32 ShowThresholdPending = 8;

	/** 로딩 화면 최소 표시 시간(초) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0.0"))
	float MinDisplaySeconds = 0.2f;

	/** 남은 컴파일이 0인 상태가 이 시간(초) 유지되면 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0.0"))
	float StableZeroSeconds = 0.15f;

	/** 최대 대기 시간(초). 넘으면 컴파일이 남아 있어도 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "1.0"))
	float MaxWaitSeconds = 20.0f;

	/**
	 * 게임 실행 후 첫 맵(타이틀)에서의 최대 대기 시간(초).
	 * 첫 실행은 시작 시점에 수천 개의 PSO 프리캐시가 쌓이는데, 이걸 다 끝내기 전에 다음 맵으로 넘어가면
	 * 화면에 필요한 PSO가 그 대기열 뒤에 밀려 수십 초 멈춤(실측: 프로필 생성 진입 시 55초).
	 * 그래서 첫 맵에서는 대기열이 빌 때까지 기다림. 두 번째 실행부터는 남은 개수가 0이라 바로 닫힘.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "1.0"))
	float InitialMaxWaitSeconds = 300.0f;

	/** 로딩 화면이 떠 있는 동안 프레임 제한(0이면 제한 없음). 렌더링에 쓰는 CPU를 줄여 PSO 컴파일을 빨리 끝내게 함 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0"))
	int32 LoadingMaxFPS = 30;
};

/** 맵이 열릴 때마다 PSO 컴파일 상태를 보고 필요하면 로딩 화면을 띄웠다가 끝나면 내림 */
UCLASS()
class PARTTIMEBEAT_API UPTBLoadingScreenSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 현재 남은 PSO 컴파일 수 (런타임 프리캐시 요청 + 번들 PSO 캐시) */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	static int32 GetPendingPSOCompileCount();

	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	bool IsLoadingScreenVisible() const { return bVisible; }

	/**
	 * 로딩 진행률 0~1 (줄어들지 않음). 로딩 화면이 뜬 시점의 남은 PSO 수 대비 처리된 비율.
	 * 커스텀 로딩 WBP에서 Progress Bar의 Percent / 텍스트(%)에 바인딩해서 사용.
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	float GetLoadingProgress() const { return DisplayProgress; }

	/** 진행률을 0~99 정수로 (텍스트 표시용, 100%는 표시 전에 닫히므로 99에서 멈춤) */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	int32 GetLoadingPercent() const { return FMath::Min(99, FMath::FloorToInt(DisplayProgress * 100.0f)); }

	/**
	 * 지금 로딩 화면이 무엇을 기다리는지 안내 문구.
	 * 첫 실행의 셰이더 준비(오래 걸림)와 일반 맵 로딩을 구분해서 보여줌. 커스텀 로딩 WBP의 텍스트에 바인딩.
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	FText GetLoadingStatusText() const;

	/** 첫 실행 셰이더 준비 중인지 (안내 문구/연출 분기용) */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	bool IsPreparingShaders() const;

	/**
	 * 로딩 화면을 먼저 한 프레임 그린 뒤 맵을 연다. (Open Level 대신 사용)
	 * 맵 로딩 중에는 게임이 멈추므로 마지막으로 그려진 로딩 화면이 그대로 보이고,
	 * 맵이 열린 뒤에는 PSO 컴파일이 끝날 때까지 로딩 화면이 이어짐.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Loading", meta = (WorldContext = "WorldContextObject"))
	static void OpenLevelWithLoadingScreen(const UObject* WorldContextObject, FName LevelName);

private:
	void HandlePreLoadMap(const FString& MapName);
	void HandlePostLoadMap(UWorld* LoadedWorld);
	bool TickPendingOpenLevel(float DeltaTime);
	void RemoveWidgetsOnly();
	bool Tick(float DeltaTime);

	void ShowLoadingScreen();
	void HideLoadingScreen();

	FDelegateHandle PreLoadMapHandle;
	FDelegateHandle PostLoadMapHandle;
	FTSTicker::FDelegateHandle PendingOpenTickerHandle;
	TWeakObjectPtr<UWorld> PendingOpenWorld;
	FName PendingOpenLevelName;
	int32 PendingOpenFrames = 0;
	/** OpenLevelWithLoadingScreen으로 연 맵이면 PSO가 적어도 최소 시간은 로딩 화면 유지 */
	bool bTransitionRequested = false;
	FTSTicker::FDelegateHandle TickerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoadingWidget;

	TSharedPtr<SWidget> DefaultSlateWidget;

	bool bVisible = false;
	double ShownTime = 0.0;
	double ZeroSinceTime = -1.0;
	int32 PeakPending = 0;
	float DisplayProgress = 0.0f;

	/** 로딩 화면 표시 후 Tick 횟수(= 그려진 프레임 수) */
	int32 FramesSinceShown = 0;

	/** 연속 부드러운 프레임 수 */
	int32 SmoothFrameStreak = 0;

	/** 로딩 화면 뒤에서 그려진 가장 느린 프레임(ms) — 가려진 끊김 확인용 */
	float WorstFrameMs = 0.0f;

	void ResetFrameTracking();

	/** 이번 로딩 화면의 최대 대기 시간(초) */
	float CurrentMaxWaitSeconds = 20.0f;

	/** 실행 후 첫 맵 로딩을 처리했는지 */
	bool bInitialMapHandled = false;

	/** 지금 떠 있는 로딩 화면이 실행 후 첫 맵의 것인지 */
	bool bShowingInitialMap = false;

	/** 로딩 중 프레임 제한 적용 전 t.MaxFPS 값 */
	float SavedMaxFPS = 0.0f;
	bool bMaxFPSOverridden = false;

	void ApplyLoadingFrameCap();
	void RestoreFrameCap();

	void UpdateProgress(int32 Pending);
};
