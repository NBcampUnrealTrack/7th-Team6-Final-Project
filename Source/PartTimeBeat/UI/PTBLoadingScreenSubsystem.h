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

	/** 남은 PSO 컴파일이 이 개수 이하면 로딩 화면을 띄우지 않음 (두 번째 실행 이후 깜빡임 방지) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0"))
	int32 ShowThresholdPending = 8;

	/** 로딩 화면 최소 표시 시간(초) */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0.0"))
	float MinDisplaySeconds = 0.5f;

	/** 남은 컴파일이 0인 상태가 이 시간(초) 유지되면 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "0.0"))
	float StableZeroSeconds = 0.3f;

	/** 최대 대기 시간(초). 넘으면 컴파일이 남아 있어도 닫음 */
	UPROPERTY(Config, EditAnywhere, Category = "Loading", meta = (EditCondition = "bEnable", ClampMin = "1.0"))
	float MaxWaitSeconds = 20.0f;
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

	/** 진행률을 0~100 정수로 (텍스트 표시용) */
	UFUNCTION(BlueprintPure, Category = "PTB|Loading")
	int32 GetLoadingPercent() const { return FMath::RoundToInt(DisplayProgress * 100.0f); }

private:
	void HandlePostLoadMap(UWorld* LoadedWorld);
	bool Tick(float DeltaTime);

	void ShowLoadingScreen();
	void HideLoadingScreen();

	FDelegateHandle PostLoadMapHandle;
	FTSTicker::FDelegateHandle TickerHandle;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> LoadingWidget;

	TSharedPtr<SWidget> DefaultSlateWidget;

	bool bVisible = false;
	double ShownTime = 0.0;
	double ZeroSinceTime = -1.0;
	int32 PeakPending = 0;
	float DisplayProgress = 0.0f;

	void UpdateProgress(int32 Pending);
};
