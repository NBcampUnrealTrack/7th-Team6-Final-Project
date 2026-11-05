#include "UI/PTBLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "PipelineStateCache.h"
#include "ShaderPipelineCache.h"
#include "UObject/UObjectGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Styling/CoreStyle.h"
#include "Debug/PTBLogChannels.h"
#include "Debug/PTBTeamLog.h"

void UPTBLoadingScreenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UPTBLoadingScreenSubsystem::HandlePreLoadMap);
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UPTBLoadingScreenSubsystem::HandlePostLoadMap);
}

void UPTBLoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.Remove(PreLoadMapHandle);
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
	if (PendingOpenTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(PendingOpenTickerHandle);
		PendingOpenTickerHandle.Reset();
	}
	HideLoadingScreen();
	Super::Deinitialize();
}

int32 UPTBLoadingScreenSubsystem::GetPendingPSOCompileCount()
{
	const int32 RuntimePrecache = PipelineStateCache::GetNumActivePipelinePrecompileTasks(); // UE 5.6: 진행 중인 PSO 프리컴파일 작업 수
	const int32 BundledCache = static_cast<int32>(FShaderPipelineCache::NumPrecompilesRemaining());
	return RuntimePrecache + BundledCache;
}

void UPTBLoadingScreenSubsystem::HandlePostLoadMap(UWorld* LoadedWorld)
{
	const UPTBLoadingScreenSettings* Settings = GetDefault<UPTBLoadingScreenSettings>();
	if (!Settings || !Settings->bEnable || !LoadedWorld || !LoadedWorld->IsGameWorld())
	{
		return;
	}
	if (GIsEditor && !Settings->bShowInEditor)
	{
		return;
	}

	const int32 Pending = GetPendingPSOCompileCount();
	const bool bWasTransition = bTransitionRequested;
	bTransitionRequested = false;
	ResetFrameTracking();
	if (!Settings->bAlwaysShowOnMapLoad && Pending <= Settings->ShowThresholdPending && !bWasTransition)
	{
		PTB_VERBOSE(LogPTBUI, TEXT("[Loading] %s: 남은 PSO 컴파일 %d → 로딩 화면 생략"), *LoadedWorld->GetMapName(), Pending);
		return;
	}

	PTB_RECORD(LogPTBUI, TEXT("[Loading] %s: 남은 PSO 컴파일 %d → 로딩 화면 표시"), *LoadedWorld->GetMapName(), Pending);
	PeakPending = Pending;
	DisplayProgress = 0.0f;
	ShowLoadingScreen();
}

void UPTBLoadingScreenSubsystem::OpenLevelWithLoadingScreen(const UObject* WorldContextObject, FName LevelName)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World || LevelName.IsNone())
	{
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	UPTBLoadingScreenSubsystem* Self = GI ? GI->GetSubsystem<UPTBLoadingScreenSubsystem>() : nullptr;
	const UPTBLoadingScreenSettings* Settings = GetDefault<UPTBLoadingScreenSettings>();
	const bool bUseLoading = Self && Settings && Settings->bEnable && (!GIsEditor || Settings->bShowInEditor);
	if (!bUseLoading)
	{
		UGameplayStatics::OpenLevel(World, LevelName);
		return;
	}

	// 로딩 화면을 띄우고 최소 2프레임 그린 뒤(화면에 확실히 표시된 뒤) 맵을 연다
	Self->PeakPending = 0;
	Self->DisplayProgress = 0.0f;
	Self->ShowLoadingScreen();
	Self->PendingOpenWorld = World;
	Self->PendingOpenLevelName = LevelName;
	Self->PendingOpenFrames = 0;
	if (!Self->PendingOpenTickerHandle.IsValid())
	{
		Self->PendingOpenTickerHandle = FTSTicker::GetCoreTicker().AddTicker(
			FTickerDelegate::CreateUObject(Self, &UPTBLoadingScreenSubsystem::TickPendingOpenLevel));
	}
	PTB_RECORD(LogPTBUI, TEXT("[Loading] 로딩 화면 표시 후 맵 열기 예약: %s"), *LevelName.ToString());
}

