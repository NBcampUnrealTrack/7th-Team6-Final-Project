#include "MiniGames/DW/PTBDWMiniGame.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "MiniGames/DW/PTBDWMiniGameRuleSet.h"
#include "MiniGames/DW/PTBDWNoteMarker.h"
#include "MiniGames/DW/PTBDWCharacter.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"

namespace
{
	float DefaultObstacleOffset(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return 300.f;
		case EPTBActionType::ActionB: return 250.f;
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

	// 좌/우 분할 쪼개짐 placeholder.
	namespace DWBreak
	{
		constexpr float SplitDuration = 0.40f;
		constexpr float FlyApart = 28.f;
		constexpr float TipBack  = 12.f;
		constexpr float Fall     = 70.f;
		constexpr float TipAngle = 55.f;
	}
}

bool APTBDWMiniGame::IsSupportedAction(EPTBActionType Action) const
{
	if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
	{
		return DWRule->SupportsAction(Action);
	}
	return Action == EPTBActionType::ActionA || Action == EPTBActionType::ActionB;
}

void APTBDWMiniGame::RequestDWSfx(FName Key, AActor* Target)
{
	if (Key.IsNone())
	{
		return;
	}
	RequestWwiseEvent(Key, Target);
}

void APTBDWMiniGame::RequestDWVfx(UNiagaraSystem* System, const FVector& Location)
{
	if (!System)
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Location);
	}
}

void APTBDWMiniGame::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpawnJudgeTargetIfNeeded();

	for (int32 i = ResolvingObstacles.Num() - 1; i >= 0; --i)
	{
		FDWResolvingObstacle& R = ResolvingObstacles[i];
		AActor* Obs = R.Obstacle;
		if (!Obs)
		{
			ResolvingObstacles.RemoveAtSwap(i);
			continue;
		}

		R.Timer -= DeltaTime;
		const float P = FMath::Clamp(1.0f - R.Timer / FMath::Max(KINDA_SMALL_NUMBER, R.Duration), 0.f, 1.f);
		if (R.bSplitPiece)
		{
			const FVector Loc = R.StartLoc
				+ R.SideDir * (DWBreak::FlyApart * P)
				+ R.BackDir * (DWBreak::TipBack * P)
				+ FVector(0.f, 0.f, -1.f) * (DWBreak::Fall * P * P);
			Obs->SetActorLocation(Loc);
			const FQuat Tip = FQuat(R.RightAxis, FMath::DegreesToRadians(DWBreak::TipAngle * P));
			Obs->SetActorRotation(Tip * R.StartRot.Quaternion());
			Obs->SetActorScale3D(R.StartScale * (1.0f - P));
		}
		else
		{
			Obs->SetActorRotation(R.StartRot + FRotator(70.0f * P, 0.f, 0.f));
			Obs->SetActorScale3D(R.StartScale * (1.0f - P));
		}

		if (R.Timer <= 0.f)
		{
			Obs->Destroy();
			ResolvingObstacles.RemoveAtSwap(i);
		}
	}

	if (!RhythmSyncComponent)
	{
		return;
	}

	const float VisualNow = RhythmSyncComponent->GetVisualChartTimeMs();
	const FVector ForwardDir = GetActorForwardVector();

	TArray<int32> CompletedIds;

	for (TPair<int32, FDWNoteView>& Pair : ActiveNoteViews)
	{
		FDWNoteView& View = Pair.Value;

		const float Alpha = FMath::Clamp((VisualNow - View.SpawnVisualMs) * View.InvDuration, 0.f, 1.f);
		const FVector MarkerPos = FMath::Lerp(View.SpawnPos, View.LinePos, Alpha);

		float VisibleTailLen = 0.f;
		if (View.bIsLong)
		{
			const float HoldDen  = FMath::Max(KINDA_SMALL_NUMBER, View.ReleaseMs - View.TargetMs);
			const float HoldProg = FMath::Clamp((VisualNow - View.TargetMs) / HoldDen, 0.f, 1.f);
			VisibleTailLen = View.TailLengthCm * (1.0f - HoldProg);

			if (View.bJudgedSuccess && HoldProg >= 1.0f)
			{
				CompletedIds.Add(Pair.Key);
			}
		}

		if (View.Marker)
		{
			View.Marker->SetActorLocation(MarkerPos);
			const float Xs = FMath::Lerp(View.MarkerSquareSize, View.MarkerThinX, Alpha);
			View.Marker->SetActorScale3D(FVector(Xs, View.MarkerSquareSize, View.MarkerFlatZ));
		}
		if (View.TailMarker)
		{
			View.TailMarker->SetActorLocation(MarkerPos + ForwardDir * (VisibleTailLen * 0.5f));
			View.TailMarker->SetActorScale3D(FVector(VisibleTailLen / 100.0f, View.MarkerSquareSize, View.MarkerFlatZ));
		}
		if (View.Obstacle)
		{
			View.Obstacle->SetActorLocation(MarkerPos + ForwardDir * VisibleTailLen + View.ObstacleOffsetVec);
		}
	}

	for (int32 Id : CompletedIds)
	{
		RecycleNoteView(Id);
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

	if (RhythmConductor)
	{
		if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
		{
			if (const float* FoundLookAhead = DWRule->LookAheadBeatsByDifficulty.Find(GameContext.SessionRequest.Difficulty))
			{
				RhythmConductor->SetLookAheadBeats(*FoundLookAhead);
				PTB_RECORD(LogPTBMiniGames, TEXT("[%s] DW LookAheadBeats override=%.2f"), *GetNameSafe(this), *FoundLookAhead);
			}
		}
	}

	ClearAllNoteViews();
	SpawnCharactersIfNeeded();

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW BuildRuntimeState RuleSet=%s Chart=%s"),
		*GetNameSafe(this),
		*GetNameSafe(RuleSet.Get()),
		*GetNameSafe(ChartAsset.Get()));

	ValidateDWConfig();
}

void APTBDWMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);

	SpawnNoteView(Note);

	if (Dog && IsSupportedAction(Note.ActionType))
	{
		const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
		const float MaxBeats = DWRule ? DWRule->DogCueMaxBeats : 0.5f;
		const float Bpm      = GameContext.ChartData.BPM;
		const float BeatSec  = (Bpm > 0.f) ? (60.0f / Bpm) : 0.5f;
		const float CueSec   = FMath::Min(0.3f, MaxBeats * BeatSec);

		Dog->PlayCue(Note.ActionType, CueSec);
		RequestDWSfx(DWRule ? DWRule->DogCueSFXKey : NAME_None, Dog);
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

	const bool bSuccess = (Result.Reason == EPTBJudgementReason::Note)
		&& (Result.JudgementType != EPTBJudgementType::Miss);

	bool bIsLong = false;
	AActor* FailObstacle = nullptr;
	if (FDWNoteView* View = ActiveNoteViews.Find(Result.NoteId))
	{
		bIsLong = View->bIsLong;
		if (!bSuccess && View->Obstacle)
		{
			FailObstacle = View->Obstacle;
			View->Obstacle = nullptr;
		}
	}

	if (Protagonist && IsSupportedAction(Result.ActionType))
	{
		Protagonist->PlayReaction(bSuccess, Result.ActionType, bIsLong);

		const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
		if (bSuccess)
		{
			if (Result.ActionType == EPTBActionType::ActionA)
			{
				RequestDWSfx(DWRule ? DWRule->JumpSFXKey : NAME_None, Protagonist);
				if (bIsLong)
				{
					RequestDWSfx(DWRule ? DWRule->SplashSFXKey : NAME_None, Protagonist);
					RequestDWVfx(DWRule ? DWRule->SplashVFX.Get() : nullptr, Protagonist->GetActorLocation());
				}
			}
			else if (Result.ActionType == EPTBActionType::ActionB)
			{
				RequestDWSfx(DWRule ? DWRule->SlideSFXKey : NAME_None, Protagonist);
				RequestDWVfx(DWRule ? DWRule->SlideDustVFX.Get() : nullptr, Protagonist->GetActorLocation());
			}
		}
		else
		{
			RequestDWSfx(DWRule ? DWRule->FailReactionSFXKey : NAME_None, Protagonist);
		}
	}

	if (FailObstacle)
	{
		ResolveObstacleFail(FailObstacle, Result.ActionType);
	}

	if (bIsLong && bSuccess)
	{
		if (FDWNoteView* View = ActiveNoteViews.Find(Result.NoteId))
		{
			View->bJudgedSuccess = true;
		}
	}
	else
	{
		RecycleNoteView(Result.NoteId);
	}

	PTB_RECORD(LogPTBMiniGames,
		TEXT("[%s] DW Judgement NoteId=%d Action=%d Type=%d Reason=%d Long=%d Success=%d DeltaMs=%.3f ScoreDelta=%d"),
		*GetNameSafe(this),
		Result.NoteId,
		static_cast<int32>(Result.ActionType),
		static_cast<int32>(Result.JudgementType),
		static_cast<int32>(Result.Reason),
		bIsLong ? 1 : 0,
		bSuccess ? 1 : 0,
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
	const float HoldScale     = DWRule ? DWRule->HoldVisualScale : 0.5f;
	const float ObstScale     = DWRule ? DWRule->ObstacleHoldScale : 0.5f;

	const FVector ForwardDir = GetActorForwardVector();
	const FVector LinePos = GetActorLocation();
	const FVector SpawnPos = LinePos + ForwardDir * SpawnDistance;

	const FVector OffsetVec = ForwardDir * GetObstacleOffset(Note.ActionType);

	const float SpawnVisualMs = RhythmSyncComponent->GetVisualChartTimeMs();
	const float InvDuration   = 1.0f / FMath::Max(1.0f, Note.TimeMs - SpawnVisualMs);

	const bool bIsLong = Note.bIsLongNote || Note.NoteType == EPTBNoteType::Hold;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float SqSize = DWRule ? DWRule->MarkerSquareSize : 0.6f;
	const float ThinX  = DWRule ? DWRule->MarkerThinX : 0.08f;
	const float FlatZ  = DWRule ? DWRule->MarkerFlatZ : 0.05f;
	const FRotator MarkerRot = ForwardDir.Rotation();

	UMaterialInterface* MarkerMat = DWRule ? DWRule->MarkerMaterial.Get() : nullptr;

	APTBDWNoteMarker* Marker = World->SpawnActor<APTBDWNoteMarker>(SpawnPos, MarkerRot, Params);
	if (!Marker)
	{
		return;
	}
	{
		const FLinearColor MarkerCol = DWRule ? DWRule->MarkerColor : FLinearColor(0.9f, 0.9f, 1.0f, 0.85f);
		Marker->Configure(nullptr, MarkerMat, MarkerCol, FVector(SqSize, SqSize, FlatZ));
	}

	APTBDWNoteMarker* TailMarker = nullptr;
	float TailLengthCm = 0.f;
	if (bIsLong)
	{
		const float HoldDurationMs = FMath::Max(0.0f, Note.ReleaseTimeMs - Note.TimeMs);
		TailLengthCm = (SpawnDistance * InvDuration) * HoldDurationMs * HoldScale;
		TailMarker = World->SpawnActor<APTBDWNoteMarker>(SpawnPos, MarkerRot, Params);
		if (TailMarker)
		{
			const FLinearColor TailCol = DWRule ? DWRule->MarkerTailColor : FLinearColor(0.9f, 0.9f, 1.0f, 0.20f);
			TailMarker->Configure(nullptr, MarkerMat, TailCol, FVector(TailLengthCm / 100.0f, SqSize, FlatZ));
		}
	}

	// ── 장애물(판정선 대비 α 오프셋) ─────────────────────
	FVector  ObstacleScl = GetObstacleScale(Note.ActionType);
	FRotator ObstacleRot = FRotator::ZeroRotator;
	if (bIsLong)
	{
		const float HoldDurationMs = FMath::Max(0.0f, Note.ReleaseTimeMs - Note.TimeMs);
		const float VisualSpeed    = SpawnDistance * InvDuration;
		const float HoldLengthCm   = VisualSpeed * HoldDurationMs * HoldScale * ObstScale;
		ObstacleScl.X = FMath::Max(ObstacleScl.X, HoldLengthCm / 100.0f);
		ObstacleRot   = ForwardDir.Rotation();
	}

	APTBDWNoteMarker* Obstacle = World->SpawnActor<APTBDWNoteMarker>(SpawnPos + OffsetVec, ObstacleRot, Params);
	if (Obstacle)
	{
		UMaterialInterface* BaseMat = DWRule ? DWRule->ObstacleMaterial.Get() : nullptr;
		Obstacle->Configure(GetObstacleMesh(Note.ActionType), BaseMat, GetObstacleColor(Note.ActionType), ObstacleScl);
	}

	FDWNoteView View;
	View.Marker            = Marker;
	View.Obstacle          = Obstacle;
	View.TailMarker        = TailMarker;
	View.SpawnVisualMs     = SpawnVisualMs;
	View.TargetMs          = Note.TimeMs;
	View.ReleaseMs         = Note.ReleaseTimeMs;
	View.InvDuration       = InvDuration;
	View.SpawnPos          = SpawnPos;
	View.LinePos           = LinePos;
	View.ObstacleOffsetVec = OffsetVec;
	View.MarkerSquareSize  = SqSize;
	View.MarkerThinX       = ThinX;
	View.MarkerFlatZ       = FlatZ;
	View.TailLengthCm      = TailLengthCm;
	View.Action            = Note.ActionType;
	View.bIsLong           = bIsLong;

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
				DWRule ? DWRule->ProtagonistJumpAnim.Get() : nullptr,
				DWRule ? DWRule->ProtagonistSlideAnim.Get() : nullptr,
				DWRule ? DWRule->ProtagonistFailAnim.Get() : nullptr,
				DWRule ? DWRule->ProtagonistAnimClass : nullptr);
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
			Dog->SetAnimations(DWRule ? DWRule->DogRunAnim.Get() : nullptr, nullptr, nullptr, nullptr, nullptr);
		}
	}
}

