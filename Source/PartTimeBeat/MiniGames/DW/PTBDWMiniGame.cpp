#include "MiniGames/DW/PTBDWMiniGame.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "MiniGames/DW/PTBDWMiniGameRuleSet.h"
#include "MiniGames/DW/PTBDWNoteMarker.h"
#include "MiniGames/DW/PTBDWCharacter.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Rhythm/PTBRhythmChartAsset.h"

namespace
{
	float DefaultObstacleOffset(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return 300.f;
		case EPTBActionType::ActionB: return 120.f;
		default:                      return 200.f;
		}
	}

	FLinearColor DefaultObstacleColor(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return FLinearColor(1.0f, 0.25f, 0.25f);
		case EPTBActionType::ActionB: return FLinearColor(0.25f, 0.55f, 1.0f);
		case EPTBActionType::ActionC: return FLinearColor(0.30f, 1.00f, 0.40f);
		case EPTBActionType::ActionD: return FLinearColor(1.00f, 0.90f, 0.25f);
		case EPTBActionType::ActionE: return FLinearColor(1.00f, 0.35f, 1.00f);
		default:                      return FLinearColor::White;
		}
	}
}

bool APTBDWMiniGame::IsSupportedAction(EPTBActionType Action)
{
	return Action == EPTBActionType::ActionA || Action == EPTBActionType::ActionB;
}

void APTBDWMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!RhythmSyncComponent)
	{
		return;
	}

	const float VisualNow = RhythmSyncComponent->GetVisualChartTimeMs();

	for (TPair<int32, FDWNoteView>& Pair : ActiveNoteViews)
	{
		FDWNoteView& View = Pair.Value;

		const float Alpha = FMath::Clamp((VisualNow - View.SpawnVisualMs) * View.InvDuration, 0.f, 1.f);
		const FVector MarkerPos = FMath::Lerp(View.SpawnPos, View.LinePos, Alpha);

		if (View.Marker)
		{
			View.Marker->SetActorLocation(MarkerPos);

			const FVector CurrentScale = View.MarkerBaseScale * (1.0f - Alpha);
			View.Marker->SetActorScale3D(CurrentScale);
		}
		if (View.Obstacle)
		{
			View.Obstacle->SetActorLocation(MarkerPos + View.ObstacleOffsetVec);
		}
	}
}

void APTBDWMiniGame::StartMiniGame()
{
	Super::StartMiniGame();

	if (Protagonist) { Protagonist->StartRunning(); }
	if (Dog)         { Dog->StartRunning(); }
}

void APTBDWMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	ClearAllNoteViews();
	SpawnCharactersIfNeeded();

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW BuildRuntimeState RuleSet=%s Chart=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()));
}

void APTBDWMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	SpawnNoteView(Note);

	if (Dog && IsSupportedAction(Note.ActionType))
	{
		Dog->PlayCue(Note.ActionType);
	}

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW NoteCue NoteId=%d Action=%d NoteType=%d Long=%d Beat=%.3f TimeMs=%.3f"),
		*GetNameSafe(this),
		Note.NoteId,
		static_cast<int32>(Note.ActionType),
		static_cast<int32>(Note.NoteType),
		Note.bIsLongNote ? 1 : 0,
		Note.BeatTime,
		Note.TimeMs);
}

void APTBDWMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);

	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		PTB_VERBOSE(LogPTBMiniGames,
			TEXT("[%s] DW EmptyInput Action=%d"),
			*GetNameSafe(this),
			static_cast<int32>(Result.ActionType));
		return;
	}

	if (Protagonist && IsSupportedAction(Result.ActionType))
	{
		const bool bSuccess = (Result.JudgementType != EPTBJudgementType::Miss);
		Protagonist->PlayReaction(bSuccess, Result.ActionType);
	}

	RecycleNoteView(Result.NoteId);

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW Judgement NoteId=%d Action=%d Type=%d Reason=%d DeltaMs=%.3f ScoreDelta=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		Result.DeltaMs,
		Result.ScoreDelta);
}

void APTBDWMiniGame::SpawnNoteView(const FPTBNoteEvent& Note)
{
	UWorld* World = GetWorld();
	if (!World || !RhythmSyncComponent)
	{
		return;
	}

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	const float SpawnDistance = DWRule ? DWRule->MarkerSpawnDistance : 1200.0f;

	const FVector ForwardDir = GetActorForwardVector();
	const FVector LinePos = GetActorLocation();
	const FVector SpawnPos = LinePos + ForwardDir * SpawnDistance;

	const FVector OffsetVec = ForwardDir * GetObstacleOffset(Note.ActionType);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APTBDWNoteMarker* Marker = World->SpawnActor<APTBDWNoteMarker>(SpawnPos, FRotator::ZeroRotator, Params);
	if (!Marker)
	{
		return;
	}
	FVector MarkerScl = FVector(0.3f); 
	{
		UMaterialInterface* MarkerMat = DWRule ? DWRule->MarkerMaterial.Get() : nullptr;
		const FLinearColor MarkerCol = DWRule ? DWRule->MarkerColor : FLinearColor(0.9f, 0.9f, 1.0f, 0.35f);
		MarkerScl = DWRule ? DWRule->MarkerScale : FVector(0.3f);
		Marker->Configure(nullptr, MarkerMat, MarkerCol, MarkerScl); // nullptr=기본 구체 유지
	}

	APTBDWNoteMarker* Obstacle = World->SpawnActor<APTBDWNoteMarker>(SpawnPos + OffsetVec, FRotator::ZeroRotator, Params);
	if (Obstacle)
	{
		UMaterialInterface* BaseMat = DWRule ? DWRule->ObstacleMaterial.Get() : nullptr;
		Obstacle->Configure(GetObstacleMesh(Note.ActionType), BaseMat, GetObstacleColor(Note.ActionType), GetObstacleScale(Note.ActionType));
	}

	FDWNoteView View;
	View.Marker            = Marker;
	View.Obstacle          = Obstacle;
	View.MarkerBaseScale   = MarkerScl;
	View.SpawnVisualMs     = RhythmSyncComponent->GetVisualChartTimeMs();
	View.TargetMs          = Note.TimeMs;
	View.InvDuration       = 1.0f / FMath::Max(1.0f, View.TargetMs - View.SpawnVisualMs);
	View.SpawnPos          = SpawnPos;
	View.LinePos           = LinePos;
	View.ObstacleOffsetVec = OffsetVec;

	ActiveNoteViews.Add(Note.NoteId, View);
}