bool UPTBLoadingScreenSubsystem::TickPendingOpenLevel(float DeltaTime)
{
	if (++PendingOpenFrames < 3)
	{
		return true;
	}

	PendingOpenTickerHandle.Reset();
	UWorld* World = PendingOpenWorld.Get();
	const FName LevelName = PendingOpenLevelName;
	PendingOpenWorld.Reset();
	PendingOpenLevelName = NAME_None;

	if (World)
	{
		bTransitionRequested = true;
		UGameplayStatics::OpenLevel(World, LevelName);
	}
	return false;
}

void UPTBLoadingScreenSubsystem::HandlePreLoadMap(const FString& MapName)
{
	// 맵을 열면 엔진이 뷰포트 위젯을 전부 지움 → 우리 쪽 참조도 정리 (화면은 마지막 프레임이 남아 보임)
	if (bVisible)
	{
		RemoveWidgetsOnly();
	}
}

void UPTBLoadingScreenSubsystem::RemoveWidgetsOnly()
{
	if (TickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
		TickerHandle.Reset();
	}
	if (LoadingWidget)
	{
		LoadingWidget->RemoveFromParent();
		LoadingWidget = nullptr;
	}
	if (DefaultSlateWidget.IsValid())
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->RemoveViewportWidgetContent(DefaultSlateWidget.ToSharedRef());
		}
		DefaultSlateWidget.Reset();
	}
	bVisible = false;
}

void UPTBLoadingScreenSubsystem::ShowLoadingScreen()
{
	if (bVisible)
	{
		ShownTime = FPlatformTime::Seconds();
		ZeroSinceTime = -1.0;
		return;
	}

	const UPTBLoadingScreenSettings* Settings = GetDefault<UPTBLoadingScreenSettings>();
	UClass* WidgetClass = Settings ? Settings->LoadingWidgetClass.LoadSynchronous() : nullptr;

	if (WidgetClass)
	{
		LoadingWidget = CreateWidget<UUserWidget>(GetGameInstance(), WidgetClass);
		if (LoadingWidget)
		{
			LoadingWidget->AddToViewport(10000);
		}
	}

	if (!LoadingWidget && GEngine && GEngine->GameViewport)
	{
		TWeakObjectPtr<UPTBLoadingScreenSubsystem> WeakThis(this);
		auto GetProgress = [WeakThis]() -> float
		{
			return WeakThis.IsValid() ? WeakThis->GetLoadingProgress() : 0.0f;
		};

		DefaultSlateWidget =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SOverlay)
				// 가운데: 돌아가는 원
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.WidthOverride(96.0f)
					.HeightOverride(96.0f)
					[
						SNew(SCircularThrobber)
						.Radius(48.0f)
						.NumPieces(10)
						.Period(0.9f)
					]
				]
				// 아래: Loading... NN% + 진행 바
				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Bottom)
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 80.0f))
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(FMargin(0.0f, 0.0f, 0.0f, 12.0f))
					[
						SNew(STextBlock)
						.Text_Lambda([GetProgress]()
						{
							return FText::FromString(FString::Printf(TEXT("Loading... %d%%"), FMath::Min(99, FMath::FloorToInt(GetProgress() * 100.0f))));
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
						.ColorAndOpacity(FLinearColor::White)
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(600.0f)
						.HeightOverride(14.0f)
						[
							SNew(SProgressBar)
							.Percent_Lambda([GetProgress]() -> TOptional<float> { return GetProgress(); })
							.FillColorAndOpacity(FLinearColor(1.0f, 0.75f, 0.2f))
						]
					]
				]
			];
		GEngine->GameViewport->AddViewportWidgetContent(DefaultSlateWidget.ToSharedRef(), 10000);
	}

	bVisible = true;
	ShownTime = FPlatformTime::Seconds();
	ZeroSinceTime = -1.0;
	ResetFrameTracking();

	if (!TickerHandle.IsValid())
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UPTBLoadingScreenSubsystem::Tick));
	}
}

