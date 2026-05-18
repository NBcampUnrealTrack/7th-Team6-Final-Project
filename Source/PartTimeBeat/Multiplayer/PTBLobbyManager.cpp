// Fill out your copyright notice in the Description page of Project Settings.

#include "Multiplayer/PTBLobbyManager.h"
#include "Core/PTBStructEnums.h"

void UPTBLobbyManager::SetReady(const FString& PlayerId, bool bReady)
{
	for (FPTBLobbyPlayer& Player : LobbyPlayers)
	{
		if (Player.PlayerId == PlayerId)
		{
			Player.bReady = bReady;
			return;
		}
	}
}

bool UPTBLobbyManager::CheckAllReady() const
{
	if (LobbyPlayers.IsEmpty())
	{
		return false;
	}

	for (const FPTBLobbyPlayer& Player : LobbyPlayers)
	{
		if (!Player.bReady)
		{
			return false;
		}
	}

	return true;
}

FPTBGameSessionRequest UPTBLobbyManager::BuildSessionRequest() const
{
	FPTBGameSessionRequest Request;

	Request.MiniGameId = SelectedMiniGameId;
	Request.MiniGameCode = SelectedMiniGameId;
	Request.Difficulty = SelectedDifficulty;
	Request.PlayMode = 
	(LobbyPlayers.Num() > 1)
	? EPTBPlayMode::Multiplayer
	: EPTBPlayMode::Single;
	Request.RandomSeed = FMath::Rand();
	Request.ExpectedPlayerCount = LobbyPlayers.Num();

	return Request;
}