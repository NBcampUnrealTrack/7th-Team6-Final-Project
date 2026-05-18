// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "PTBResultVerifier.generated.h"

/**
 * 멀티 결과의 신뢰성을 검증하고 순위를 생성하는 클래스
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBResultVerifier : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|Result")
	void SubmitClientResult(const FString& PlayerId, const FPTBRoundResult& Result);

	UFUNCTION(BlueprintCallable, Category = "PTB|Result")
	bool VerifyResult(const FString& PlayerId, const FPTBRoundResult& Result) const;

	UFUNCTION(BlueprintCallable, Category = "PTB|Result")
	TArray<FPTBPlayerRanking> BuildRanking() const;

public:
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Result")
	TMap<FString, FPTBRoundResult> ServerResults;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Result")
	TArray<FPTBPlayerRanking> CachedRankingList;
};