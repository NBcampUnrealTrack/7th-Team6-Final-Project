#include "MiniGames/BB/PTBBBCueWidgetBase.h"

void UPTBBBCueWidgetBase::InitCue(int32 InNoteId, EPTBActionType InActionType, float InApproachDurationSec)
{
	NoteId = InNoteId;
	SetActionType(InActionType);   // OnActionTypeSet() 호출 → BP에서 색·텍스트 반영
	OnCueStarted(InNoteId, InActionType, InApproachDurationSec);
}

void UPTBBBCueWidgetBase::RemoveCue()
{
	OnCueRemoved();
	RemoveFromParent();
}
