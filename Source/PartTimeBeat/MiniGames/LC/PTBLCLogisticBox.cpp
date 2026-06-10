#include "PTBLCLogisticBox.h"

#include "AkGeometryComponent.h"

APTBLCLogisticBox::APTBLCLogisticBox()
{
	PrimaryActorTick.bCanEverTick = true;

	BaseMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMeshComponent"));

	if (BaseMeshComponent)
	{
		RootComponent = BaseMeshComponent;

		BaseMeshComponent->SetSimulatePhysics(false);
	}
}

void APTBLCLogisticBox::BeginPlay()
{
	Super::BeginPlay();
}

void APTBLCLogisticBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		float DistanceToMove = MovingSpeed * DeltaTime;

		FVector ForwardVector = GetActorForwardVector();

		FVector NewOffset = ForwardVector * DistanceToMove;

		AddActorWorldOffset(NewOffset);
	}
}

EPTBLCLogisticBoxState APTBLCLogisticBox::GetLogisticBoxState() const
{
	return BoxState;
}

void APTBLCLogisticBox::SetLogisticBoxState(EPTBLCLogisticBoxState NewState)
{
	BoxState = NewState;
}

UStaticMeshComponent* APTBLCLogisticBox::GetStaticMeshComponent() const
{
	return BaseMeshComponent;
}

UMaterialInstance* APTBLCLogisticBox::GetRedMaterialInstance() const
{
	return RedBoxMaterialInstance;
}

UMaterialInstance* APTBLCLogisticBox::GetBlueMaterialInstance() const
{
	return BlueBoxMaterialInstance;
}

UMaterialInstance* APTBLCLogisticBox::GetYellowMaterialInstance() const
{
	return YellowBoxMaterialInstance;
}

void APTBLCLogisticBox::SetBoxMaterlalInstanceByActionType(EPTBActionType ActionType)
{
	if (!BaseMeshComponent || !RedBoxMaterialInstance || !BlueBoxMaterialInstance || !YellowBoxMaterialInstance)
	{
		return;
	}
	
	switch (ActionType)
	{
	case EPTBActionType::ActionA:
		BaseMeshComponent->SetMaterial(0, RedBoxMaterialInstance);
		break;
	case EPTBActionType::ActionB:
		BaseMeshComponent->SetMaterial(0, BlueBoxMaterialInstance);
		break;
	case EPTBActionType::ActionC:
		BaseMeshComponent->SetMaterial(0, YellowBoxMaterialInstance);
		break;
	default:
		break;
	}
}

void APTBLCLogisticBox::StopMovingAndEnablePhysics()
{
	if (!BaseMeshComponent)
	{
		return;
	}
	
	bIsMoving = false;
	BaseMeshComponent->SetSimulatePhysics(true);
}

void APTBLCLogisticBox::ChangeMeshToBox()
{
	if (!BaseMeshComponent || !BoxMeshAsset)
	{
		return;
	}
	
	BaseMeshComponent->SetStaticMesh(BoxMeshAsset);
}
