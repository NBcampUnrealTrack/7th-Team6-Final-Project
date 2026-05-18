// Fill out your copyright notice in the Description page of Project Settings.


#include "Rhythm/PTBRhythmConductorComponent.h"

// Sets default values for this component's properties
UPTBRhythmConductorComponent::UPTBRhythmConductorComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPTBRhythmConductorComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPTBRhythmConductorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

