#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Core/PTBStructEnums.h"
#include "SRWidget.generated.h"

UCLASS()
class PARTTIMEBEAT_API USRWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget)) class UImage* Img_NextTopping;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* Txt_SuccessCount;

	/** WBP_SR_Preview에 이 이름의 TextBlock을 추가해주세요 (판정 종류 + 빠름/느림 ms 표시용) */
	UPROPERTY(meta = (BindWidget)) class UTextBlock* Txt_Judgement;

	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdatePreviewImage(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdateSuccessCount(int32 NewCount);

	/** 판정 결과(HighPerfect/Perfect/Good/Miss)와, Good/Miss일 때는 몇 ms 빠르거나 늦었는지도 같이 표시합니다.
	 *  DeltaMs는 음수면 빠르게 침(Early), 양수면 늦게 침(Late)을 의미합니다. */
	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdateJudgementText(EPTBJudgementType JudgementType, float DeltaMs, bool bHasTimingInfo);
};