void UPTBLoadingScreenSubsystem::ResetFrameTracking()
{
	FramesSinceShown = 0;
	SmoothFrameStreak = 0;
	WorstFrameMs = 0.0f;
}

void UPTBLoadingScreenSubsystem::HideLoadingScreen()
{
	RemoveWidgetsOnly();
}

void UPTBLoadingScreenSubsystem::UpdateProgress(int32 Pending)
{
	// 로딩 중에 새 컴파일 요청이 추가되면 전체 개수(Peak)를 늘려서 계산 → 진행률은 절대 뒤로 가지 않게 Max 유지
	PeakPending = FMath::Max(PeakPending, Pending);
	const float Raw = PeakPending > 0 ? 1.0f - static_cast<float>(Pending) / static_cast<float>(PeakPending) : 1.0f;
	DisplayProgress = FMath::Clamp(FMath::Max(DisplayProgress, Raw), 0.0f, 1.0f);
}

bool UPTBLoadingScreenSubsystem::Tick(float DeltaTime)
{
	if (!bVisible)
	{
		TickerHandle.Reset();
		return false;
	}

	// 맵 열기 대기 중(OpenLevelWithLoadingScreen)에는 닫지 않음
	if (PendingOpenTickerHandle.IsValid() || bTransitionRequested)
	{
		return true;
	}

	const UPTBLoadingScreenSettings* Settings = GetDefault<UPTBLoadingScreenSettings>();
	const double Now = FPlatformTime::Seconds();
	const double Elapsed = Now - ShownTime;
	const int32 Pending = GetPendingPSOCompileCount();
	UpdateProgress(Pending);

	// 첫 Tick의 DeltaTime에는 맵 로딩 시간 자체가 들어 있으므로 프레임 품질 판단에서 제외
	++FramesSinceShown;
	if (FramesSinceShown > 1)
	{
		WorstFrameMs = FMath::Max(WorstFrameMs, DeltaTime * 1000.0f);
		const float SmoothLimit = Settings ? Settings->SmoothFrameSeconds : 0.05f;
		SmoothFrameStreak = DeltaTime <= SmoothLimit ? SmoothFrameStreak + 1 : 0;
	}

	if (Pending <= 0)
	{
		if (ZeroSinceTime < 0.0)
		{
			ZeroSinceTime = Now;
		}
	}
	else
	{
		ZeroSinceTime = -1.0;
	}

	const float MinDisplay = Settings ? Settings->MinDisplaySeconds : 0.5f;
	const float StableZero = Settings ? Settings->StableZeroSeconds : 0.3f;
	const float MaxWait = Settings ? Settings->MaxWaitSeconds : 20.0f;

	const int32 MinFrames = Settings ? Settings->MinRenderedFrames : 5;
	const int32 NeedSmooth = Settings ? Settings->RequiredSmoothFrames : 10;
	const bool bFramesStable = FramesSinceShown >= MinFrames && SmoothFrameStreak >= NeedSmooth;

	const bool bDone = Elapsed >= MinDisplay && ZeroSinceTime >= 0.0 && (Now - ZeroSinceTime) >= StableZero && bFramesStable;
	const bool bTimedOut = Elapsed >= MaxWait;
	if (bDone || bTimedOut)
	{
		PTB_RECORD(LogPTBUI, TEXT("[Loading] 로딩 화면 닫음 (%.1f초, 최대 대기 PSO %d개, 남은 %d개, 프레임 %d, 가려진 최대 프레임 %.0fms%s)"),
			Elapsed, PeakPending, Pending, FramesSinceShown, WorstFrameMs, bTimedOut ? TEXT(", 시간 초과") : TEXT(""));
		// Ticker 콜백 안에서는 RemoveTicker를 부르지 않도록 핸들을 먼저 비우고 위젯만 정리한 뒤 false 반환
		TickerHandle.Reset();
		RemoveWidgetsOnly();
		return false;
	}

	return true;
}
