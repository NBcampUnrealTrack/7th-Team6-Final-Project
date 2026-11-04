#include "UI/PTBLoadingScreenSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "PipelineStateCache.h"
#include "ShaderPipelineCache.h"
#include "UObject/UObjectGlobals.h"
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
	PostLoadMapHandle = FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UPTBLoadingScreenSubsystem::HandlePostLoadMap);
}

void UPTBLoadingScreenSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadMapHandle);
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
	if (Pending <= Settings->ShowThresholdPending)
	{
		PTB_VERBOSE(LogPTBUI, TEXT("[Loading] %s: 남은 PSO 컴파일 %d → 로딩 화면 생략"), *LoadedWorld->GetMapName(), Pending);
		return;
	}

	PTB_RECORD(LogPTBUI, TEXT("[Loading] %s: 남은 PSO 컴파일 %d → 로딩 화면 표시"), *LoadedWorld->GetMapName(), Pending);
	PeakPending = Pending;
	DisplayProgress = 0.0f;
	ShowLoadingScreen();
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
							return FText::FromString(FString::Printf(TEXT("Loading... %d%%"), FMath::RoundToInt(GetProgress() * 100.0f)));
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

	if (!TickerHandle.IsValid())
	{
		TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &UPTBLoadingScreenSubsystem::Tick));
	}
}

void UPTBLoadingScreenSubsystem::HideLoadingScreen()
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

	const UPTBLoadingScreenSettings* Settings = GetDefault<UPTBLoadingScreenSettings>();
	const double Now = FPlatformTime::Seconds();
	const double Elapsed = Now - ShownTime;
	const int32 Pending = GetPendingPSOCompileCount();
	UpdateProgress(Pending);

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

	const bool bDone = Elapsed >= MinDisplay && ZeroSinceTime >= 0.0 && (Now - ZeroSinceTime) >= StableZero;
	const bool bTimedOut = Elapsed >= MaxWait;
	if (bDone || bTimedOut)
	{
		PTB_RECORD(LogPTBUI, TEXT("[Loading] 로딩 화면 닫음 (%.1f초, 최대 대기 PSO %d개, 남은 %d개%s)"),
			Elapsed, PeakPending, Pending, bTimedOut ? TEXT(", 시간 초과") : TEXT(""));
		// Ticker 콜백 안에서 RemoveTicker를 부르지 않도록 핸들만 비우고 false 반환
		TickerHandle.Reset();
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
		return false;
	}

	return true;
}
