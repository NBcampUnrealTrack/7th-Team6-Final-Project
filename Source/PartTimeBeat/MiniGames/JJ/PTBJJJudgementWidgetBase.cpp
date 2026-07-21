// Fill out your copyright notice in the Description page of Project Settings.

#include "MiniGames/JJ/PTBJJJudgementWidgetBase.h"

void UPTBJJJudgementWidgetBase::ShowJudgement(EPTBJudgementType InJudgementType, int32 InCharacterIndex)
{
	JudgementType = InJudgementType;
	CharacterIndex = InCharacterIndex;

	const FText Label = GetLabelForJudgement(InJudgementType);
	const FLinearColor Color = GetColorForJudgement(InJudgementType);

	// BP에서 텍스트 세팅 + 팝업 애니메이션 재생
	OnJudgementShown(InJudgementType, Label, Color, InCharacterIndex);
}

FText UPTBJJJudgementWidgetBase::GetLabelForJudgement_Implementation(EPTBJudgementType InJudgementType) const
{
	switch (InJudgementType)
	{
	case EPTBJudgementType::HighPerfect: return NSLOCTEXT("PTBJJ", "HighPerfect", "HighPerfect");
	case EPTBJudgementType::Perfect:     return NSLOCTEXT("PTBJJ", "Perfect", "PERFECT");
	case EPTBJudgementType::Good:        return NSLOCTEXT("PTBJJ", "Good", "GOOD");
	case EPTBJudgementType::Miss:        return NSLOCTEXT("PTBJJ", "Miss", "MISS");
	default:                             return FText::GetEmpty();
	}
}

FLinearColor UPTBJJJudgementWidgetBase::GetColorForJudgement_Implementation(EPTBJudgementType InJudgementType) const
{
	// JumpActor의 FlashJudgementColor와 동일한 색 체계
	switch (InJudgementType)
	{
	case EPTBJudgementType::HighPerfect: return FLinearColor(1.0f, 0.84f, 0.0f); // 금색
	case EPTBJudgementType::Perfect:     return FLinearColor(1.0f, 0.41f, 0.7f); // 분홍
	case EPTBJudgementType::Good:        return FLinearColor(0.4f, 0.86f, 0.4f); // 초록
	case EPTBJudgementType::Miss:        return FLinearColor(0.9f, 0.1f, 0.1f);  // 빨강
	default:                             return FLinearColor::White;
	}
}