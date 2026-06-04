#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"

void UPTBActionCueWidgetBase::SetActionType(EPTBActionType InActionType)
{
	CurrentActionType = InActionType;
	OnActionTypeSet(InActionType);
}

FText UPTBActionCueWidgetBase::GetActionKeyLabel(EPTBActionType InActionType)
{
	switch (InActionType)
	{
	case EPTBActionType::ActionA: return FText::FromString(TEXT("Z"));
	case EPTBActionType::ActionB: return FText::FromString(TEXT("X"));
	case EPTBActionType::ActionC: return FText::FromString(TEXT("C"));
	case EPTBActionType::ActionD: return FText::FromString(TEXT("V"));
	case EPTBActionType::ActionE: return FText::FromString(TEXT("B"));
	default:                      return FText::FromString(TEXT("?"));
	}
}

FLinearColor UPTBActionCueWidgetBase::GetActionColor(EPTBActionType InActionType)
{
	switch (InActionType)
	{
	case EPTBActionType::ActionA: return FLinearColor(1.00f, 0.20f, 0.20f, 1.f); // 빨강  (Z)
	case EPTBActionType::ActionB: return FLinearColor(0.20f, 0.50f, 1.00f, 1.f); // 파랑  (X)
	case EPTBActionType::ActionC: return FLinearColor(1.00f, 0.85f, 0.00f, 1.f); // 노랑  (C)
	case EPTBActionType::ActionD: return FLinearColor(0.10f, 0.90f, 0.30f, 1.f); // 초록  (V)
	case EPTBActionType::ActionE: return FLinearColor(0.65f, 0.10f, 1.00f, 1.f); // 보라  (B)
	default:                      return FLinearColor::White;
	}
}
