#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "UI/PTBKeyTextLibrary.h"

void UPTBActionCueWidgetBase::SetActionType(EPTBActionType InActionType)
{
	CurrentActionType = InActionType;
	OnActionTypeSet(InActionType);
}

FText UPTBActionCueWidgetBase::GetActionKeyLabel(EPTBActionType InActionType) const
{
	// 실제 키 바인딩 기준 표시 (폴백 포함). 스토리/튜토리얼과 같은 공용 유틸 사용
	return UPTBKeyTextLibrary::GetActionKeyLabel(this, InActionType);
}

FLinearColor UPTBActionCueWidgetBase::GetActionColor(EPTBActionType InActionType)
{
	switch (InActionType)
	{
	case EPTBActionType::ActionA: return FLinearColor(1.00f, 0.20f, 0.20f, 1.f); // 빨강
	case EPTBActionType::ActionB: return FLinearColor(0.20f, 0.50f, 1.00f, 1.f); // 파랑
	case EPTBActionType::ActionC: return FLinearColor(1.00f, 0.85f, 0.00f, 1.f); // 노랑
	case EPTBActionType::ActionD: return FLinearColor(0.10f, 0.90f, 0.30f, 1.f); // 초록
	case EPTBActionType::ActionE: return FLinearColor(0.65f, 0.10f, 1.00f, 1.f); // 보라
	default:                      return FLinearColor::White;
	}
}