void APTBDWMiniGame::SpawnCharactersIfNeeded()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();

	const FVector Origin = GetActorLocation();
	const FVector Fwd = GetActorForwardVector();
	const FVector Right = GetActorRightVector();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (!Protagonist)
	{
		Protagonist = World->SpawnActor<APTBDWCharacter>(Origin, FRotator::ZeroRotator, Params);
		if (Protagonist)
		{
			USkeletalMesh* Mesh = DWRule ? DWRule->ProtagonistMesh.Get() : nullptr;
			const FVector Scale = DWRule ? DWRule->ProtagonistScale : FVector(1.0f);
			const float Yaw = DWRule ? DWRule->ProtagonistYaw : 0.f;
			Protagonist->InitPlaceholder(Mesh, Scale, Yaw, FLinearColor::White, nullptr);
			Protagonist->SetAnimations(
				DWRule ? DWRule->ProtagonistRunAnim.Get() : nullptr,
				DWRule ? DWRule->ProtagonistJumpAnim.Get() : nullptr);
		}
	}

	if (!Dog)
	{
		const FVector DogLoc = Origin + Fwd * 600.f + Right * 180.f;
		Dog = World->SpawnActor<APTBDWCharacter>(DogLoc, FRotator::ZeroRotator, Params);
		if (Dog)
		{
			USkeletalMesh* Mesh = DWRule ? DWRule->DogMesh.Get() : nullptr;
			const FVector Scale = DWRule ? DWRule->DogScale : FVector(1.0f);
			const float Yaw = DWRule ? DWRule->DogYaw : 0.f;
			Dog->InitPlaceholder(Mesh, Scale, Yaw, FLinearColor::White, nullptr);
			Dog->SetAnimations(DWRule ? DWRule->DogRunAnim.Get() : nullptr, nullptr);
		}
	}
}

void APTBDWMiniGame::RecycleNoteView(int32 NoteId)
{
	FDWNoteView View;
	if (ActiveNoteViews.RemoveAndCopyValue(NoteId, View))
	{
		if (View.Marker)   { View.Marker->Destroy(); }
		if (View.Obstacle) { View.Obstacle->Destroy(); }
	}
}

void APTBDWMiniGame::ClearAllNoteViews()
{
	for (TPair<int32, FDWNoteView>& Pair : ActiveNoteViews)
	{
		if (Pair.Value.Marker)   { Pair.Value.Marker->Destroy(); }
		if (Pair.Value.Obstacle) { Pair.Value.Obstacle->Destroy(); }
	}
	ActiveNoteViews.Reset();
}

float APTBDWMiniGame::GetObstacleOffset(EPTBActionType Action) const
{
	if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
	{
		if (const float* Found = DWRule->ObstacleOffsetByAction.Find(Action))
		{
			return *Found;
		}
	}
	return DefaultObstacleOffset(Action);
}

FLinearColor APTBDWMiniGame::GetObstacleColor(EPTBActionType Action) const
{
	if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
	{
		if (const FLinearColor* Found = DWRule->ObstacleColorByAction.Find(Action))
		{
			return *Found;
		}
	}
	return DefaultObstacleColor(Action);
}

FVector APTBDWMiniGame::GetObstacleScale(EPTBActionType Action) const
{
	if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
	{
		if (const FVector* Found = DWRule->ObstacleScaleByAction.Find(Action))
		{
			return *Found;
		}
	}
	return FVector(0.5f);
}

UStaticMesh* APTBDWMiniGame::GetObstacleMesh(EPTBActionType Action)
{
	if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
	{
		if (const TObjectPtr<UStaticMesh>* Found = DWRule->ObstacleMeshByAction.Find(Action))
		{
			if (*Found)
			{
				return *Found;
			}
		}
	}

	if (!CachedObstacleMesh)
	{
		CachedObstacleMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	return CachedObstacleMesh;
}

const UPTBDWMiniGameRuleSet* APTBDWMiniGame::GetDWRuleSet() const
{
	return Cast<UPTBDWMiniGameRuleSet>(RuleSet.Get());
}
