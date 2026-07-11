#include "MiniGames/BB/PTBBBMiniGame.h"
#include "MiniGames/BB/PTBBBMiniGameRuleSet.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Camera/CameraShakeBase.h"
#include "Debug/PTBTeamLog.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Rhythm/PTBRhythmChartAsset.h"
#include "Rhythm/PTBRhythmConductorComponent.h"

// ── 런타임 상태 조회 ─────────────────────────────────────────────

float APTBBBMiniGame::GetBossHPPercent() const
{
	return (BossMaxHP > 0.f) ? (BossCurrentHP / BossMaxHP) : 0.f;
}

float APTBBBMiniGame::GetPlayerHPPercent() const
{
	return (PlayerMaxHP > 0.f) ? (PlayerCurrentHP / PlayerMaxHP) : 0.f;
}

// ── 입력 편의 함수 ───────────────────────────────────────────────

void APTBBBMiniGame::HandleActionAInput(float TimeMs)
{
	HandleRhythmInput(EPTBActionType::ActionA, TimeMs);
}

void APTBBBMiniGame::HandleActionBInput(float TimeMs)
{
	HandleRhythmInput(EPTBActionType::ActionB, TimeMs);
}

void APTBBBMiniGame::HandleActionCInput(float TimeMs)
{
	HandleRhythmInput(EPTBActionType::ActionC, TimeMs);
}

void APTBBBMiniGame::HandleActionDInput(float TimeMs)
{
	HandleRhythmInput(EPTBActionType::ActionD, TimeMs);
}

void APTBBBMiniGame::HandleActionEInput(float TimeMs)
{
	HandleRhythmInput(EPTBActionType::ActionE, TimeMs);
}

// ── 키 → 액션 매핑 ──────────────────────────────────────────────

TMap<FKey, EPTBActionType> APTBBBMiniGame::GetActionMapping() const
{
	TMap<FKey, EPTBActionType> Map;
	Map.Add(EKeys::Z, EPTBActionType::ActionA);
	Map.Add(EKeys::X, EPTBActionType::ActionB);
	Map.Add(EKeys::C, EPTBActionType::ActionC);
	Map.Add(EKeys::V, EPTBActionType::ActionD);
	Map.Add(EKeys::B, EPTBActionType::ActionE);
	return Map;
}

// ── 게임플레이 시작 훅 ───────────────────────────────────────────

void APTBBBMiniGame::ReceiveGameplayStarted_Implementation()
{
	Super::ReceiveGameplayStarted_Implementation();

	if (RhythmConductor)
	{
		RhythmConductor->OnAllNotesPassed.AddUniqueDynamic(this, &APTBBBMiniGame::OnAllNotesDispatched);
	}
}

void APTBBBMiniGame::OnAllNotesDispatched()
{
	if (!bIsRoundActive || !AudioManager || ActiveBGMPlayingId == 0)
	{
		return;
	}

	// 주의: 이 시점에는 마지막 노트의 판정이 아직 확정되지 않았을 수 있다
	// (막판 늦은 입력/오토 미스 판정 윈도우가 아직 열려 있을 수 있음).
	// 따라서 보스 처치 확정은 여기서 하지 않고, 판정 윈도우가 확실히 닫힌
	// OnFadeFinished(BGM 페이드 종료 시점)에서 수행한다.
	AudioManager->StopBGM(BGMFadeDurationSec * 1000.0f);

	GetWorldTimerManager().SetTimer(
		TimeLimitTimerHandle, this, &APTBBBMiniGame::OnFadeFinished,
		BGMFadeDurationSec, false);
}

void APTBBBMiniGame::OnFadeFinished()
{
	if (!bIsRoundActive)
	{
		return;
	}

	// 마지막 노트까지 판정이 모두 끝난 시점 — 이때의 보스 체력으로 처치 여부를 최종 확정한다.
	if (!bBossDefeated && BossCurrentHP <= 0.f)
	{
		bBossDefeated = true;
		OnBBBossDefeated.Broadcast();
	}

	// Failed는 플레이어 체력 0(HandleJudgementResult)에서만 발생한다.
	// 채보를 끝까지 마쳤다면 목표 점수/보스 체력과 무관하게 항상 정상 종료로 처리하고,
	// 결과 등급은 아웃트로 화면에서 보스 잔여 체력으로 나눈다.
	bPendingRoundFinish = true;
}

