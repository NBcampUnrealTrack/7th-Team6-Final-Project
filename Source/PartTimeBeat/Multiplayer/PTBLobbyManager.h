// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "PTBLobbyManager.generated.h"

enum class EPTBDifficulty : uint8;

/**
 * 로비 참가자, Ready 상태, 선택된 미니게임 정보를 관리하는 클래스
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBLobbyManager : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|Lobby")
	void SetReady(const FString& PlayerId, bool bReady);

	UFUNCTION(BlueprintCallable, Category = "PTB|Lobby")
	bool CheckAllReady() const;

	UFUNCTION(BlueprintCallable, Category = "PTB|Lobby")
	FPTBGameSessionRequest BuildSessionRequest() const;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
	TArray<FPTBLobbyPlayer> LobbyPlayers;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
	FName SelectedMiniGameId = NAME_None;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
	EPTBDifficulty SelectedDifficulty = EPTBDifficulty::Standard;
};