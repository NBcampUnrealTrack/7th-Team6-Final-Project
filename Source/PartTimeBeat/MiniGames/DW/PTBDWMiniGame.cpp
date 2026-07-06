#include "MiniGames/DW/PTBDWMiniGame.h"

#include "Debug/PTBTeamLog.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "MiniGames/DW/PTBDWMiniGameRuleSet.h"
#include "MiniGames/DW/PTBDWNoteMarker.h"
#include "MiniGames/DW/PTBDWCharacter.h"
#include "MiniGames/DW/PTBDWBackgroundScroller.h"
#include "MiniGames/DW/PTBDWCameraRig.h"
#include "EngineUtils.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"
#include "Rhythm/PTBScoreCalculator.h"

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

	const float KickGravity = GetDWRuleSet() ? GetDWRuleSet()->KickGravity : 2000.0f;
	const FVector ResolveFwd = GetActorForwardVector();
	const FVector ResolveRight = GetActorRightVector();
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
		if (R.bLaunch)
		{
			R.Vel += FVector(0.f, 0.f, -KickGravity) * DeltaTime;
			FVector NewLoc = Obs->GetActorLocation() + R.Vel * DeltaTime;
			if (NewLoc.Z < R.GroundZ)
			{
				NewLoc.Z = R.GroundZ;
				R.Vel.Z = FMath::Abs(R.Vel.Z) * R.Restitution;
				R.Vel.X *= 0.7f;
				R.Vel.Y *= 0.7f;
			}
			Obs->SetActorLocation(NewLoc);
			Obs->AddActorLocalRotation(FRotator(R.SpinDegPerSec * DeltaTime, 0.f, 0.f));
			if (P > 0.7f) { Obs->SetActorScale3D(R.StartScale * FMath::Max(0.f, 1.f - (P - 0.7f) / 0.3f)); }
		}
		else if (R.bSplitPiece)
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
			const float FallFrac = FMath::Clamp(R.FallFrac, 0.02f, 1.f);
			if (R.BackVel > 1.f)
			{
				Obs->AddActorWorldOffset(-ResolveFwd * R.BackVel * DeltaTime);
				const float fp = FMath::Clamp(P / FallFrac, 0.f, 1.f);
				const float eased = 1.f - (1.f - fp) * (1.f - fp);
				Obs->SetActorRotation(R.StartRot.Quaternion() * FRotator(R.EndRot.Pitch * eased, R.EndRot.Yaw * eased, R.EndRot.Roll * eased).Quaternion());
				if (P < FallFrac && R.SpreadY != 0.f)
				{
					const float FallTime = FMath::Max(KINDA_SMALL_NUMBER, FallFrac * R.Duration);
					Obs->AddActorWorldOffset(ResolveRight * (R.SpreadY / FallTime) * DeltaTime);
				}
			}
			else
			{
				if (P <= FallFrac)
				{
					const float fp = P / FallFrac;
					const float eased = 1.f - (1.f - fp) * (1.f - fp);
					Obs->SetActorRotation(R.StartRot.Quaternion() * FRotator(R.EndRot.Pitch * eased, R.EndRot.Yaw * eased, R.EndRot.Roll * eased).Quaternion());
					Obs->SetActorScale3D(R.StartScale);
				}
				else
				{
					const float gp = (P - FallFrac) / FMath::Max(KINDA_SMALL_NUMBER, 1.f - FallFrac);
					Obs->SetActorRotation(R.StartRot.Quaternion() * R.EndRot.Quaternion());
					Obs->SetActorScale3D(R.StartScale * FMath::Max(0.f, 1.f - gp));
				}
			}
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
	const float StarSpinDeg = (GetDWRuleSet() ? GetDWRuleSet()->StarSpinDegPerSec : 0.f) * DeltaTime;

	TArray<TTuple<int32, EPTBActionType, bool>> ToResolve;

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

			if (View.bJudged && HoldProg >= 1.0f)
			{
				ToResolve.Add(MakeTuple(Pair.Key, View.Action, View.bJudgedSuccess));
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
			View.Obstacle->SetActorLocation(MarkerPos + ForwardDir * VisibleTailLen + View.ObstacleOffsetVec + View.ObstaclePivotWorld);
			if (View.Action == EPTBActionType::ActionE && StarSpinDeg != 0.f)
			{
				View.Obstacle->AddActorLocalRotation(FRotator(0.f, StarSpinDeg, 0.f));
			}
		}
	}

	for (const TTuple<int32, EPTBActionType, bool>& T : ToResolve)
	{
		PlayNoteResultEffects(T.Get<0>(), T.Get<1>(), T.Get<2>(), true);
	}
}

