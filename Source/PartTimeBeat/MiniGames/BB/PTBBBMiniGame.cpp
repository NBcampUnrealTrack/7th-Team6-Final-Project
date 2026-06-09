#include "MiniGames/BB/PTBBBMiniGame.h"
#include "MiniGames/BB/PTBBBMiniGameRuleSet.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Debug/PTBTeamLog.h"
#include "Rhythm/PTBScoreCalculator.h"
#include "Rhythm/PTBRhythmChartAsset.h"

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

// ── 초기화 ───────────────────────────────────────────────────────

void APTBBBMiniGame::BuildRuntimeState()
{
	Super::BuildRuntimeState();

	const UPTBBBMiniGameRuleSet* BBRuleSet = GetBBRuleSet();
	if (!BBRuleSet)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBMiniGame] BuildRuntimeState: BBRuleSet을 찾을 수 없습니다. 기본값을 사용합니다."));
	}

	// 난이도 수치 캐시 (BossMaxHP 자동 계산에 DamagePerParry가 필요하므로 먼저 처리)
	const EPTBDifficulty Difficulty = GameContext.SessionRequest.Difficulty;
	const FPTBBBDifficultyConfig Config = BBRuleSet
		? BBRuleSet->GetDifficultyConfig(Difficulty)
		: FPTBBBDifficultyConfig{};

	CachedDamagePerParry    = Config.DamagePerParry;
	CachedDamageTakenOnMiss = Config.DamageTakenOnMiss;
	CachedTargetScore       = Config.TargetScore;

	// HP 초기화
	PlayerMaxHP     = BBRuleSet ? BBRuleSet->PlayerMaxHP : 100.f;
	PlayerCurrentHP = PlayerMaxHP;
	bBossDefeated   = false;

	// 보스 MaxHP: bAutoScaleBossHP이면 채보 노트 수 × DamagePerParry로 자동 계산
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
		BossMaxHP = FMath::Max(1.f, RequiredParries * CachedDamagePerParry);
		PTB_WARNING(LogPTBMiniGames, TEXT("[BBMiniGame] AutoScaleBossHP: %d 노트 x %.2f(ratio) = %.0f회 패링 필요, BossMaxHP=%.1f"),
			HittableCount, ClearRatio, RequiredParries, BossMaxHP);
	}
	else
	{
		BossMaxHP = BBRuleSet ? BBRuleSet->BossMaxHP : 100.f;
	}
	BossCurrentHP = BossMaxHP;
}

void APTBBBMiniGame::PreloadAudioAssets()
{
	Super::PreloadAudioAssets();

	// BGM Bank 선로드
	if (AudioManager && !GameContext.ChartData.WwiseBankName.IsNone())
	{
		AudioManager->LoadSoundBank(GameContext.ChartData.WwiseBankName);
	}
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

		// 플레이어 HP 0 실패 처리 (RuleSet 옵션이 켜져 있을 때만)
		if (PlayerCurrentHP <= 0.f && !bPendingRoundFinish)
		{
			const UPTBBBMiniGameRuleSet* BBRuleSet = GetBBRuleSet();
			if (BBRuleSet && BBRuleSet->bFailOnPlayerHPDepleted)
			{
				FinishMiniGame(EPTBRoundEndReason::Failed);
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
	const float PrevHP = BossCurrentHP;
	BossCurrentHP = FMath::Max(0.f, BossCurrentHP - Damage);

	OnBBBossHPChanged.Broadcast(BossCurrentHP, BossMaxHP);

	// HP 0 최초 도달 시에만 이벤트 발행 (이후 추가 데미지에는 발행 안 함)
	if (PrevHP > 0.f && BossCurrentHP <= 0.f)
	{
		bBossDefeated = true;
		OnBBBossDefeated.Broadcast();
	}
}

void APTBBBMiniGame::ApplyPlayerDamage(float Damage)
{
	PlayerCurrentHP = FMath::Max(0.f, PlayerCurrentHP - Damage);
	OnBBPlayerHPChanged.Broadcast(PlayerCurrentHP, PlayerMaxHP);
}
