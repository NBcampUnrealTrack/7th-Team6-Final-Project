// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTBTeamLogManager.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
// ai사용 무슨 타입인지 몰라서 대충 만들어달라고함 h 58번째줄용
struct FPTBLogEntry
{
	GENERATED_BODY()

	FDateTime Timestamp; // 로그가 찍힌 시간
	FName ChannelName;   // 어떤 종류의 로그인가 (Rhythm, Wwise 등)
	FString Message;     // 진짜 찍힌 문장 ("판정 성공! 콤보: 10")
};
UCLASS()
class PARTTIMEBEAT_API UPTBTeamLogManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/**판정 로그*/
	UFUNCTION()
	void LogRhythmJudge(const FPTBJudgementResult& Result,const FPTBLogContext& Context);
	
	/** WWise 결과*/
	UFUNCTION()
	void LogWWiseEvent(FName EventName, int32 AkResultCode, const FPTBLogContext& Context);
	
	/**플로우 전환*/
	UFUNCTION()
	void LogFlowTransition(EGameFlowState From, EGameFlowState To, const FPTBLogContext& Context);
	
	/**멀티 불일치*/
	UFUNCTION()
	void LogMultiResultMismatch(const FString& PlayerId, const FPTBRoundResult& Local, const FPTBRoundResult& Server, const FPTBLogContext& Context);
	
	/**파일저장*/
	UFUNCTION()
	void FlushSessionLog();
	
private:
	/** 현재 세션*/
	UPROPERTY()
	FGuid SessionId;
	/** 활성채널 */
	UPROPERTY()
	TSet<FName> EnabledChannels;
	/**메모리 버퍼*/
	UPROPERTY()
	TArray<FPTBLogEntry> LogBuffer;
	/** 파일 출력여부*/
	UPROPERTY()
	bool bWriteToFile;
};
