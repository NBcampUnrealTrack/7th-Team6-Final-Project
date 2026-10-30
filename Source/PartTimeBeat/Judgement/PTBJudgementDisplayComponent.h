#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/PTBStructEnums.h"
#include "Judgement/PTBJudgeSkin.h"
#include "PTBJudgementDisplayComponent.generated.h"

class APTBBaseMiniGame;
class UWidget;
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

	/**
	 * 화면에 떠 있는 위젯(노트 큐 등)의 현재 중심 위치로 판정 위치 생성.
	 * 위젯이 없거나 아직 화면에 그려지지 않았으면 Type=Default
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Judgement")
	static FPTBJudgementAnchor MakeJudgementAnchorFromWidget(UWidget* Widget, int32 SlotIndex = 0);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UFUNCTION()
	void HandleMiniGameJudgement(FPTBJudgementResult Result);

	/** 라운드 시작마다 장착 스킨 다시 읽기 (미니게임 액터가 레벨에 계속 남아 있는 경우 대비) */
	UFUNCTION()
	void HandleMiniGameStarted();

	/** 인벤토리·상점에서 판정 스킨을 바꾸면 다음 판정부터 반영 */
	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	class UPTBItemSubsystem* GetItemSubsystem() const;

	UPTBJudgementDisplayWidget* GetOrCreateWidget(int32 SlotIndex);
	APTBBaseMiniGame* GetMiniGame() const;

	UPROPERTY(Transient)
	TObjectPtr<UPTBJudgeSkin> ActiveSkin;

	UPROPERTY(Transient)
	TMap<int32, TObjectPtr<UPTBJudgementDisplayWidget>> Widgets;

	bool bSkinResolved = false;

	/** 장착 스킨 아이템 행의 NumericParams "Scale" (DT_PTBItems에서 스킨별 크기 조절) */
	float ItemScale = 1.f;
};
