#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBDWKeySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UWidget;

/**
 * DW 키 안내 칸 1개: 별 아이콘(노트 색) + 키 이름 + 눌림 표시.
 * HUD(UPTBDWKeyGuideWidget)에 배치하고, Details에서 Action만 지정하면 나머지는 자동.
 *
 * Designer에 같은 이름으로 두면 자동 연결 (전부 선택사항):
 *  - IMG_Star          : 흰색 별 이미지 → 노트 색으로 칠해짐
 *  - TXT_Key           : 키 이름 (설정한 키로 자동)
 *  - IMG_PressHighlight: 누르는 동안만 보이는 테두리/빛
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBDWKeySlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이 칸이 담당하는 키 (HUD Designer에서 칸마다 지정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|KeyGuide")
	EPTBActionType Action = EPTBActionType::ActionA;

	/** 색·키 이름 적용 (HUD가 호출) */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW|KeyGuide")
	void SetupSlot(const FLinearColor& InNoteColor, const FText& InKeyLabel);

	/** 눌림 표시 */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW|KeyGuide")
	void SetPressed(bool bInPressed);

	UFUNCTION(BlueprintPure, Category = "PTB|DW|KeyGuide")
	bool IsPressed() const { return bIsPressed; }

	UFUNCTION(BlueprintPure, Category = "PTB|DW|KeyGuide")
	FLinearColor GetNoteColor() const { return NoteColor; }

	/** 누르는 동안 크기 (1.0 = 변화 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|KeyGuide", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float PressedScale = 1.15f;

	/** 누르는 동안 별 밝기 배율 (1.0 = 변화 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|KeyGuide", meta = (ClampMin = "1.0", ClampMax = "4.0"))
	float PressedBrightness = 1.5f;

	/** 누르지 않을 때 별 투명도 (누르면 1.0) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|KeyGuide", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float IdleStarOpacity = 0.85f;

	/** 테두리(IMG_PressHighlight)도 노트 색으로 칠할지 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|DW|KeyGuide")
	bool bTintHighlightWithNoteColor = true;

protected:
	virtual void NativeConstruct() override;

	/** 기본 처리 후 호출 (BP 추가 연출: 사운드, 애니메이션 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|DW|KeyGuide")
	void OnPressedChanged(bool bPressed);

	/** 색·키 이름이 적용된 뒤 호출 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|DW|KeyGuide")
	void OnSlotSetup(FLinearColor InNoteColor, const FText& InKeyLabel);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> IMG_Star;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TXT_Key;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UWidget> IMG_PressHighlight;

private:
	void ApplyVisual();

	FLinearColor NoteColor = FLinearColor::White;
	bool bIsPressed = false;
};