void APTBDWMiniGame::RecycleNoteView(int32 NoteId)
{
	FDWNoteView View;
	if (ActiveNoteViews.RemoveAndCopyValue(NoteId, View))
	{
		if (View.Marker)     { View.Marker->Destroy(); }
		if (View.TailMarker) { View.TailMarker->Destroy(); }
		if (View.Obstacle)   { View.Obstacle->Destroy(); }
	}
}

void APTBDWMiniGame::ResolveObstacleFail(AActor* Obstacle, EPTBActionType Action)
{
	if (!Obstacle)
	{
		return;
	}

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();

	const FVector BreakLoc = GetActorLocation() + GetActorForwardVector() * GetObstacleOffset(Action);
	Obstacle->SetActorLocation(BreakLoc);

	if (DWRule)
	{
		if (const TObjectPtr<UNiagaraSystem>* VFX = DWRule->BreakVFXByAction.Find(Action))
		{
			RequestDWVfx(*VFX, Obstacle->GetActorLocation());
		}
	}

	UStaticMesh* BrokenMesh = nullptr;
	if (DWRule)
	{
		if (const TObjectPtr<UStaticMesh>* Broken = DWRule->BrokenObstacleMeshByAction.Find(Action))
		{
			BrokenMesh = *Broken;
		}
	}

	if (BrokenMesh)
	{
		if (APTBDWNoteMarker* M = Cast<APTBDWNoteMarker>(Obstacle))
		{
			M->SetMesh(BrokenMesh);
		}

		FDWResolvingObstacle R;
		R.Obstacle    = Obstacle;
		R.Duration    = 0.18f;
		R.Timer       = 0.18f;
		R.StartLoc    = Obstacle->GetActorLocation();
		R.StartScale  = Obstacle->GetActorScale3D();
		R.StartRot    = Obstacle->GetActorRotation();
		R.bSplitPiece = false;
		ResolvingObstacles.Add(R);
		return;
	}

	SpawnSplitHalves(Obstacle, Action);
	Obstacle->Destroy();
}

