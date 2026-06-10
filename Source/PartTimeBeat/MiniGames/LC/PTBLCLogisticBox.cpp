#include "PTBLCLogisticBox.h"


// Sets default values
APTBLCLogisticBox::APTBLCLogisticBox()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void APTBLCLogisticBox::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APTBLCLogisticBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

