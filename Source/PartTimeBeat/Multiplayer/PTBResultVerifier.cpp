// Fill out your copyright notice in the Description page of Project Settings.

#include "Multiplayer/PTBResultVerifier.h"

void UPTBResultVerifier::SubmitClientResult(const FString& PlayerId, const FPTBRoundResult& Result)
{
	if (!VerifyResult(PlayerId, Result))
	{
		return;
	}

	ServerResults.Add(PlayerId, Result);
	CachedRankingList = BuildRanking();
}

bool UPTBResultVerifier::VerifyResult(const FString& PlayerId, const FPTBRoundResult& Result) const
{
	if (PlayerId.IsEmpty())
	{
		return false;
	}

	if (Result.Score < 0 || Result.Score > 1000000)
	{
		return false;
	}

	if (Result.AccuracyRate < 0.0f || Result.AccuracyRate > 1.0f)
	{
		return false;
	}

	if (Result.StarCount < 0 || Result.StarCount > 3)
	{
		return false;
	}

	return true;
}

TArray<FPTBPlayerRanking> UPTBResultVerifier::BuildRanking() const
{
	TArray<FPTBPlayerRanking> RankingList;

	for (const TPair<FString, FPTBRoundResult>& Pair : ServerResults)
	{
		FPTBPlayerRanking Ranking;
		Ranking.PlayerId = Pair.Key;
		Ranking.Score = Pair.Value.Score;
		Ranking.MaxCombo = Pair.Value.MaxCombo;
		Ranking.AccuracyRate = Pair.Value.AccuracyRate;

		RankingList.Add(Ranking);
	}

	RankingList.Sort([](const FPTBPlayerRanking& A, const FPTBPlayerRanking& B)
	{
		if (A.Score != B.Score)
		{
			return A.Score > B.Score;
		}

		if (A.AccuracyRate != B.AccuracyRate)
		{
			return A.AccuracyRate > B.AccuracyRate;
		}

		return A.MaxCombo > B.MaxCombo;
	});

	for (int32 Index = 0; Index < RankingList.Num(); ++Index)
	{
		RankingList[Index].Rank = Index + 1;
	}

	return RankingList;
}