// ── 초기화 ───────────────────────────────────────────────────────

void APTBBBMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	const UPTBBBMiniGameRuleSet* BBRuleSet = GetBBRuleSet();
	if (!BBRuleSet)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBMiniGame] BuildRuntimeState: BBRuleSet을 찾을 수 없습니다. 기본값을 사용합니다."));
	}

	const EPTBDifficulty Difficulty = GameContext.SessionRequest.Difficulty;
	const FPTBBBDifficultyConfig Config = BBRuleSet
		? BBRuleSet->GetDifficultyConfig(Difficulty)
		: FPTBBBDifficultyConfig{};

	CachedDamageTakenOnMiss = Config.DamageTakenOnMiss;
	CachedTargetScore       = Config.TargetScore;

	// HP 초기화
	PlayerMaxHP     = BBRuleSet ? BBRuleSet->PlayerMaxHP : 100.f;
	PlayerCurrentHP = PlayerMaxHP;
	bBossDefeated   = false;

	// 보스 MaxHP 및 패링 데미지 계산
	BossMaxHP = BBRuleSet ? BBRuleSet->BossMaxHP : 100.f;

	// bAutoScaleBossHP: BossMaxHP는 고정, ClearRatio를 만족하도록 DamagePerParry를 역산
	// → ClearRatio만이 실질적인 난이도 변수가 됨
	if (BBRuleSet && BBRuleSet->bAutoScaleBossHP && ChartAsset)
	{
		int32 HittableCount = 0;
		for (const FPTBNoteEvent& Note : ChartAsset->NoteEvents)
		{
			if (Note.NoteType != EPTBNoteType::Release)
			{
				++HittableCount;
			}
		}
		const float ClearRatio = FMath::Clamp(Config.ClearRatio, 0.01f, 1.f);
		const float RequiredParries = FMath::CeilToFloat(HittableCount * ClearRatio);
		CachedDamagePerParry = FMath::Max(0.01f, BossMaxHP / RequiredParries);
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBMiniGame] AutoScaleBossHP: %d 노트 x %.2f(ratio) = %.0f회 패링 필요, DamagePerParry=%.2f BossMaxHP=%.1f"),
			HittableCount, ClearRatio, RequiredParries, CachedDamagePerParry, BossMaxHP);
	}
	else
	{
		// bAutoScaleBossHP = false: DamagePerParry를 직접 설정값으로 사용
		CachedDamagePerParry = Config.DamagePerParry;
	}
	BossCurrentHP = BossMaxHP;
}

void APTBBBMiniGame::PreloadAudioAssets()
{
	Super::PreloadAudioAssets();
}

// ── Conductor 이벤트 훅 ──────────────────────────────────────────

void APTBBBMiniGame::HandleNoteCue(FPTBNoteEvent Note)
{
	Super::HandleNoteCue(Note);
	OnBBNoteCue.Broadcast(Note);
}

void APTBBBMiniGame::HandleNoteArm(FPTBNoteEvent Note)
{
	Super::HandleNoteArm(Note);   // ← JudgementSystem 판정 등록. 반드시 유지.
	OnBBNoteArm.Broadcast(Note);
}

void APTBBBMiniGame::HandleChartEvent(FPTBNoteEvent Note)
{
	Super::HandleChartEvent(Note);
	OnBBNoteReached.Broadcast(Note);
}

// ── 판정 결과 처리 ───────────────────────────────────────────────

