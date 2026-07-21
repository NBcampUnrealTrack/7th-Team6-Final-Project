// Fill out your copyright notice in the Description page of Project Settings.


#include "Multiplayer/PTBSessionSubsystem.h"

void UPTBSessionSubsystem::CreateSession(int32 InMaxPlayers, bool bIsLANMatch)
{
	bIsHost = true;
	MaxPlayers = InMaxPlayers;
	CurrentSessionName = TEXT("PTBGameSession");

	// TODO:
	// IOnlineSessionPtr를 사용해 Steam/LAN 세션 생성 로직 구현
	// bIsLANMatch 값에 따라 LAN/Steam 세션 설정 분기
}

void UPTBSessionSubsystem::FindSessions(int32 MaxSearchResults, bool bIsLANQuery)
{
	// TODO:
	// OnlineSubsystem 세션 검색 로직 구현
	// MaxSearchResults: 최대 검색 개수
	// bIsLANQuery: LAN 검색 여부
}

void UPTBSessionSubsystem::JoinSessionByIndex(int32 SessionIndex)
{
	// TODO:
	// 검색 결과 배열에서 SessionIndex에 해당하는 세션 참가
}