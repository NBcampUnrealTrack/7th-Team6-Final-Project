#include "Rhythm/PTBScoreCalculator.h"

int32 UPTBScoreCalculator::AddJudgementScore(const FPTBJudgementResult& Result)
{
	return 0;
}

FPTBRoundResult UPTBScoreCalculator::BuildRoundResult(const FString& ProfileId, FName MiniGameId,
	EPTBDifficulty Difficulty, const FPTBMiniGameResultPayload& Payload) const
{
	return FPTBRoundResult();
}

int32 UPTBScoreCalculator::CalculateStarRating(int32 Score) const
{
	return 0;
}

EPTBGradeType UPTBScoreCalculator::CalculateGrade() const
{
	return EPTBGradeType();
}

void UPTBScoreCalculator::Reset()
{

}