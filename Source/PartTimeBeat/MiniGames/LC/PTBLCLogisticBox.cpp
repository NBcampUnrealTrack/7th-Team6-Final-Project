#include "PTBLCLogisticBox.h"

#include "AkGeometryComponent.h"
#include "Debug/PTBTeamLog.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
EPTBLCColorState ActionTypeToLCColorState(EPTBActionType ActionType)
{
	switch (ActionType)
	{
	case EPTBActionType::ActionA:
		return EPTBLCColorState::Red;
	case EPTBActionType::ActionB:
		return EPTBLCColorState::Yellow;
	case EPTBActionType::ActionC:
		return EPTBLCColorState::Blue;
	default:
		return EPTBLCColorState::None;
	}
}
}

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
	
	MovementDirection = GetActorRightVector();

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

		FVector NewOffset = MovementDirection * DistanceToMove;

		AddActorWorldOffset(NewOffset);
	}
	
	if (bIsPackagingSpinActive)
	{
		PackagingSpinElapsedSeconds += DeltaTime;
		
		constexpr float PackagingSpinDurationSeconds = 0.3f;
		constexpr float PackagingSpinTurns = 1.0f;
		const float Alpha = FMath::Clamp(PackagingSpinElapsedSeconds / PackagingSpinDurationSeconds, 0.0f, 1.0f);
		const float EasedAlpha = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.5f);
		const float YawOffset = 360.0f * PackagingSpinTurns * EasedAlpha;
		
		SetActorRotation(PackagingSpinStartRotation + FRotator(0.0f, YawOffset, 0.0f));
		
		if (Alpha >= 1.0f)
		{
			bIsPackagingSpinActive = false;
			SetActorRotation(PackagingSpinStartRotation);
		}
	}
}

int32 APTBLCLogisticBox::GetNoteId() const
{
	return NoteId;
}

float APTBLCLogisticBox::GetNoteTimeMs() const
{
	return NoteTimeMs;
}

EPTBLCColorState APTBLCLogisticBox::GetContentColorState() const
{
	return ContentColorState;
}

EPTBLCColorState APTBLCLogisticBox::GetBoxColorState() const
{
	return BoxColorState;
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

void APTBLCLogisticBox::InitializeFromNote(const FPTBNoteEvent& Note)
{
	if (!BaseMeshComponent || !RedBoxMaterialInstance || !BlueBoxMaterialInstance || !YellowBoxMaterialInstance)
	{
		return;
	}

	NoteId = Note.NoteId;
	NoteTimeMs = Note.TimeMs;
	ContentColorState = ActionTypeToLCColorState(Note.ActionType);
	BoxColorState = EPTBLCColorState::None;

	switch (ContentColorState)
	{
	case EPTBLCColorState::Red:
		BaseMeshComponent->SetMaterial(0, RedBoxMaterialInstance);
		break;
	case EPTBLCColorState::Yellow:
		BaseMeshComponent->SetMaterial(0, YellowBoxMaterialInstance);
		break;
	case EPTBLCColorState::Blue:
		BaseMeshComponent->SetMaterial(0, BlueBoxMaterialInstance);
		break;
	default:
		break;
	}
}

void APTBLCLogisticBox::PackageWithActionType(EPTBActionType ActionType)
{
	BoxColorState = ActionTypeToLCColorState(ActionType);
	ChangeMeshToBox();
	ApplyPackagedMaterial();
	StartPackagingSpin();
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

void APTBLCLogisticBox::SetMovementRotation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	MovementDirection = GetActorRightVector();
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

void APTBLCLogisticBox::ApplyPackagedMaterial()
{
	if (!BaseMeshComponent || !PackagedBoxMaterial)
	{
		return;
	}

	UMaterialInstanceDynamic* DynamicMaterial = BaseMeshComponent->CreateDynamicMaterialInstance(0, PackagedBoxMaterial);
	if (!DynamicMaterial)
	{
		return;
	}

	FLinearColor BoxColor = FLinearColor::White;
	switch (BoxColorState)
	{
	case EPTBLCColorState::Red:
		BoxColor = RedColor;
		break;
	case EPTBLCColorState::Yellow:
		BoxColor = YellowColor;
		break;
	case EPTBLCColorState::Blue:
		BoxColor = BlueColor;
		break;
	default:
		break;
	}

	FLinearColor MarkColor = FLinearColor::White;
	switch (ContentColorState)
	{
	case EPTBLCColorState::Red:
		MarkColor = RedColor;
		break;
	case EPTBLCColorState::Yellow:
		MarkColor = YellowColor;
		break;
	case EPTBLCColorState::Blue:
		MarkColor = BlueColor;
		break;
	default:
		break;
	}

	DynamicMaterial->SetVectorParameterValue(BoxColorParameterName, BoxColor);
	DynamicMaterial->SetVectorParameterValue(MarkColorParameterName, MarkColor);
}

void APTBLCLogisticBox::StartPackagingSpin()
{
	bIsPackagingSpinActive = true;
	PackagingSpinElapsedSeconds = 0.0f;
	PackagingSpinStartRotation = GetActorRotation();
}
