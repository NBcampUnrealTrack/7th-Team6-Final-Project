#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "SRWidget.generated.h"

UCLASS()
class PARTTIMEBEAT_API USRWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget)) class UImage* Img_NextTopping;

	UFUNCTION(BlueprintCallable, Category = "SushiUI") void UpdatePreviewImage(UTexture2D* NewTexture);
	
};
