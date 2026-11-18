#include "Rhythm/PTBScoreCalculator.h"

namespace PTBScoreCalculatorInternal
{
	// 골드·별용 예전 방식 점수 (11/17 이전 계산식 그대로)
	constexpr int32 LegacyMaxComboBonusCount = 50;
}

void UPTBScoreCalculator::SetExpectedNoteCount(int32 InExpectedNoteCount)
{
	ExpectedNoteCount = FMath::Max(0, InExpectedNoteCount);
	CurrentScore = ComputeNormalizedScore();
}

int32 UPTBScoreCalculator::GetJudgementWeight(EPTBJudgementType JudgementType)
{
	switch (JudgementType)
	{
	case EPTBJudgementType::HighPerfect:
	case EPTBJudgementType::Perfect:
		return WeightPerfect;
	case EPTBJudgementType::Good:
		return WeightGood;
	case EPTBJudgementType::Miss:
	default:
		return 0;
	}
}

int32 UPTBScoreCalculator::GetScoreDenominatorNoteCount() const
{
	// 빈 입력 Miss 등으로 판정 수가 채보 노트 수보다 많아지면 판정 수를 분모로 (100만 초과 방지)
	return FMath::Max(ExpectedNoteCount, TotalNoteCount);
}

int32 UPTBScoreCalculator::ComputeNormalizedScore() const
{
	const int64 DenominatorNotes = GetScoreDenominatorNoteCount();
	if (DenominatorNotes <= 0)
	{
		return 0;
	}

	const int64 MaxWeightSum = DenominatorNotes * WeightPerfect;
	const int64 Score = (static_cast<int64>(MaxScore) * WeightSum) / MaxWeightSum; // 정수 나눗셈 = floor
	return static_cast<int32>(FMath::Clamp<int64>(Score, 0, MaxScore));
}

int32 UPTBScoreCalculator::AddJudgementScore(const FPTBJudgementResult& Result)
{
	++TotalNoteCount;

	switch (Result.JudgementType)
	{
	case EPTBJudgementType::HighPerfect:
		++HighPerfectCount;
		break;
	case EPTBJudgementType::Perfect:
		++PerfectCount;
		break;
	case EPTBJudgementType::Good:
		++GoodCount;
		break;
	case EPTBJudgementType::Miss:
	default:
		++MissCount;
		break;
	}

	const bool bBreaksCombo = Result.bBreaksCombo || Result.JudgementType == EPTBJudgementType::Miss;

	// ── 정규화 점수 (판정 종류만으로 계산. 미니게임별 ScoreDelta 배율은 점수에 반영하지 않음)
	if (!bBreaksCombo)
	{
		WeightSum += GetJudgementWeight(Result.JudgementType);
	}
	const int32 PrevScore = CurrentScore;
	CurrentScore = ComputeNormalizedScore();

	// ── 콤보 (연출/기록용)
	if (bBreaksCombo)
	{
		const int32 FinalCombo = ComboCount;
		ComboCount = 0;

		if (FinalCombo > 0)
		{
			OnComboBreak.Broadcast(FinalCombo);
		}
	}
	else
	{
		++ComboCount;
		MaxCombo = FMath::Max(MaxCombo, ComboCount);

		// ── 골드·별용 예전 방식 점수 (기본점수 × 콤보 배율)
		const int32 ComboBonusCount = FMath::Min(ComboCount, PTBScoreCalculatorInternal::LegacyMaxComboBonusCount);
		const float ScoreMultiplier = 1.0f + static_cast<float>(ComboBonusCount) / 100.0f;
		const int32 LegacyDelta = FMath::RoundToInt(static_cast<float>(Result.ScoreDelta) * ScoreMultiplier);
		const int32 LegacyRemaining = FMath::Max(0, MaxScore - LegacyScore);
		LegacyScore += FMath::Clamp(LegacyDelta, 0, LegacyRemaining);
	}

	// 분모가 늘어나는 경우(판정 수 > 총 노트 수) 점수가 줄 수 있으므로 0 미만은 0으로
	return FMath::Max(0, CurrentScore - PrevScore);
}

