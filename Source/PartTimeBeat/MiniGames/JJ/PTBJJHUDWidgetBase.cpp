// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGames/JJ/PTBJJHUDWidgetBase.h"
#include "MiniGames/JJ/PTBJJMiniGame.h"

void UPTBJJHUDWidgetBase::BindToMiniGame(APTBJJMiniGame* InMiniGame)
{
	// 기존 연결 해제(중복 바인딩 방지)
	if (MiniGame)
	{
		MiniGame->OnJJLanding.RemoveDynamic(this, &UPTBJJHUDWidgetBase::HandleLanding);
	}

	MiniGame = InMiniGame;

	if (MiniGame)
	{
		MiniGame->OnJJLanding.AddDynamic(this, &UPTBJJHUDWidgetBase::HandleLanding);
	}
}

void UPTBJJHUDWidgetBase::HandleLanding(int32 CharacterIndex, FPTBJudgementResult Result, FPTBNoteEvent Note)
{
	// BP로 전달 → 판정 위젯에 ShowJudgement 호출
	OnJudgement(CharacterIndex, Result.JudgementType, Result);
}

void UPTBJJHUDWidgetBase::NativeDestruct()
{
	// 위젯 파괴 시 델리게이트 정리(댕글링 방지)
	if (MiniGame)
	{
		MiniGame->OnJJLanding.RemoveDynamic(this, &UPTBJJHUDWidgetBase::HandleLanding);
		MiniGame = nullptr;
	}

	Super::NativeDestruct();
}