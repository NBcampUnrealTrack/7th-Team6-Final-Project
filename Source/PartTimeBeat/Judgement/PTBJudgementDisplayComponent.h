#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgementDisplayComponent.generated.h"

class APTBBaseMiniGame;
class UPTBJudgeSkin;
class UPTBJudgementDisplayWidget;

/**
 * 공통 판정 표시.
 * PTBBaseMiniGame이 BeginPlay에서 자동으로 붙임 → OnMiniGameJudgement를 받아 장착한 판정 스킨으로 이미지 표시.
 *
 * 동작 조건: 미니게임 RuleSet의 bUseCommonJudgementDisplay = true
 * 위치
 *  - 기본: RuleSet의 JudgementScreenPosition (화면 비율)
 *  - 미니게임이 GetJudgementDisplayAnchor를 재정의해 true를 반환하면: 그 월드 위치 + JudgementWorldOffset
 *    (OutSlotIndex가 다르면 별도 위젯 → 캐릭터 2명이 동시에 판정을 받아도 각자 표시)
 */
UCLASS(ClassGroup = (PTB))
class PARTTIMEBEAT_API UPTBJudgementDisplayComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBJudgementDisplayComponent();

	/** 현재 적용 중인 판정 스킨 (장착 스킨, 없으면 기본 스킨) */
	UFUNCTION(BlueprintPure, Category = "PTB|Judgement")
	UPTBJudgeSkin* GetActiveSkin() const { return ActiveSkin; }

	/** 장착 스킨 다시 읽기 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Judgement")
	void RefreshSkin();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleMiniGameJudgement(FPTBJudgementResult Result);

	UPTBJudgementDisplayWidget* GetOrCreateWidget(int32 SlotIndex);
	APTBBaseMiniGame* GetMiniGame() const;

	UPROPERTY(Transient)
	TObjectPtr<UPTBJudgeSkin> ActiveSkin;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UPTBJudgementDisplayWidget>> Widgets;

	bool bSkinResolved = false;
};
