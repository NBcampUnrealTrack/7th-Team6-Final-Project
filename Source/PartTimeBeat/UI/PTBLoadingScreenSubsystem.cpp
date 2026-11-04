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
	const int32 RuntimePrecache = static_cast<int32>(PipelineStateCache::NumActivePrecompileRequests());
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
		DefaultSlateWidget =
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor::Black)
			.HAlign(HAlign_Right)
			.VAlign(VAlign_Bottom)
			.Padding(FMargin(0.0f, 0.0f, 60.0f, 50.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Loading...")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
				.ColorAndOpacity(FLinearColor::White)
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
	PeakPending = FMath::Max(PeakPending, Pending);

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
