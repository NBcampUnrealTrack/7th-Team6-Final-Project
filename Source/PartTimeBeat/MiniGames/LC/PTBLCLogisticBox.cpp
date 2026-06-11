#include "PTBLCLogisticBox.h"

#include "AkGeometryComponent.h"
#include "Debug/PTBTeamLog.h"

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

	if (!BaseMeshComponent)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[LC] LogisticBox BeginPlay failed: BaseMeshComponent is null."));
		return;
	}

	if (!bIsPackaged)
	{
		if (TriangleMeshAsset)
		{
			BaseMeshComponent->SetStaticMesh(TriangleMeshAsset);
		}
		else
		{
			PTB_WARNING(LogPTBMiniGames, TEXT("[LC] LogisticBox BeginPlay: TriangleMeshAsset is null on [%s]."),
			            *GetNameSafe(this));
		}
	}
}

void APTBLCLogisticBox::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsMoving)
	{
		float DistanceToMove = MovingSpeed * DeltaTime;

		FVector RightVector = GetActorRightVector();

		FVector NewOffset = RightVector * DistanceToMove;

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
		BoxState = EPTBLCLogisticBoxState::UnpackagedRed;
		break;
	case EPTBActionType::ActionB:
		BaseMeshComponent->SetMaterial(0, YellowBoxMaterialInstance);
		BoxState = EPTBLCLogisticBoxState::UnpackagedBlue;
		break;
	case EPTBActionType::ActionC:
		BaseMeshComponent->SetMaterial(0, BlueBoxMaterialInstance);
		BoxState = EPTBLCLogisticBoxState::UnpackagedBlue;
		break;
	default:
		break;
	}
}

void APTBLCLogisticBox::SetBoxStateByActionType(EPTBActionType ActionType)
{
	switch (ActionType)
	{
	case EPTBActionType::ActionA:
		BoxState = EPTBLCLogisticBoxState::RedBox;
		break;
	case EPTBActionType::ActionB:
		BoxState = EPTBLCLogisticBoxState::YellowBox;
		break;
	case EPTBActionType::ActionC:
		BoxState = EPTBLCLogisticBoxState::BlueBox;
		break;
	default:
		break;
	}
}

bool APTBLCLogisticBox::GetIsPackaged() const
{
	return bIsPackaged;
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
	bIsPackaged = true;
}
