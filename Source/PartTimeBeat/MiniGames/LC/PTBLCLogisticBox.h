#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBLCLogisticBox.generated.h"

UCLASS()
class PARTTIMEBEAT_API APTBLCLogisticBox : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APTBLCLogisticBox();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
