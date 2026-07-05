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
	BoxMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMeshComponent"));

	if (BaseMeshComponent)
	{
		RootComponent = BaseMeshComponent;

		BaseMeshComponent->SetSimulatePhysics(false);
	}

	if (BoxMeshComponent)
	{
		BoxMeshComponent->SetupAttachment(BaseMeshComponent);
		BoxMeshComponent->SetSimulatePhysics(false);
		BoxMeshComponent->SetVisibility(false);
		BoxMeshComponent->SetHiddenInGame(true);
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
	BaseMeshRelativeScale = BaseMeshComponent->GetRelativeScale3D();
	if (BoxMeshComponent)
	{
		BoxMeshRelativeScale = BoxMeshComponent->GetRelativeScale3D();
		BoxMeshComponent->SetStaticMesh(BoxMeshAsset);
		if (PackagedBoxMaterial)
		{
			PackagedBoxDynamicMaterial = BoxMeshComponent->CreateDynamicMaterialInstance(0, PackagedBoxMaterial);
		}
		BoxMeshComponent->SetVisibility(bIsPackaged);
		BoxMeshComponent->SetHiddenInGame(!bIsPackaged);
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

	BaseMeshComponent->SetVisibility(!bIsPackaged);
	BaseMeshComponent->SetHiddenInGame(bIsPackaged);
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
	NoteId = Note.NoteId;
	NoteTimeMs = Note.TimeMs;
	ContentColorState = ActionTypeToLCColorState(Note.ActionType);
	BoxColorState = EPTBLCColorState::None;

	if (!BaseMeshComponent)
	{
		return;
	}

	switch (ContentColorState)
	{
	case EPTBLCColorState::Red:
		if (RedBoxMaterialInstance)
		{
			BaseMeshComponent->SetMaterial(0, RedBoxMaterialInstance);
		}
		break;
	case EPTBLCColorState::Yellow:
		if (YellowBoxMaterialInstance)
		{
			BaseMeshComponent->SetMaterial(0, YellowBoxMaterialInstance);
		}
		break;
	case EPTBLCColorState::Blue:
		if (BlueBoxMaterialInstance)
		{
			BaseMeshComponent->SetMaterial(0, BlueBoxMaterialInstance);
		}
		break;
	default:
		break;
	}
}

void APTBLCLogisticBox::ActivateFromPool(const FPTBNoteEvent& Note, const FVector& SpawnLocation)
{
	ResetForPool();
	SetActorLocation(SpawnLocation);
	SetActorRotation(FRotator::ZeroRotator);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	bIsMoving = true;
	MovementDirection = GetActorRightVector();

	InitializeFromNote(Note);
}

void APTBLCLogisticBox::ResetForPool()
{
	NoteId = 0;
	NoteTimeMs = 0.0f;
	ContentColorState = EPTBLCColorState::None;
	BoxColorState = EPTBLCColorState::None;
	bIsMoving = false;
	bIsPackaged = false;
	bIsPackagingSpinActive = false;
	PackagingSpinElapsedSeconds = 0.0f;
	PackagingSpinStartRotation = FRotator::ZeroRotator;
	MovementDirection = FVector::RightVector;

	if (BaseMeshComponent)
	{
		BaseMeshComponent->SetSimulatePhysics(false);
		BaseMeshComponent->SetRelativeScale3D(BaseMeshRelativeScale);
		if (TriangleMeshAsset && BaseMeshComponent->GetStaticMesh() != TriangleMeshAsset)
		{
			BaseMeshComponent->SetStaticMesh(TriangleMeshAsset);
		}
		BaseMeshComponent->SetVisibility(true);
		BaseMeshComponent->SetHiddenInGame(false);
	}

	if (BoxMeshComponent)
	{
		BoxMeshComponent->SetSimulatePhysics(false);
		BoxMeshComponent->SetRelativeScale3D(BoxMeshRelativeScale);
		if (BoxMeshAsset && BoxMeshComponent->GetStaticMesh() != BoxMeshAsset)
		{
			BoxMeshComponent->SetStaticMesh(BoxMeshAsset);
		}
		BoxMeshComponent->SetVisibility(false);
		BoxMeshComponent->SetHiddenInGame(true);
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	SetActorTickEnabled(false);
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

float APTBLCLogisticBox::GetMovingSpeed() const
{
	return MovingSpeed;
}

void APTBLCLogisticBox::SetBeatPulseScale(float NewScale)
{
	if (!BaseMeshComponent || !BoxMeshComponent)
	{
		return;
	}

	const float AppliedScale = ShouldApplyBeatPulse() ? FMath::Max(0.0f, NewScale) : 1.0f;
	BaseMeshComponent->SetRelativeScale3D(BaseMeshRelativeScale * AppliedScale);
	BoxMeshComponent->SetRelativeScale3D(BoxMeshRelativeScale * AppliedScale);
}

void APTBLCLogisticBox::StopMovingAndEnablePhysics()
{
	UStaticMeshComponent* ActiveMeshComponent = bIsPackaged ? BoxMeshComponent.Get() : BaseMeshComponent.Get();
	if (!ActiveMeshComponent)
	{
		return;
	}

	bIsMoving = false;
	ActiveMeshComponent->SetSimulatePhysics(true);
}

void APTBLCLogisticBox::SetMovementRotation(FRotator NewRotation)
{
	SetActorRotation(NewRotation);
	MovementDirection = GetActorRightVector();
}

void APTBLCLogisticBox::ChangeMeshToBox()
{
	if (!BaseMeshComponent || !BoxMeshComponent || !BoxMeshAsset)
	{
		PTB_WARNING(LogPTBMiniGames, TEXT("[LC] ChangeMeshToBox skipped: invalid mesh setup. Actor=%s BaseMesh=%s BoxMesh=%s BoxMeshAsset=%s"),
			*GetNameSafe(this),
			*GetNameSafe(BaseMeshComponent.Get()),
			*GetNameSafe(BoxMeshComponent.Get()),
			*GetNameSafe(BoxMeshAsset.Get()));
		return;
	}

	if (BoxMeshComponent->GetStaticMesh() != BoxMeshAsset)
	{
		BoxMeshComponent->SetStaticMesh(BoxMeshAsset);
	}

	bIsPackaged = true;
	BaseMeshComponent->SetVisibility(false);
	BaseMeshComponent->SetHiddenInGame(true);
	BoxMeshComponent->SetVisibility(true);
	BoxMeshComponent->SetHiddenInGame(false);
}

void APTBLCLogisticBox::ApplyPackagedMaterial()
{
	if (!PackagedBoxDynamicMaterial)
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

	PackagedBoxDynamicMaterial->SetVectorParameterValue(BoxColorParameterName, BoxColor);
	PackagedBoxDynamicMaterial->SetVectorParameterValue(MarkColorParameterName, MarkColor);
}

void APTBLCLogisticBox::StartPackagingSpin()
{
	bIsPackagingSpinActive = true;
	PackagingSpinElapsedSeconds = 0.0f;
	PackagingSpinStartRotation = GetActorRotation();
}

bool APTBLCLogisticBox::ShouldApplyBeatPulse() const
{
	if (!bIsPackaged)
	{
		return ContentColorState != EPTBLCColorState::None;
	}

	return ContentColorState != EPTBLCColorState::None && ContentColorState == BoxColorState;
}