void APTBDWMiniGame::SpawnSplitHalves(AActor* Obstacle, EPTBActionType Action)
{
	UWorld* World = GetWorld();
	if (!World || !Obstacle)
	{
		return;
	}

	const FVector  L       = Obstacle->GetActorLocation();
	const FRotator R       = Obstacle->GetActorRotation();
	const FVector  S       = Obstacle->GetActorScale3D();
	const FVector  Right   = Obstacle->GetActorRightVector();
	const FVector  Forward = Obstacle->GetActorForwardVector();

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	UStaticMesh*        Mesh = GetObstacleMesh(Action);
	UMaterialInterface* Mat  = DWRule ? DWRule->ObstacleMaterial.Get() : nullptr;
	const FLinearColor  Col  = GetObstacleColor(Action);

	const float    BaseX     = GetObstacleScale(Action).X;
	const FVector HalfScale  = FVector(BaseX, S.Y * 0.5f, S.Z);
	const float   SideOffset = 100.0f * S.Y * 0.25f;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const FVector StartLoc = L + Right * (Side * SideOffset);
		APTBDWNoteMarker* Half = World->SpawnActor<APTBDWNoteMarker>(StartLoc, R, Params);
		if (!Half)
		{
			continue;
		}
		Half->Configure(Mesh, Mat, Col, HalfScale);

		FDWResolvingObstacle Rsv;
		Rsv.Obstacle    = Half;
		Rsv.Duration    = DWBreak::SplitDuration;
		Rsv.Timer       = DWBreak::SplitDuration;
		Rsv.StartLoc    = StartLoc;
		Rsv.StartScale  = HalfScale;
		Rsv.StartRot    = R;
		Rsv.SideDir     = Right * static_cast<float>(Side);
		Rsv.BackDir     = -Forward;
		Rsv.RightAxis   = Right;
		Rsv.bSplitPiece = true;
		ResolvingObstacles.Add(Rsv);
	}
}

