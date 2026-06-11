// Fill out your copyright notice in the Description page of Project Settings.


#include "MiniGames/JJ/PTBJJCueWidgetBase.h"

void UPTBJJCueWidgetBase::InitCue(int32 InNoteId, EPTBActionType InActionType)
{
	NoteId = InNoteId;
	SetActionType(InActionType);   // OnActionTypeSet() 호출 → BP에서 색·텍스트 반영
	OnCueStarted(InNoteId, InActionType);
}

void UPTBJJCueWidgetBase::RemoveCue()
{
	OnCueRemoved();
	RemoveFromParent();
}
