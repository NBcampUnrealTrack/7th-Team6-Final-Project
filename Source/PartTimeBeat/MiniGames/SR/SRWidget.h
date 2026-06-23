#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "SRWidget.generated.h"

UCLASS()
class PARTTIMEBEAT_API USRWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget)) class UImage* Img_NextTopping;
	UPROPERTY(meta = (BindWidget)) class UTextBlock* Txt_SuccessCount;

	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdatePreviewImage(UTexture2D* NewTexture);
	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdateSuccessCount(int32 NewCount);
};
