// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PTBSessionSubsystem.generated.h"

/**
 * Steam/LAN 세션 생성, 검색, 참가를 담당하는 서브시스템
 */
UCLASS()
class PARTTIMEBEAT_API UPTBSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|Multiplayer")
	void CreateSession(int32 InMaxPlayers, bool bIsLANMatch);

	UFUNCTION(BlueprintCallable, Category = "PTB|Multiplayer")
	void FindSessions(int32 MaxSearchResults, bool bIsLANQuery);

	UFUNCTION(BlueprintCallable, Category = "PTB|Multiplayer")
	void JoinSessionByIndex(int32 SessionIndex);

public:
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Multiplayer")
	bool bIsHost = false;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Multiplayer")
	int32 MaxPlayers = 4;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Multiplayer")
	FName CurrentSessionName = NAME_None;
};