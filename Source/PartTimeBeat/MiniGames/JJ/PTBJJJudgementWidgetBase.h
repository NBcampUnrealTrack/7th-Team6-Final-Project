// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"   // EPTBJudgementType
#include "PTBJJJudgementWidgetBase.generated.h"

/**
 * JumpJump 판정 텍스트(HighPerfect / Perfect / Good / Miss) 팝업 위젯의 C++ 베이스입니다.
 *
 * 하이브리드 패턴(= PTBJJCueWidgetBase와 동일):
 *  - C++은 "판정 데이터를 받아 BP로 전달"하는 뼈대만 담당.
 *  - 실제 텍스트/색/팝업 애니메이션은 WBP(BP)에서 OnJudgementShown()을 구현해 처리.
 *
 * 이렇게 두면 나중에 콤보 수치, 판정 누적 통계 등을 C++ 쪽에 얹어 확장하기 쉽습니다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBJJJudgementWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 판정 결과를 위젯에 전달한다.
	 * 판정 타입과 캐릭터 인덱스를 저장한 뒤, 표시용 라벨/색을 계산해
	 * OnJudgementShown()을 발행한다(→ BP에서 텍스트 세팅 + 팝업 애니메이션).
	 *
	 * @param InJudgementType 판정 타입(HighPerfect/Perfect/Good/Miss)
	 * @param InCharacterIndex 좌0 / 중1 / 우2 (머리 위 배치 등에 활용)
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JJ|Judgement")
	void ShowJudgement(EPTBJudgementType InJudgementType, int32 InCharacterIndex);

	/** 이 위젯이 마지막으로 표시한 판정 타입 */
	UFUNCTION(BlueprintPure, Category = "PTB|JJ|Judgement")
	EPTBJudgementType GetJudgementType() const { return JudgementType; }

	/** 이 위젯이 대응하는 캐릭터 인덱스(좌0/중1/우2) */
	UFUNCTION(BlueprintPure, Category = "PTB|JJ|Judgement")
	int32 GetCharacterIndex() const { return CharacterIndex; }

protected:
	/**
	 * ShowJudgement() 후 BP에서 실제 연출 처리.
	 * Label/Color를 그대로 TextBlock에 꽂고 팝업 애니메이션을 재생하면 된다.
	 *
	 * @param Label 표시 문구("PERFECT!!" 등)
	 * @param Color 판정 색(금/분홍/초록/빨강)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JJ|Judgement")
	void OnJudgementShown(EPTBJudgementType InJudgementType, const FText& Label, FLinearColor Color, int32 InCharacterIndex);

	/** 판정 타입 → 표시 문구. BP에서 오버라이드해 문구를 바꿀 수 있다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|JJ|Judgement")
	FText GetLabelForJudgement(EPTBJudgementType InJudgementType) const;
	virtual FText GetLabelForJudgement_Implementation(EPTBJudgementType InJudgementType) const;

	/** 판정 타입 → 색. BP에서 오버라이드해 색을 바꿀 수 있다. */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|JJ|Judgement")
	FLinearColor GetColorForJudgement(EPTBJudgementType InJudgementType) const;
	virtual FLinearColor GetColorForJudgement_Implementation(EPTBJudgementType InJudgementType) const;

	/** 마지막 판정 타입 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|JJ|Judgement")
	EPTBJudgementType JudgementType = EPTBJudgementType::Miss;

	/** 대응 캐릭터 인덱스 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|JJ|Judgement")
	int32 CharacterIndex = 0;
};