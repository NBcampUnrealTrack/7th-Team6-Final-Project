#include "PTBLCBeatPulseComponent.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "MiniGames/LC/PTBLCMiniGame.h"
#include "Rhythm/PTBRhythmChartAsset.h"

UPTBLCBeatPulseComponent::UPTBLCBeatPulseComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPTBLCBeatPulseComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!SourceMiniGame && bAutoFindSourceMiniGame)
	{
		SourceMiniGame = FindSourceMiniGame();
	}

	USceneComponent* ResolvedTargetComponent = ResolveTargetComponent();
	if (ResolvedTargetComponent)
	{
		BaseRelativeScale = ResolvedTargetComponent->GetRelativeScale3D();
	}
}

void UPTBLCBeatPulseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	USceneComponent* ResolvedTargetComponent = ResolveTargetComponent();
	if (!ResolvedTargetComponent)
	{
		return;
	}

	float BeatPhase = 0.0f;
	const float PulseScale = bEnableBeatPulse && TryGetBeatPhase(BeatPhase)
		? CalculateBeatPulseScale(BeatPhase)
		: 1.0f;
	ResolvedTargetComponent->SetRelativeScale3D(BaseRelativeScale * PulseScale);
}

APTBLCMiniGame* UPTBLCBeatPulseComponent::FindSourceMiniGame() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<APTBLCMiniGame> It(World); It; ++It)
	{
		return *It;
	}

	return nullptr;
}

USceneComponent* UPTBLCBeatPulseComponent::ResolveTargetComponent() const
{
	if (TargetComponent)
	{
		return TargetComponent;
	}

	AActor* Owner = GetOwner();
	return Owner ? Owner->GetRootComponent() : nullptr;
}

float UPTBLCBeatPulseComponent::CalculateBeatPulseScale(float BeatPhase) const
{
	const float SafeShrinkRatio = FMath::Max(0.01f, BeatPulseShrinkBeatRatio);
	const float SafeRecoverRatio = FMath::Max(0.01f, BeatPulseRecoverBeatRatio);
	const float SafeMinScale = FMath::Clamp(BeatPulseMinScale, 0.0f, 1.0f);

	if (BeatPhase < SafeShrinkRatio)
	{
		const float Alpha = FMath::Clamp(BeatPhase / SafeShrinkRatio, 0.0f, 1.0f);
		const float EasedAlpha = FMath::InterpEaseIn(0.0f, 1.0f, Alpha, BeatPulseShrinkEaseExponent);
		return FMath::Lerp(1.0f, SafeMinScale, EasedAlpha);
	}

	if (BeatPhase < SafeShrinkRatio + SafeRecoverRatio)
	{
		const float Alpha = FMath::Clamp((BeatPhase - SafeShrinkRatio) / SafeRecoverRatio, 0.0f, 1.0f);
		const float EasedAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, BeatPulseRecoverEaseExponent);
		return FMath::Lerp(SafeMinScale, 1.0f, EasedAlpha);
	}

	return 1.0f;
}

bool UPTBLCBeatPulseComponent::TryGetBeatPhase(float& OutBeatPhase) const
{
	if (!SourceMiniGame || !SourceMiniGame->ChartAsset || SourceMiniGame->ChartAsset->ChartData.BPM <= 0.0f)
	{
		return false;
	}

	const float BeatMs = 60000.0f / SourceMiniGame->ChartAsset->ChartData.BPM;
	if (BeatMs <= 0.0f)
	{
		return false;
	}

	const float CurrentBeat = SourceMiniGame->GetCurrentChartTimeMs() / BeatMs;
	OutBeatPhase = CurrentBeat - FMath::FloorToFloat(CurrentBeat);
	return true;
}
