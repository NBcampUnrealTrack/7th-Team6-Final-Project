// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "UObject/Interface.h"
#include "PTBInteractable.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UPTBInteractable : public UInterface
{
	GENERATED_BODY()
};


class PARTTIMEBEAT_API IPTBInteractable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**초기화*/
	virtual void InitGame(FPTBMiniGameContext& Context) = 0;
	/**노트도달*/
	virtual void OnNoteTriggered(FPTBNoteEvent& NoteEvent)=0;
	/**선행큐*/
	virtual void OnNoteCue(FPTBNoteEvent& NoteEvent)=0;
	/**입력*/
	virtual EPTBJudgementType OnInputReceived(EPTBJudgementType Type,float TimeMs)=0;
	/** 종료*/
	virtual FPTBRoundResult OnGame()=0;
	/** 일시정지*/
	virtual void OnPause()=0;
	/**재개 */
	virtual void OnResume()=0;
	/**키 맵핑*/
	virtual TMap<FKey, EPTBActionType> GetActionMapping()=0;
};

