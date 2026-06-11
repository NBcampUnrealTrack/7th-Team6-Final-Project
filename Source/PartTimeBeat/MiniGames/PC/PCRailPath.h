
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "PCRailPath.generated.h"

UCLASS()
class PARTTIMEBEAT_API APCRailPath : public AActor
{
	GENERATED_BODY()
	
public:	
	APCRailPath();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;
};