void APTBDWMiniGame::SpawnJudgeTargetIfNeeded()
{
	if (JudgeTargetBar)
	{
		return;
	}

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	if (DWRule && !DWRule->bShowJudgeTarget)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector  LinePos = GetActorLocation();
	const FRotator Rot     = GetActorForwardVector().Rotation();
	const float SqSize = DWRule ? DWRule->MarkerSquareSize : 0.6f;
	const float ThinX  = DWRule ? DWRule->MarkerThinX : 0.08f;
	const float FlatZ  = DWRule ? DWRule->MarkerFlatZ : 0.05f;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	APTBDWNoteMarker* Bar = World->SpawnActor<APTBDWNoteMarker>(LinePos, Rot, Params);
	if (Bar)
	{
		UMaterialInterface* Mat = DWRule ? DWRule->MarkerMaterial.Get() : nullptr;
		const FLinearColor Col  = DWRule ? DWRule->JudgeTargetColor : FLinearColor(1.0f, 0.85f, 0.2f, 0.9f);
		Bar->Configure(nullptr, Mat, Col, FVector(ThinX, SqSize, FlatZ));
	}
	JudgeTargetBar = Bar;
}

void APTBDWMiniGame::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (JudgeTargetBar)
	{
		JudgeTargetBar->Destroy();
		JudgeTargetBar = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void APTBDWMiniGame::ClearAllNoteViews()
{
	for (TPair<int32, FDWNoteView>& Pair : ActiveNoteViews)
	{
		if (Pair.Value.Marker)     { Pair.Value.Marker->Destroy(); }
		if (Pair.Value.TailMarker) { Pair.Value.TailMarker->Destroy(); }
		if (Pair.Value.Obstacle)   { Pair.Value.Obstacle->Destroy(); }
	}
	ActiveNoteViews.Reset();

	for (FDWResolvingObstacle& R : ResolvingObstacles)
	{
		if (R.Obstacle) { R.Obstacle->Destroy(); }
	}
	ResolvingObstacles.Reset();
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

void APTBDWMiniGame::ValidateDWConfig() const
{
	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	if (!DWRule)
	{
		PTB_RECORD(LogPTBMiniGames,
			TEXT("[%s] DW config WARN: RuleSet이 없거나 UPTBDWMiniGameRuleSet이 아님 — BP_DW_MiniGame.RuleSet / DataAsset 클래스 확인."),
			*GetNameSafe(this));
		return;
	}

	if (DWRule->MiniGameId != FName(TEXT("DW")) || DWRule->MiniGameCode != FName(TEXT("DW")))
	{
		PTB_RECORD(LogPTBMiniGames,
			TEXT("[%s] DW config WARN: MiniGameId/Code가 DW가 아님 (Id=%s Code=%s). 5개 값 일치 규칙 확인."),
			*GetNameSafe(this), *DWRule->MiniGameId.ToString(), *DWRule->MiniGameCode.ToString());
	}
	if (!DWRule->SupportsAction(EPTBActionType::ActionA) || !DWRule->SupportsAction(EPTBActionType::ActionB))
	{
		PTB_RECORD(LogPTBMiniGames,
			TEXT("[%s] DW config WARN: SupportedActions에 A/B가 모두 있어야 함(MVP)."),
			*GetNameSafe(this));
	}
	if (!ChartAsset)
	{
		PTB_RECORD(LogPTBMiniGames,
			TEXT("[%s] DW config WARN: ChartAsset 미해결 — ChartAssetsByDifficulty(Easy+Standard) 확인."),
			*GetNameSafe(this));
	}

	TArray<FString> Missing;
	if (!DWRule->ProtagonistMesh)      { Missing.Add(TEXT("ProtagonistMesh")); }
	if (!DWRule->DogMesh)              { Missing.Add(TEXT("DogMesh")); }
	if (!DWRule->ProtagonistRunAnim)   { Missing.Add(TEXT("ProtagonistRunAnim")); }
	if (!DWRule->ProtagonistJumpAnim)  { Missing.Add(TEXT("ProtagonistJumpAnim")); }
	if (!DWRule->ProtagonistSlideAnim) { Missing.Add(TEXT("ProtagonistSlideAnim")); }
	if (!DWRule->ProtagonistFailAnim)  { Missing.Add(TEXT("ProtagonistFailAnim")); }
	if (!DWRule->DogRunAnim)           { Missing.Add(TEXT("DogRunAnim")); }
	if (!DWRule->MarkerMaterial)       { Missing.Add(TEXT("MarkerMaterial")); }
	if (!DWRule->ObstacleMaterial)     { Missing.Add(TEXT("ObstacleMaterial")); }
	if (!DWRule->ObstacleMeshByAction.Contains(EPTBActionType::ActionA)) { Missing.Add(TEXT("ObstacleMeshByAction[A]")); }
	if (!DWRule->ObstacleMeshByAction.Contains(EPTBActionType::ActionB)) { Missing.Add(TEXT("ObstacleMeshByAction[B]")); }
	if (DWRule->DogCueSFXKey.IsNone()) { Missing.Add(TEXT("DogCueSFXKey")); }
	if (DWRule->JumpSFXKey.IsNone())   { Missing.Add(TEXT("JumpSFXKey")); }
	if (DWRule->SplashSFXKey.IsNone()) { Missing.Add(TEXT("SplashSFXKey")); }
	if (DWRule->SlideSFXKey.IsNone())  { Missing.Add(TEXT("SlideSFXKey")); }
	if (DWRule->FailReactionSFXKey.IsNone())   { Missing.Add(TEXT("FailReactionSFXKey")); }

	if (Missing.Num() > 0)
	{
		PTB_RECORD(LogPTBMiniGames,
			TEXT("[%s] DW config: 미설정 슬롯(placeholder/폴백 사용, 곡·에셋 확정 시 채울 것) → %s"),
			*GetNameSafe(this), *FString::Join(Missing, TEXT(", ")));
	}
}
