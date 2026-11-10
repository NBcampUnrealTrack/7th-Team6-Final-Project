#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBDWKeyGuideWidget.generated.h"

class APTBDWMiniGame;
class UPTBDWKeySlotWidget;

/**
 * DW 키 안내 HUD.
 * Designer에 UPTBDWKeySlotWidget(을 부모로 한 WBP)을 원하는 만큼 배치하고 칸마다 Action을 지정하면,
 *  - 이번 판에서 안 쓰는 키의 칸은 자동으로 숨김
 *  - 별 색 = DataAsset NoteColorByAction, 키 이름 = 설정한 키
 *  - 키를 누르는 동안 해당 칸 강조
 *  - 입력을 받는 동안(본게임·연습)만 표시, 대사/튜토리얼 설명/결과 화면에서는 숨김
 * DW 미니게임 BP의 KeyGuideWidgetClass에 지정하면 자동 생성된다.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBDWKeyGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitializeWithMiniGame(APTBDWMiniGame* InMiniGame);

	/** 키 이름·색 다시 적용 (키 설정 변경 후 등) */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW|KeyGuide")
	void RefreshSlots();

	/** 입력을 받지 않을 때도 항상 보이게 (디버그·연출용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|KeyGuide")
	bool bAlwaysVisible = false;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "PTB|DW|KeyGuide")
	APTBDWMiniGame* GetDWMiniGame() const { return DWMiniGame.Get(); }

	/** 표시/숨김이 바뀔 때 (BP: 페이드 애니메이션 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|DW|KeyGuide")
	void OnGuideShownChanged(bool bShown);

private:
	UFUNCTION()
	void HandleRhythmKeyStateChanged(EPTBActionType Action, bool bPressed);

	void CollectSlots();
	void SetGuideShown(bool bShown);

	TWeakObjectPtr<APTBDWMiniGame> DWMiniGame;

	UPROPERTY()
	TArray<TObjectPtr<UPTBDWKeySlotWidget>> KeySlots;

	bool bGuideShown = true;
	bool bSlotsDirty = true;
};