FPTBRoundResult UPTBScoreCalculator::BuildRoundResult(const FString& ProfileId, FName MiniGameId,
	EPTBDifficulty Difficulty, const FPTBMiniGameResultPayload& Payload) const
{
	FPTBRoundResult Result;

	FGuid ParsedProfileId;
	if (FGuid::Parse(ProfileId, ParsedProfileId))
	{
		Result.ProfileId = ParsedProfileId;
	}

	const int32 JudgedCount = HighPerfectCount + PerfectCount + GoodCount + MissCount;
	if (ExpectedNoteCount > 0 && JudgedCount != ExpectedNoteCount)
	{
		// 끝까지 플레이했는데 수가 다르면 미니게임 쪽 판정 횟수(홀드/동시입력/빈입력) 확인 필요
		UE_LOG(LogTemp, Log, TEXT("[Score] %s 판정 수(%d) != 채보 노트 수(%d). 분모=%d (중간 실패·빈 입력 Miss면 정상)"),
			*MiniGameId.ToString(), JudgedCount, ExpectedNoteCount, GetScoreDenominatorNoteCount());
	}

	Result.MiniGameId = MiniGameId;
	Result.Difficulty = Difficulty;
	Result.Score = CurrentScore;
	Result.ScorePercent = ScoreToPercent(CurrentScore);
	Result.HighPerfectCount = HighPerfectCount;
	Result.PerfectCount = PerfectCount;
	Result.GoodCount = GoodCount;
	Result.MissCount = MissCount;
	Result.MaxCombo = MaxCombo;
	Result.AccuracyRate = static_cast<float>(CurrentScore) / static_cast<float>(MaxScore);
	Result.Grade = CalculateGrade();

	const bool bPlayedAll = JudgedCount > 0 && JudgedCount >= ExpectedNoteCount;
	Result.bIsFullCombo = bPlayedAll && MissCount == 0;
	Result.bIsPerfectFullCombo = Result.bIsFullCombo && GoodCount == 0;
	Result.bIsMax = Result.bIsPerfectFullCombo && PerfectCount == 0;

	Result.StarCount = CalculateStarRating(LegacyScore);
	Result.EarnedMoney = LegacyScore;
	Result.IsNewHighScore = false;
	Result.MiniGamePayload = Payload;

	UE_LOG(LogTemp, Log, TEXT("[Score] %s 결과: %d점 (%s) HP=%d P=%d G=%d M=%d / 노트 %d / 골드 %d"),
		*MiniGameId.ToString(), CurrentScore, *ScoreToPercentText(CurrentScore).ToString(),
		HighPerfectCount, PerfectCount, GoodCount, MissCount, ExpectedNoteCount, LegacyScore);

	return Result;
}

int32 UPTBScoreCalculator::CalculateStarRating(int32 Score) const
{
	if (Score >= 800000)
	{
		return 3;
	}

	if (Score >= 500000)
	{
		return 2;
	}

	if (Score >= 250000)
	{
		return 1;
	}

	return 0;
}

EPTBGradeType UPTBScoreCalculator::CalculateGrade() const
{
	const int32 JudgedNoteCount = HighPerfectCount + PerfectCount + GoodCount + MissCount;
	if (JudgedNoteCount <= 0)
	{
		return EPTBGradeType::Fail;
	}

	// TODO(11/17 문서): 등급(D~S)과 풀콤보 계열(FC/PFC/MAX)을 분리할 예정.
	// 저장 데이터·발광모드 해금 조건이 이 enum 순서를 쓰고 있어서 이번엔 구조 유지, 정확도만 점수 기준으로 통일.
	if (MissCount == 0 && GoodCount == 0)
	{
		return EPTBGradeType::PerfectFullCombo;
	}

	if (MissCount == 0)
	{
		return EPTBGradeType::FullCombo;
	}

	const float AccuracyRate = static_cast<float>(CurrentScore) / static_cast<float>(MaxScore);

	if (AccuracyRate >= 0.95f)
	{
		return EPTBGradeType::S;
	}

	if (AccuracyRate >= 0.85f)
	{
		return EPTBGradeType::A;
	}

	if (AccuracyRate >= 0.70f)
	{
		return EPTBGradeType::B;
	}

	if (AccuracyRate >= 0.60f)
	{
		return EPTBGradeType::C;
	}

	if (AccuracyRate >= 0.50f)
	{
		return EPTBGradeType::Clear;
	}

	return EPTBGradeType::Fail;
}

float UPTBScoreCalculator::ScoreToPercent(int32 Score)
{
	// 소수점 2자리 버림: 965239 → 9652 → 96.52
	const int32 Hundredths = FMath::Clamp(Score, 0, MaxScore) / 100;
	return static_cast<float>(Hundredths) / 100.0f;
}

FText UPTBScoreCalculator::ScoreToPercentText(int32 Score)
{
	const int32 Hundredths = FMath::Clamp(Score, 0, MaxScore) / 100;
	return FText::FromString(FString::Printf(TEXT("%d.%02d%%"), Hundredths / 100, Hundredths % 100));
}

void UPTBScoreCalculator::Reset()
{
	CurrentScore = 0;
	ComboCount = 0;
	MaxCombo = 0;
	HighPerfectCount = 0;
	PerfectCount = 0;
	GoodCount = 0;
	MissCount = 0;
	TotalNoteCount = 0;
	WeightSum = 0;
	LegacyScore = 0;
}
