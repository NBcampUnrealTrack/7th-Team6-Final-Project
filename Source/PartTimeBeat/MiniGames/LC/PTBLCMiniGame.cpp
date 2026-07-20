

#include "PTBLCMiniGame.h"

#include "Components/BoxComponent.h"
#include "Debug/PTBTeamLog.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBLCLogisticBox.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"


APTBLCMiniGame::APTBLCMiniGame()
{
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
}

void APTBLCMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bEnableBeatPulse || !RhythmConductor)
	{
		ApplyBeatPulseToBoxes(1.0f);
		return;
	}

	const float CurrentBeat = RhythmConductor->GetCurrentBeat();
	const float BeatPhase = CurrentBeat - FMath::FloorToFloat(CurrentBeat);
	ApplyBeatPulseToBoxes(CalculateBeatPulseScale(BeatPhase));
}

void APTBLCMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();
	PrepareLogisticBoxPool();
}

void APTBLCMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
}

void APTBLCMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);
}

void APTBLCMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	if (IsLogisticBoxAction(Note.ActionType))
	{
		APTBLCLogisticBox* SpawnedActor = AcquireLogisticBoxFromPool();
	
		if (SpawnedActor)
		{
			SpawnedActor->ActivateFromPool(Note);
			ApplyCueSpawnDelayCompensation(SpawnedActor, Note);
			ActiveLogisticBoxes.Add(SpawnedActor);
		}
		else
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] Logistic box pool exhausted. NoteId=%d Action=%d PoolSize=%d NextIndex=%d"),
				Note.NoteId,
				static_cast<int32>(Note.ActionType),
				LogisticBoxPool.Num(),
				NextLogisticBoxPoolIndex);
		}
	}
	else PTB_WARNING(LogPTBMiniGames, TEXT("[LC] HandleNoteCue: Unsupported ActionType. NoteId=%d Action=%d"),
		Note.NoteId,
		static_cast<int32>(Note.ActionType));
}

void APTBLCMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);
	
	
	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		FPTBNoteEvent EmptyInputNote;
		OnJudgement.Broadcast(Result, EmptyInputNote);
		return;
	}
	
	if (Result.Reason == EPTBJudgementReason::Note || Result.Reason == EPTBJudgementReason::WrongInput)
	{
		FPTBNoteEvent Note;
		OnJudgement.Broadcast(Result, Note);
		
		if (CollisionBox)
		{
			TArray<AActor*> OverlappingActors;
			CollisionBox->GetOverlappingActors(OverlappingActors);
			if (OverlappingActors.Num() == 0)
			{
				return;
			}
			
			APTBLCLogisticBox* TargetLogisticBox = nullptr;
			for (AActor* OverlappingActor : OverlappingActors)
			{
				APTBLCLogisticBox* LogisticBox = Cast<APTBLCLogisticBox>(OverlappingActor);
				if (LogisticBox && !LogisticBox->GetIsPackaged() && LogisticBox->GetNoteId() == Result.NoteId)
				{
					TargetLogisticBox = LogisticBox;
					break;
				}
			}

			const bool bShouldPackage =
				Result.Reason == EPTBJudgementReason::WrongInput ||
				Result.JudgementType == EPTBJudgementType::Good ||
				Result.JudgementType == EPTBJudgementType::Perfect ||
				Result.JudgementType == EPTBJudgementType::HighPerfect;
			if (TargetLogisticBox && bShouldPackage)
			{
				const EPTBActionType PackageActionType = Result.Reason == EPTBJudgementReason::WrongInput
					? Result.InputActionType
					: Result.ActionType;
				TargetLogisticBox->PackageWithActionType(PackageActionType);
			}
		}
	}
}

bool APTBLCMiniGame::IsLogisticBoxAction(EPTBActionType ActionType) const
{
	return ActionType == EPTBActionType::ActionA ||
		ActionType == EPTBActionType::ActionB ||
		ActionType == EPTBActionType::ActionC;
}

float APTBLCMiniGame::CalculateScheduledCueTimeMs(const FPTBNoteEvent& Note) const
{
	if (!RuleSet)
	{
		return Note.TimeMs;
	}

	if (RuleSet->CueLeadTimeMode == EPTBCueLeadTimeMode::MS)
	{
		return Note.TimeMs - FMath::Max(0.0f, RuleSet->CueLeadTimeMs);
	}

	const float BeatMs = GameContext.ChartData.BPM > 0.0f
		? 60000.0f / GameContext.ChartData.BPM
		: 0.0f;
	return Note.TimeMs - FMath::Max(0.0f, RuleSet->LookAheadBeats) * BeatMs;
}

