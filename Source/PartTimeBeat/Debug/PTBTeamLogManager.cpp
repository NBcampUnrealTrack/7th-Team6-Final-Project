// Fill out your copyright notice in the Description page of Project Settings.


#include "PTBTeamLogManager.h"

void UPTBTeamLogManager::LogRhythmJudge(const FPTBJudgementResult& Result, const FPTBLogContext& Context)
{
}

void UPTBTeamLogManager::LogWWiseEvent(FName EventName, int32 AkResultCode, const FPTBLogContext& Context)
{
}

void UPTBTeamLogManager::LogFlowTransition(EGameFlowState From, EGameFlowState To, const FPTBLogContext& Context)
{
}

void UPTBTeamLogManager::LogMultiResultMismatch(const FString& PlayerId, const FPTBRoundResult& Local,
	const FPTBRoundResult& Server, const FPTBLogContext& Context)
{
}

void UPTBTeamLogManager::FlushSessionLog()
{
}
