#include "MiniGames/TG/PTBTGCueWidgetBase.h"

void UPTBTGCueWidgetBase::InitCue(int32 InNoteId, EPTBActionType InActionType)
{
	NoteId = InNoteId;
	SetActionType(InActionType);   // OnActionTypeSet() 호출 → BP에서 색·텍스트 반영
	OnCueStarted(InNoteId, InActionType);
}

void UPTBTGCueWidgetBase::RemoveCue()
{
	OnCueRemoved();
	RemoveFromParent();
}