void APTBLCMiniGame::ApplyCueSpawnDelayCompensation(APTBLCLogisticBox* LogisticBox, const FPTBNoteEvent& Note) const
{
	if (!bUseCueSpawnDelayCompensation || !LogisticBox)
	{
		return;
	}

	const float CurrentCueTimeMs = RhythmSyncComponent
		? RhythmSyncComponent->GetVisualChartTimeMs()
		: GetCurrentChartTimeMs();
	const float ScheduledCueTimeMs = CalculateScheduledCueTimeMs(Note);
	const float DelayMs = FMath::Clamp(CurrentCueTimeMs - ScheduledCueTimeMs, 0.0f, MaxCueSpawnCompensationMs);
	if (DelayMs <= 0.0f)
	{
		return;
	}

	const float CompensationDistance = LogisticBox->GetMovingSpeed() * (DelayMs / 1000.0f);
	LogisticBox->AddActorWorldOffset(LogisticBox->GetActorRightVector() * CompensationDistance);
}

float APTBLCMiniGame::CalculateBeatPulseScale(float BeatPhase) const
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

void APTBLCMiniGame::ApplyBeatPulseToBoxes(float PulseScale)
{
	for (int32 Index = ActiveLogisticBoxes.Num() - 1; Index >= 0; --Index)
	{
		APTBLCLogisticBox* LogisticBox = ActiveLogisticBoxes[Index].Get();
		if (!LogisticBox)
		{
			ActiveLogisticBoxes.RemoveAt(Index);
			continue;
		}

		LogisticBox->SetBeatPulseScale(PulseScale);
	}
}

void APTBLCMiniGame::PrepareLogisticBoxPool()
{
	for (TObjectPtr<APTBLCLogisticBox>& LogisticBox : LogisticBoxPool)
	{
		if (LogisticBox)
		{
			LogisticBox->Destroy();
		}
	}

	LogisticBoxPool.Reset();
	ActiveLogisticBoxes.Reset();
	NextLogisticBoxPoolIndex = 0;

	if (!LogisticBoxClass)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[LC] PrepareLogisticBoxPool skipped: LogisticBoxClass is null."));
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[LC] PrepareLogisticBoxPool skipped: World is null."));
		return;
	}

	int32 RequiredPoolSize = 0;
	if (ChartAsset)
	{
		for (const FPTBNoteEvent& Note : ChartAsset->NoteEvents)
		{
			if (IsLogisticBoxAction(Note.ActionType))
			{
				++RequiredPoolSize;
			}
		}
	}

	if (RequiredPoolSize <= 0)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector StandbyLocation = GetLogisticBoxStandbyLocation();
	const FTransform SpawnTransform(FRotator::ZeroRotator, StandbyLocation);
	LogisticBoxPool.Reserve(RequiredPoolSize);
	for (int32 Index = 0; Index < RequiredPoolSize; ++Index)
	{
		APTBLCLogisticBox* LogisticBox = World->SpawnActor<APTBLCLogisticBox>(
			LogisticBoxClass,
			SpawnTransform,
			SpawnParams
		);

		if (!LogisticBox)
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] PrepareLogisticBoxPool spawn failed. Index=%d Required=%d Class=%s"),
				Index,
				RequiredPoolSize,
				*GetNameSafe(LogisticBoxClass));
			continue;
		}

		LogisticBox->ResetForPool(StandbyLocation);
		LogisticBox->OnDestroyed.AddUniqueDynamic(this, &APTBLCMiniGame::HandleLogisticBoxDestroyed);
		LogisticBoxPool.Add(LogisticBox);
		RegisterSpawnedRoundActor(LogisticBox);
	}

	PTB_RECORD(LogPTBMiniGames, TEXT("[LC] Logistic box pool prepared. Required=%d Created=%d Class=%s"),
		RequiredPoolSize,
		LogisticBoxPool.Num(),
		*GetNameSafe(LogisticBoxClass));
}

APTBLCLogisticBox* APTBLCMiniGame::AcquireLogisticBoxFromPool()
{
	if (!LogisticBoxPool.IsValidIndex(NextLogisticBoxPoolIndex))
	{
		return nullptr;
	}

	return LogisticBoxPool[NextLogisticBoxPoolIndex++].Get();
}

FVector APTBLCMiniGame::GetLogisticBoxStandbyLocation() const
{
	return BoxSpawnLocation;
}

void APTBLCMiniGame::HandleLogisticBoxDestroyed(AActor* DestroyedActor)
{
	APTBLCLogisticBox* DestroyedLogisticBox = Cast<APTBLCLogisticBox>(DestroyedActor);
	if (!DestroyedLogisticBox)
	{
		return;
	}

	for (int32 Index = ActiveLogisticBoxes.Num() - 1; Index >= 0; --Index)
	{
		if (!ActiveLogisticBoxes[Index].IsValid() || ActiveLogisticBoxes[Index].Get() == DestroyedLogisticBox)
		{
			ActiveLogisticBoxes.RemoveAt(Index);
		}
	}
}
