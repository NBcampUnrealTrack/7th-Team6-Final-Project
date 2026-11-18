#pragma once

#include "CoreMinimal.h"
#include "UI/PTBModalMenuWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBGameSelectPopupWidget.generated.h"

class UTextBlock;

/**
 * 미니게임 난이도 선택 팝업 베이스 (WBP_GameSelectPopUp의 부모).
 *
 * 열릴 때 난이도별 최고 점수를 "965,239 (96.52%)" 형식으로 자동 표시한다. 기록이 없으면 "-".
 *
 * Designer 위젯 (이름·타입 일치 시 자동 연결, 모두 선택):
 *  BestScore_Easy, BestScore_Standard, BestScore_Insane (Text Block)
 *
 * 미니게임 ID 찾는 순서:
 *  1) SetBestScoreMiniGameId()로 직접 지정한 값
 *  2) BP 변수 중 이름에 "MiniGame"이 들어간 Name 변수, 또는 MiniGameId를 가진 구조체 변수(세션 요청 등)
 *  → 대부분 BP 수정 없이 동작. 못 찾으면 Output Log에 [GameSelectPopup] 경고.
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBGameSelectPopupWidget : public UPTBModalMenuWidget
{
	GENERATED_BODY()

public:
	/** 최고 점수를 표시할 미니게임 ID를 직접 지정하고 바로 갱신 (자동으로 못 찾을 때만 사용) */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|GameSelect")
	void SetBestScoreMiniGameId(FName InMiniGameId);

	/** 난이도별 최고 점수 텍스트 다시 채우기 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|GameSelect")
	void RefreshBestScoreTexts();

	/** {Score} = 965,239 / {Percent} = 96.52% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|GameSelect")
	FText BestScoreFormat = NSLOCTEXT("PTBGameSelectPopup", "BestScoreFormat", "{Score} ({Percent})");

	/** 기록이 없을 때 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|GameSelect")
	FText NoRecordText = NSLOCTEXT("PTBGameSelectPopup", "NoRecord", "-");

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI|GameSelect") TObjectPtr<UTextBlock> BestScore_Easy;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI|GameSelect") TObjectPtr<UTextBlock> BestScore_Standard;
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI|GameSelect") TObjectPtr<UTextBlock> BestScore_Insane;

private:
	/** BP 변수에서 미니게임 ID 찾기 */
	FName FindMiniGameIdFromBlueprintVariables() const;
	FName ResolveMiniGameId() const;
	void SetBestScoreText(UTextBlock* TextBlock, FName InMiniGameId, EPTBDifficulty Difficulty) const;

	FName ExplicitMiniGameId = NAME_None;
	FName LastShownMiniGameId = NAME_None;
	bool bWarnedNoMiniGameId = false;
};