void APTBDWMiniGame::StartMiniGame()
{
	Super::StartMiniGame();


	if (!BackgroundScroller)
	{
		for (TActorIterator<APTBDWBackgroundScroller> It(GetWorld()); It; ++It)
		{
			BackgroundScroller = *It;
			break;
		}
	}

	if (!CameraRig)
	{
		for (TActorIterator<APTBDWCameraRig> It(GetWorld()); It; ++It)
		{
			CameraRig = *It;
			break;
		}
	}
}

void APTBDWMiniGame::ReceiveIntroStarted_Implementation()
{
	Super::ReceiveIntroStarted_Implementation();

	if (!CameraRig)
	{
		for (TActorIterator<APTBDWCameraRig> It(GetWorld()); It; ++It)
		{
			CameraRig = *It;
			break;
		}
	}
	if (CameraRig)
	{
		CameraRig->StartIntroMove();
	}
}

void APTBDWMiniGame::ReceiveGameplayStarted_Implementation()
{
	Super::ReceiveGameplayStarted_Implementation();

	const float MotionInfluence = GetDWRuleSet() ? GetDWRuleSet()->AnimSpeedInfluence : 0.3f;
	const float MotionRate = 1.0f + (ActiveSpeedScale - 1.0f) * MotionInfluence;

	const float BgMultiplier = GetDWRuleSet() ? GetDWRuleSet()->BackgroundScrollMultiplier : 1.0f;
	if (BackgroundScroller)
	{
		BackgroundScroller->SetSpeedScale(ActiveSpeedScale, BgMultiplier);
		BackgroundScroller->SetRunning(true);
	}
	if (Protagonist) { Protagonist->StartRunning(); Protagonist->SetMotionPlayRate(MotionRate); }
	if (Dog)         { Dog->StartRunning();         Dog->SetMotionPlayRate(MotionRate); }
}

void APTBDWMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	{
		float EffectiveLookAhead = RuleSet ? RuleSet->LookAheadBeats : ActiveLookAheadBeats;
		if (const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet())
		{
			if (const float* FoundLookAhead = DWRule->LookAheadBeatsByDifficulty.Find(GameContext.SessionRequest.Difficulty))
			{
				EffectiveLookAhead = *FoundLookAhead;
			}
		}
		ActiveLookAheadBeats = EffectiveLookAhead;
		if (RhythmConductor)
		{
			RhythmConductor->SetLookAheadBeats(EffectiveLookAhead);
		}

		{
			const float RefLookAhead = GetDWRuleSet() ? GetDWRuleSet()->ScrollSyncBaseLookAheadBeats : EffectiveLookAhead;
			ActiveSpeedScale = FMath::Max(0.01f, RefLookAhead) / FMath::Max(0.01f, EffectiveLookAhead);
		}
		PTB_RECORD(LogPTBMiniGames, TEXT("[%s] DW LookAheadBeats=%.2f (resolved)"), *GetNameSafe(this), EffectiveLookAhead);
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

void APTBDWMiniGame::PlayNoteResultEffects(int32 NoteId, EPTBActionType Action, bool bSuccess, bool bIsLong)
{
	AActor* FailObstacle = nullptr;
	if (FDWNoteView* View = ActiveNoteViews.Find(NoteId))
	{
		if (!bSuccess && View->Obstacle)
		{
			FailObstacle = View->Obstacle;
			View->Obstacle = nullptr;
		}
	}

	if (Protagonist && IsSupportedAction(Action))
	{
		Protagonist->PlayReaction(bSuccess, Action, bIsLong);

		const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
		if (bSuccess)
		{
			if (Action == EPTBActionType::ActionA)
			{
				RequestDWSfx(DWRule ? DWRule->JumpSFXKey : NAME_None, Protagonist);
				if (bIsLong)
				{
					RequestDWSfx(DWRule ? DWRule->SplashSFXKey : NAME_None, Protagonist);
					RequestDWVfx(DWRule ? DWRule->SplashVFX.Get() : nullptr, Protagonist->GetActorLocation());
				}
			}
			else if (Action == EPTBActionType::ActionB)
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

	if (bSuccess && (Action == EPTBActionType::ActionA || Action == EPTBActionType::ActionB))
	{
		if (FDWNoteView* V = ActiveNoteViews.Find(NoteId))
		{
			if (V->Obstacle)
			{
				ScrollObstacleAway(V->Obstacle);
				V->Obstacle = nullptr;
			}
		}
	}

	if (Action == EPTBActionType::ActionD)
	{
		AActor* Ball = nullptr;
		if (bSuccess)
		{
			if (FDWNoteView* V = ActiveNoteViews.Find(NoteId))
			{
				Ball = V->Obstacle;
				V->Obstacle = nullptr;
			}
		}
		else
		{
			Ball = FailObstacle;
			FailObstacle = nullptr;
		}
		if (Ball) { LaunchKickBall(Ball, bSuccess, bIsLong); }
	}

	if (FailObstacle)
	{
		ResolveObstacleFail(FailObstacle, Action);
	}

	OnDWNoteResolved.Broadcast(bSuccess, ScoreCalculator ? ScoreCalculator->ComboCount : 0);

	RecycleNoteView(NoteId);
}


void APTBDWMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);

	if (ScoreCalculator)
	{
		OnDWComboChanged.Broadcast(ScoreCalculator->ComboCount);
	}

	FDWNoteView* View = ActiveNoteViews.Find(Result.NoteId);
	const bool bIsLong = View ? View->bIsLong : false;

	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		if (bIsLong && View)
		{
			View->bJudged = true;
			View->bJudgedSuccess = false;
		}
		PTB_VERBOSE(LogPTBMiniGames,
			TEXT("[%s] DW EmptyInput Action=%d Long=%d"),
			*GetNameSafe(this),
			static_cast<int32>(Result.ActionType),
			bIsLong ? 1 : 0);
		return;
	}

	const bool bSuccess = (Result.Reason == EPTBJudgementReason::Note)
		&& (Result.JudgementType != EPTBJudgementType::Miss);

	if (bIsLong && View)
	{
		View->bJudged = true;
		View->bJudgedSuccess = bSuccess;
	}
	else
	{
		PlayNoteResultEffects(Result.NoteId, Result.ActionType, bSuccess, false);
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

	const FVector ForwardDir = GetActorForwardVector();
	const FVector LinePos = GetActorLocation();
	const FVector SpawnPos = LinePos + ForwardDir * SpawnDistance;

	const FVector OffsetVec = ForwardDir * GetObstacleOffset(Note.ActionType);
	const FVector RightDir = GetActorRightVector();
	const FVector UpDir    = GetActorUpVector();
	const FVector PivotLocal = DWRule ? DWRule->ObstaclePivotOffsetByAction.FindRef(Note.ActionType) : FVector::ZeroVector;
	const FVector PivotWorld = ForwardDir * PivotLocal.X + RightDir * PivotLocal.Y + UpDir * PivotLocal.Z;

	const float PrerollMsPerBeat = (GameContext.ChartData.BPM > 0.f) ? (60000.0f / GameContext.ChartData.BPM) : 588.0f;
	const float LookAheadMs   = FMath::Max(1.0f, ActiveLookAheadBeats * PrerollMsPerBeat);
	const float SpawnVisualMs = Note.TimeMs - LookAheadMs;
	const float InvDuration   = 1.0f / LookAheadMs;
	const float NowVisualMs   = RhythmSyncComponent->GetVisualChartTimeMs();
	const float InitAlpha     = FMath::Clamp((NowVisualMs - SpawnVisualMs) * InvDuration, 0.f, 1.f);
	const FVector InitMarkerPos = FMath::Lerp(SpawnPos, LinePos, InitAlpha);

	const bool bIsLong = Note.bIsLongNote || Note.NoteType == EPTBNoteType::Hold;

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const float SqSize = DWRule ? DWRule->MarkerSquareSize : 0.6f;
	const float ThinX  = DWRule ? DWRule->MarkerThinX : 0.08f;
	const float FlatZ  = DWRule ? DWRule->MarkerFlatZ : 0.05f;
	const FRotator MarkerRot = ForwardDir.Rotation();

	UMaterialInterface* MarkerMat = DWRule ? DWRule->MarkerMaterial.Get() : nullptr;

	APTBDWNoteMarker* Marker = World->SpawnActor<APTBDWNoteMarker>(InitMarkerPos, MarkerRot, Params);
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
		TailMarker = World->SpawnActor<APTBDWNoteMarker>(InitMarkerPos, MarkerRot, Params);
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
		const FVector* HoldMultPtr = DWRule ? DWRule->ObstacleHoldLengthScaleByAction.Find(Note.ActionType) : nullptr;
		const FVector HoldMult     = HoldMultPtr ? *HoldMultPtr : FVector::ZeroVector;
		const float BaseLenCm      = VisualSpeed * HoldDurationMs * HoldScale;
		if (HoldMult.X > 0.f) { ObstacleScl.X = FMath::Max(ObstacleScl.X, BaseLenCm * HoldMult.X / 100.0f); }
		if (HoldMult.Y > 0.f) { ObstacleScl.Y = FMath::Max(ObstacleScl.Y, BaseLenCm * HoldMult.Y / 100.0f); }
		if (HoldMult.Z > 0.f) { ObstacleScl.Z = FMath::Max(ObstacleScl.Z, BaseLenCm * HoldMult.Z / 100.0f); }
		ObstacleRot   = ForwardDir.Rotation();
	}

	APTBDWNoteMarker* Obstacle = World->SpawnActor<APTBDWNoteMarker>(InitMarkerPos + OffsetVec + PivotWorld, ObstacleRot, Params);
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
	View.ObstaclePivotWorld = PivotWorld;
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

float APTBDWMiniGame::GetNoteApproachSpeedCmS() const
{
	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	const float SpawnDistance = DWRule ? DWRule->MarkerSpawnDistance : 1200.f;
	const float MsPerBeat = (GameContext.ChartData.BPM > 0.f) ? (60000.f / GameContext.ChartData.BPM) : 588.f;
	const float LookAheadMs = FMath::Max(1.f, ActiveLookAheadBeats * MsPerBeat);
	return SpawnDistance / LookAheadMs * 1000.f;
}

void APTBDWMiniGame::ScrollObstacleAway(AActor* Obstacle)
{
	if (!Obstacle) { return; }

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	const float ScrollSpd = GetNoteApproachSpeedCmS();
	const float Despawn = DWRule ? DWRule->BreakScrollDespawnDistance : 3500.f;
	if (ScrollSpd <= 1.f) { Obstacle->Destroy(); return; }

	FDWResolvingObstacle R;
	R.Obstacle    = Obstacle;
	R.Duration    = FMath::Max(0.1f, Despawn / ScrollSpd);
	R.Timer       = R.Duration;
	R.StartLoc    = Obstacle->GetActorLocation();
	R.StartScale  = Obstacle->GetActorScale3D();
	R.StartRot    = Obstacle->GetActorRotation();
	R.EndRot       = FRotator::ZeroRotator;
	R.FallFrac     = 1.f;
	R.BackVel      = ScrollSpd;
	R.bSplitPiece = false;
	ResolvingObstacles.Add(R);
}

void APTBDWMiniGame::ResolveObstacleFail(AActor* Obstacle, EPTBActionType Action)
{
	if (!Obstacle)
	{
		return;
	}

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();


	if (DWRule)
	{
		if (const TObjectPtr<UNiagaraSystem>* VFX = DWRule->BreakVFXByAction.Find(Action))
		{
			RequestDWVfx(*VFX, Obstacle->GetActorLocation());
		}
	}

	if (DWRule && (DWRule->BrokenPieceLeftByAction.Contains(Action) || DWRule->BrokenPieceRightByAction.Contains(Action)))
	{
		SpawnBrokenPieces(Obstacle, Action);
		return;
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

		const float FallDur = DWRule ? DWRule->BreakFallDuration : 0.45f;
		const float FadeDur = DWRule ? DWRule->BreakFadeDuration : 0.25f;
		const float ScrollSpd = GetNoteApproachSpeedCmS();
		const float Despawn = DWRule ? DWRule->BreakScrollDespawnDistance : 3500.f;
		const float TotalDur = (ScrollSpd > 1.f)
			? FMath::Max(FallDur, Despawn / ScrollSpd)
			: FMath::Max(0.05f, FallDur + FadeDur);

		FDWResolvingObstacle R;
		R.Obstacle    = Obstacle;
		R.Duration    = TotalDur;
		R.Timer       = TotalDur;
		R.StartLoc    = Obstacle->GetActorLocation();
		R.StartScale  = Obstacle->GetActorScale3D();
		R.StartRot    = Obstacle->GetActorRotation();
		R.EndRot       = FRotator(DWRule ? DWRule->BreakFallAngleDeg : 85.f, 0.f, 0.f);
		R.FallFrac     = FallDur / TotalDur;
		R.BackVel      = ScrollSpd;
		R.bSplitPiece = false;
		ResolvingObstacles.Add(R);
		return;
	}

	SpawnSplitHalves(Obstacle, Action);
	Obstacle->Destroy();
}

void APTBDWMiniGame::LaunchKickBall(AActor* Ball, bool bSuccess, bool bIsLong)
{
	if (!Ball) { return; }

	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	const FVector Fwd   = GetActorForwardVector();
	const FVector Up    = GetActorUpVector();
	const FVector Right = GetActorRightVector();

	FVector Vel = FVector::ZeroVector;
	if (bSuccess)
	{
		const float Ang = bIsLong ? (DWRule ? DWRule->KickAngleLong : 40.f) : (DWRule ? DWRule->KickAngleSingle : 15.f);
		const float Spd = bIsLong ? (DWRule ? DWRule->KickSpeedLong : 1400.f) : (DWRule ? DWRule->KickSpeedSingle : 900.f);
		const float Rad = FMath::DegreesToRadians(Ang);
		Vel = (Fwd * FMath::Cos(Rad) + Up * FMath::Sin(Rad)) * Spd;
		RequestDWSfx(DWRule ? DWRule->KickSFXKey : NAME_None, Ball);
		UNiagaraSystem* VFX = bIsLong ? (DWRule ? DWRule->KickVFXLong.Get() : nullptr) : (DWRule ? DWRule->KickVFX.Get() : nullptr);
		RequestDWVfx(VFX, Ball->GetActorLocation());
	}
	else
	{
		const float Spd = DWRule ? DWRule->KickFailSpeed : 450.f;
		Vel = (Fwd * -0.7f + Right * 0.5f + Up * 0.3f).GetSafeNormal() * Spd;
	}

	FDWResolvingObstacle R;
	R.Obstacle      = Ball;
	R.bLaunch       = true;
	R.Vel           = Vel;
	R.GroundZ       = Ball->GetActorLocation().Z;
	R.Restitution   = DWRule ? DWRule->KickRestitution : 0.4f;
	R.Duration      = bSuccess ? (DWRule ? DWRule->KickLifetime : 1.2f)
	                            : (DWRule ? DWRule->KickFailLifetime : 5.0f);
	R.Timer         = R.Duration;
	R.StartScale    = Ball->GetActorScale3D();
	R.SpinDegPerSec = 720.f;
	ResolvingObstacles.Add(R);
}

void APTBDWMiniGame::SpawnBrokenPieces(AActor* Obstacle, EPTBActionType Action)
{
	UWorld* World = GetWorld();
	const UPTBDWMiniGameRuleSet* DWRule = GetDWRuleSet();
	if (!World || !Obstacle || !DWRule) { if (Obstacle) { Obstacle->Destroy(); } return; }

	UStaticMesh* LeftMesh = nullptr;
	UStaticMesh* RightMesh = nullptr;
	if (const TObjectPtr<UStaticMesh>* Lm = DWRule->BrokenPieceLeftByAction.Find(Action))  { LeftMesh  = *Lm; }
	if (const TObjectPtr<UStaticMesh>* Rm = DWRule->BrokenPieceRightByAction.Find(Action)) { RightMesh = *Rm; }

	const FVector  Loc = Obstacle->GetActorLocation();
	const FRotator Rot = Obstacle->GetActorRotation();
	const FVector  Scl = Obstacle->GetActorScale3D();
	UMaterialInterface* Mat = DWRule->ObstacleMaterial.Get();
	const FLinearColor Col = GetObstacleColor(Action);

	const float ScrollSpd = GetNoteApproachSpeedCmS();
	const float Despawn   = DWRule->BreakScrollDespawnDistance;
	const float FallDur   = DWRule->BreakFallDuration;
	const float TotalDur  = (ScrollSpd > 1.f) ? FMath::Max(FallDur, Despawn / ScrollSpd) : FMath::Max(0.05f, FallDur);
	const FRotator EndRotL = DWRule->BreakSplitEndRotLeft;
	const FRotator EndRotR = DWRule->BreakSplitEndRotRight;
	const float SpreadDist = DWRule->BreakSplitSpreadY;

	Obstacle->Destroy();

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UStaticMesh* Meshes[2] = { LeftMesh, RightMesh };
	const FRotator EndRots[2]  = { EndRotL, EndRotR };
	const float  SpreadSign[2] = { -1.f, 1.f };
	for (int32 s = 0; s < 2; ++s)
	{
		if (!Meshes[s]) { continue; }
		APTBDWNoteMarker* Piece = World->SpawnActor<APTBDWNoteMarker>(Loc, Rot, Params);
		if (!Piece) { continue; }
		Piece->Configure(Meshes[s], Mat, Col, Scl);

		FDWResolvingObstacle Rsv;
		Rsv.Obstacle    = Piece;
		Rsv.Duration    = TotalDur;
		Rsv.Timer       = TotalDur;
		Rsv.StartLoc    = Loc;
		Rsv.StartScale  = Scl;
		Rsv.StartRot    = Rot;
		Rsv.EndRot  = EndRots[s];
		Rsv.SpreadY = SpreadDist * SpreadSign[s];
		Rsv.FallFrac     = FallDur / TotalDur;
		Rsv.BackVel      = ScrollSpd;
		Rsv.bSplitPiece  = false;
		ResolvingObstacles.Add(Rsv);
	}
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
