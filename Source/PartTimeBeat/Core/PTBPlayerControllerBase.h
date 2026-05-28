#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PTBPlayerControllerBase.generated.h"

UCLASS()
class PARTTIMEBEAT_API APTBPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
};