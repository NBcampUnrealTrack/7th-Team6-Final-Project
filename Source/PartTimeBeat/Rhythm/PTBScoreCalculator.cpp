#include "Rhythm/PTBScoreCalculator.h"

namespace PTBScoreCalculatorInternal
{
	constexpr int32 MaxScore = 1000000;
	constexpr int32 MaxComboBonusCount = 50;

	float GetAccuracyWeight(EPTBJudgementType JudgementType)
	{
		switch (JudgementType)
		{
		case EPTBJudgementType::HighPerfect:
		case EPTBJudgementType::Perfect:
			return 1.0f;
		case EPTBJudgementType::Good:
			return 0.7f;
		case EPTBJudgementType::Miss:
		default:
			return 0.0f;
		}
	}

	float CalculateAccuracyRate(int32 HighPerfectCount, int32 PerfectCount, int32 GoodCount, int32 MissCount)
	{
		const int32 TotalCount = HighPerfectCount + PerfectCount + GoodCount + MissCount;
		if (TotalCount <= 0)
		{
			return 0.0f;
		}

		const float WeightedCount =
			static_cast<float>(HighPerfectCount) * GetAccuracyWeight(EPTBJudgementType::HighPerfect)
			+ static_cast<float>(PerfectCount) * GetAccuracyWeight(EPTBJudgementType::Perfect)
			+ static_cast<float>(GoodCount) * GetAccuracyWeight(EPTBJudgementType::Good);

		return FMath::Clamp(WeightedCount / static_cast<float>(TotalCount), 0.0f, 1.0f);
	}
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

	if (Result.bBreaksCombo || Result.JudgementType == EPTBJudgementType::Miss)
	{
		const int32 FinalCombo = ComboCount;
		ComboCount = 0;

		if (FinalCombo > 0)
		{
			OnComboBreak.Broadcast(FinalCombo);
		}

		return 0;
	}

	++ComboCount;
	MaxCombo = FMath::Max(MaxCombo, ComboCount);

	const int32 ComboBonusCount = FMath::Min(ComboCount, PTBScoreCalculatorInternal::MaxComboBonusCount);
	const float ScoreMultiplier = 1.0f + static_cast<float>(ComboBonusCount) / 100.0f;
	const int32 ScoreDelta = FMath::RoundToInt(static_cast<float>(Result.ScoreDelta) * ScoreMultiplier);
	const int32 RemainingScore = FMath::Max(0, PTBScoreCalculatorInternal::MaxScore - CurrentScore);
	const int32 ClampedScoreDelta = FMath::Clamp(ScoreDelta, 0, RemainingScore);

	CurrentScore += ClampedScoreDelta;
	return ClampedScoreDelta;
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

	Result.MiniGameId = MiniGameId;
	Result.Difficulty = Difficulty;
	Result.Score = CurrentScore;
	Result.HighPerfectCount = HighPerfectCount;
	Result.PerfectCount = PerfectCount;
	Result.GoodCount = GoodCount;
	Result.MissCount = MissCount;
	Result.MaxCombo = MaxCombo;
	Result.AccuracyRate = PTBScoreCalculatorInternal::CalculateAccuracyRate(
		HighPerfectCount,
		PerfectCount,
		GoodCount,
		MissCount);
	Result.Grade = CalculateGrade();
	Result.StarCount = CalculateStarRating(CurrentScore);
	Result.EarnedMoney = 0;
	Result.IsNewHighScore = false;
	Result.MiniGamePayload = Payload;

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

	if (MissCount == 0 && GoodCount == 0)
	{
		return EPTBGradeType::PerfectFullCombo;
	}

	if (MissCount == 0)
	{
		return EPTBGradeType::FullCombo;
	}

	const float AccuracyRate = PTBScoreCalculatorInternal::CalculateAccuracyRate(
		HighPerfectCount,
		PerfectCount,
		GoodCount,
		MissCount);

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
}
