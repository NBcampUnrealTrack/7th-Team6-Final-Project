
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Rhythm/PTBJudgementSystem.h"
#include "PTBPCJudgementPopupWidget.generated.h"

UCLASS()
class PARTTIMEBEAT_API UPTBPCJudgementPopupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Judgement")
        void ShowJudgement(EPTBJudgementType Result);
};
