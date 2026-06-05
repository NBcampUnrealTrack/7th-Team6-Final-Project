#include "MiniGames/Common/UI/PTBActionCueWidgetBase.h"
#include "Core/PTBGameInstance.h"

void UPTBActionCueWidgetBase::SetActionType(EPTBActionType InActionType)
{
	CurrentActionType = InActionType;
	OnActionTypeSet(InActionType);
}

FText UPTBActionCueWidgetBase::GetActionKeyLabel(EPTBActionType InActionType) const
{
	// GameInstance의 실제 키 바인딩 기준으로 표시
	if (const UWorld* World = GetWorld())
	{
		if (const UPTBGameInstance* GI = Cast<UPTBGameInstance>(World->GetGameInstance()))
		{
			const FKey BoundKey = GI->CachedSettings.RhythmKeys.ResolveAction(InActionType);
			if (BoundKey.IsValid() && BoundKey != EKeys::Invalid)
			{
				return BoundKey.GetDisplayName();
			}
		}
	}

	// 폴백: 기본 바인딩(Z/X/C/V/B)
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
	case EPTBActionType::ActionA: return FLinearColor(1.00f, 0.20f, 0.20f, 1.f); // 빨강
	case EPTBActionType::ActionB: return FLinearColor(0.20f, 0.50f, 1.00f, 1.f); // 파랑
	case EPTBActionType::ActionC: return FLinearColor(1.00f, 0.85f, 0.00f, 1.f); // 노랑
	case EPTBActionType::ActionD: return FLinearColor(0.10f, 0.90f, 0.30f, 1.f); // 초록
	case EPTBActionType::ActionE: return FLinearColor(0.65f, 0.10f, 1.00f, 1.f); // 보라
	default:                      return FLinearColor::White;
	}
}