void APTBBBMiniGame::HandleJudgementResult(FPTBJudgementResult Result)
{
	Super::HandleJudgementResult(Result);   // ← 점수/콤보/SFX/실패 조건. 반드시 유지.

	// 헛입력은 HP에 영향을 주지 않는다
	if (Result.Reason == EPTBJudgementReason::EmptyInput)
	{
		return;
	}

	// Miss(놓침, 조기 해제 포함) → 플레이어 데미지
	if (Result.JudgementType == EPTBJudgementType::Miss)
	{
		ApplyPlayerDamage(CachedDamageTakenOnMiss);
		OnBBParryFail.Broadcast(Result, GetPlayerHPPercent());

		if (MissCameraShakeClass)
		{
			if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
			{
				PC->ClientStartCameraShake(MissCameraShakeClass);
			}
		}

		// 플레이어 HP 0 실패 처리 (RuleSet 옵션이 켜져 있을 때만)
		if (PlayerCurrentHP <= 0.f && !bPendingRoundFinish)
		{
			const UPTBBBMiniGameRuleSet* BBRuleSet = GetBBRuleSet();
			if (BBRuleSet && BBRuleSet->bFailOnPlayerHPDepleted)
			{
				bPendingRoundFailed = true;
			}
		}
	}
	else
	{
		// HighPerfect / Perfect / Good → 패링 성공 → 보스 데미지
		ApplyBossDamage(CachedDamagePerParry);
		OnBBParrySuccess.Broadcast(Result, GetBossHPPercent());
	}
}

// ── 최종 결과 Payload ────────────────────────────────────────────

FPTBMiniGameResultPayload APTBBBMiniGame::BuildResultPayload() const
{
	FPTBMiniGameResultPayload Payload = Super::BuildResultPayload();
	Payload.PayloadType = TEXT("BB");

	// 보스 HP 관련
	Payload.FloatValues.Add(TEXT("FinalBossHP"),       BossCurrentHP);
	Payload.FloatValues.Add(TEXT("FinalBossHPPercent"),GetBossHPPercent());
	Payload.FloatValues.Add(TEXT("BossMaxHP"),         BossMaxHP);
	Payload.IntValues.Add(  TEXT("BossDefeated"),      bBossDefeated ? 1 : 0);

	// 플레이어 HP 관련
	Payload.FloatValues.Add(TEXT("FinalPlayerHP"),       PlayerCurrentHP);
	Payload.FloatValues.Add(TEXT("FinalPlayerHPPercent"),GetPlayerHPPercent());
	Payload.FloatValues.Add(TEXT("PlayerMaxHP"),          PlayerMaxHP);

	// 목표 점수 도달 여부
	// BuildResultPayload()는 ScoreCalculator->BuildRoundResult() 이전에 호출되므로
	// RoundResult.Score는 아직 0이다. ScoreCalculator->CurrentScore를 직접 읽어야 한다.
	const int32 CurrentScore = ScoreCalculator ? ScoreCalculator->CurrentScore : 0;
	const bool bReachedTargetScore = (CurrentScore >= CachedTargetScore);
	Payload.IntValues.Add(TEXT("TargetScore"),        CachedTargetScore);
	Payload.IntValues.Add(TEXT("ReachedTargetScore"), bReachedTargetScore ? 1 : 0);

	return Payload;
}

// ── 내부 헬퍼 ────────────────────────────────────────────────────

const UPTBBBMiniGameRuleSet* APTBBBMiniGame::GetBBRuleSet() const
{
	return Cast<UPTBBBMiniGameRuleSet>(RuleSet.Get());
}

void APTBBBMiniGame::ApplyBossDamage(float Damage)
{
	BossCurrentHP = FMath::Max(0.f, BossCurrentHP - Damage);
	OnBBBossHPChanged.Broadcast(BossCurrentHP, BossMaxHP);

	// 처치(쓰러짐 연출) 확정은 여기서 하지 않는다 — 곡 중간에 체력이 0이 되어도
	// 노트가 남아있는 한 게임은 계속되므로, 최종 처치 여부는 모든 노트가
	// 발행된 시점(OnAllNotesDispatched)에 확정한다.
}

void APTBBBMiniGame::ApplyPlayerDamage(float Damage)
{
	PlayerCurrentHP = FMath::Max(0.f, PlayerCurrentHP - Damage);
	OnBBPlayerHPChanged.Broadcast(PlayerCurrentHP, PlayerMaxHP);
}
