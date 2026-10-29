#include "Items/PTBVehicleComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Items/PTBItemSubsystem.h"
#include "TimerManager.h"

namespace PTBVehicleInternal
{
	UPTBItemSubsystem* GetItems(const UObject* Context)
	{
		const UWorld* World = Context ? Context->GetWorld() : nullptr;
		const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
		return GI ? GI->GetSubsystem<UPTBItemSubsystem>() : nullptr;
	}
}

UPTBVehicleComponent::UPTBVehicleComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false; // 부스터 탈것일 때만 Tick
}

void UPTBVehicleComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UPTBItemSubsystem* Items = PTBVehicleInternal::GetItems(this))
	{
		Items->OnItemStateChanged.AddUniqueDynamic(this, &UPTBVehicleComponent::HandleItemStateChanged);
	}

	// BP BeginPlay에서 이동값을 조정하는 경우를 위해 다음 프레임에 원래값 저장 후 적용
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		CacheBaseMovement();
		ReapplyVehicle();
	}));
}

void UPTBVehicleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPTBItemSubsystem* Items = PTBVehicleInternal::GetItems(this))
	{
		Items->OnItemStateChanged.RemoveDynamic(this, &UPTBVehicleComponent::HandleItemStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

ACharacter* UPTBVehicleComponent::GetCharacter() const
{
	return Cast<ACharacter>(GetOwner());
}

void UPTBVehicleComponent::HandleItemStateChanged(FName Category)
{
	if (Category.IsNone() || Category == PTBItemCategory::Vehicle)
	{
		ReapplyVehicle();
	}
}

void UPTBVehicleComponent::CacheBaseMovement()
{
	const ACharacter* Character = GetCharacter();
	const UCharacterMovementComponent* Move = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Move)
	{
		PTB_WARNING(LogPTBCharacters, TEXT("[Vehicle] %s: Character/CharacterMovement가 아님 (캐릭터 BP에 추가해야 함)"),
			*GetNameSafe(GetOwner()));
		return;
	}

	BaseMaxWalkSpeed    = Move->MaxWalkSpeed;
	BaseJumpZVelocity   = Move->JumpZVelocity;
	BaseGravityScale    = Move->GravityScale;
	BaseAirControl      = Move->AirControl;
	BaseJumpMaxHoldTime = Character->JumpMaxHoldTime;
	BaseJumpMaxCount    = Character->JumpMaxCount;
	bBaseCached = true;
}

void UPTBVehicleComponent::RestoreBaseMovement()
{
	ACharacter* Character = GetCharacter();
	UCharacterMovementComponent* Move = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Move || !bBaseCached)
	{
		return;
	}

	Move->MaxWalkSpeed  = BaseMaxWalkSpeed;
	Move->JumpZVelocity = BaseJumpZVelocity;
	Move->GravityScale  = BaseGravityScale;
	Move->AirControl    = BaseAirControl;
	Character->JumpMaxHoldTime = BaseJumpMaxHoldTime;
	Character->JumpMaxCount    = BaseJumpMaxCount;
}

void UPTBVehicleComponent::ReapplyVehicle()
{
	if (!bBaseCached)
	{
		return; // BeginPlay 다음 프레임에 다시 호출됨
	}

	const UPTBItemSubsystem* Items = PTBVehicleInternal::GetItems(this);
	const FName VehicleId = Items ? Items->GetEquippedItemId(PTBItemCategory::Vehicle) : NAME_None;

	bool bFound = false;
	const FPTBItemRow Row = Items ? Items->GetItemData(VehicleId, bFound) : FPTBItemRow();

	CurrentVehicleId = VehicleId;
	RestoreBaseMovement();
	SetBoosting(false);

	const bool bIsNone = !bFound || Row.bIsDefault;
	if (bIsNone)
	{
		CurrentParams.Reset();
		SpeedMultiplier = 1.f;
		BoostMultiplier = 1.f;
		SpinSpeed = 0.f;
		UpdateVehicleMesh(nullptr);
		SetCharacterHidden(false);
	}
	else
	{
		CurrentParams = Row.NumericParams;
		ApplyVehicleRow(Row);
		SpinSpeed = GetParam(Row, TEXT("SpinSpeed"), 0.f);
		UpdateVehicleMesh(&Row);
		SetCharacterHidden(GetParam(Row, TEXT("HideCharacter"), 0.f) > 0.f && VehicleMesh && VehicleMesh->IsVisible());
	}

	// 부스터 키 확인 또는 프로펠러 회전이 필요할 때만 Tick
	SetComponentTickEnabled(BoostMultiplier > 1.f || !FMath::IsNearlyZero(SpinSpeed));

	PTB_RECORD(LogPTBCharacters, TEXT("[Vehicle] Apply: item=%s none=%d speed=x%.2f boost=x%.2f"),
		*VehicleId.ToString(), bIsNone ? 1 : 0, SpeedMultiplier, BoostMultiplier);

	OnVehicleChanged(VehicleId, Row, bIsNone);
}

float UPTBVehicleComponent::GetParam(const FPTBItemRow& Row, const TCHAR* Key, float Default) const
{
	const float* Found = Row.NumericParams.Find(FName(Key));
	return Found ? *Found : Default;
}

float UPTBVehicleComponent::GetVehicleParam(FName Key, float Default) const
{
	const float* Found = CurrentParams.Find(Key);
	return Found ? *Found : Default;
}

void UPTBVehicleComponent::ApplyVehicleRow(const FPTBItemRow& Row)
{
	ACharacter* Character = GetCharacter();
	UCharacterMovementComponent* Move = Character ? Character->GetCharacterMovement() : nullptr;
	if (!Move)
	{
		return;
	}

	SpeedMultiplier = FMath::Max(0.1f, GetParam(Row, TEXT("SpeedMultiplier"), 1.f));
	BoostMultiplier = FMath::Max(1.f, GetParam(Row, TEXT("BoostSpeedMultiplier"), 1.f));

	Move->MaxWalkSpeed  = BaseMaxWalkSpeed * SpeedMultiplier;
	Move->JumpZVelocity = BaseJumpZVelocity * GetParam(Row, TEXT("JumpZMultiplier"), 1.f);
	Move->GravityScale  = GetParam(Row, TEXT("GravityScale"), BaseGravityScale);
	Move->AirControl    = GetParam(Row, TEXT("AirControl"), BaseAirControl);

	// 이단 점프: 엔진 기본 기능(JumpMaxCount) 사용
	const int32 ExtraJumps = FMath::Max(0, FMath::RoundToInt(GetParam(Row, TEXT("ExtraJumpCount"), 0.f)));
	Character->JumpMaxCount = BaseJumpMaxCount + ExtraJumps;

	// 헬리콥터식 상승: 점프 키를 누르고 있는 동안 JumpZVelocity로 계속 상승 (엔진 기본 기능 JumpMaxHoldTime)
	Character->JumpMaxHoldTime = FMath::Max(BaseJumpMaxHoldTime, GetParam(Row, TEXT("JumpHoldTime"), 0.f));

	// 점프 불가 (자동차 등)
	if (GetParam(Row, TEXT("DisableJump"), 0.f) > 0.f)
	{
		Character->JumpMaxCount = 0;
	}
}

void UPTBVehicleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 프로펠러 등 회전
	if (VehicleMesh && VehicleMesh->IsVisible() && !FMath::IsNearlyZero(SpinSpeed))
	{
		VehicleMesh->AddLocalRotation(FRotator(0.f, SpinSpeed * DeltaTime, 0.f));
	}

	if (BoostMultiplier <= 1.f)
	{
		return;
	}

	const ACharacter* Character = GetCharacter();
	const APlayerController* PC = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}

	bool bKeyDown = false;
	for (const FKey& Key : BoostKeys)
	{
		if (PC->IsInputKeyDown(Key))
		{
			bKeyDown = true;
			break;
		}
	}
	SetBoosting(bKeyDown);
}

void UPTBVehicleComponent::SetBoosting(bool bInBoosting)
{
	if (bBoosting == bInBoosting)
	{
		return;
	}
	bBoosting = bInBoosting;

	if (UCharacterMovementComponent* Move = GetCharacter() ? GetCharacter()->GetCharacterMovement() : nullptr)
	{
		Move->MaxWalkSpeed = BaseMaxWalkSpeed * SpeedMultiplier * (bBoosting ? BoostMultiplier : 1.f);
	}
	OnBoostChanged(bBoosting);
}

void UPTBVehicleComponent::UpdateVehicleMesh(const FPTBItemRow* Row)
{
	UStaticMesh* Mesh = (Row && !Row->Asset.IsNull()) ? Cast<UStaticMesh>(Row->Asset.LoadSynchronous()) : nullptr;

	if (!Mesh)
	{
		if (VehicleMesh)
		{
			VehicleMesh->SetVisibility(false);
			VehicleMesh->SetStaticMesh(nullptr);
		}
		if (Row && !Row->Asset.IsNull())
		{
			PTB_WARNING(LogPTBCharacters, TEXT("[Vehicle] Asset이 Static Mesh가 아님: %s"), *Row->Asset.ToString());
		}
		return;
	}

	ACharacter* Character = GetCharacter();
	if (!Character)
	{
		return;
	}

	if (!VehicleMesh)
	{
		VehicleMesh = NewObject<UStaticMeshComponent>(Character, TEXT("PTBVehicleMesh"));
		VehicleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		VehicleMesh->SetCastShadow(true);
		VehicleMesh->RegisterComponent();
	}

	// 붙일 위치: 머리(AttachToHead) 또는 발밑(캡슐 바닥)
	const bool bAttachToHead = GetParam(*Row, TEXT("AttachToHead"), 0.f) > 0.f;
	USkeletalMeshComponent* CharMesh = Character->GetMesh();
	USceneComponent* Parent = Character->GetRootComponent();
	FName Socket = NAME_None;
	FVector BaseOffset = FVector::ZeroVector;

	if (bAttachToHead && CharMesh)
	{
		Parent = CharMesh;
		if (CharMesh->DoesSocketExist(HeadSocketName))
		{
			Socket = HeadSocketName;
		}
		else
		{
			PTB_WARNING(LogPTBCharacters, TEXT("[Vehicle] 머리 소켓/뼈 '%s' 없음 → 캐릭터 메시 원점에 붙임 (HeadSocketName 확인)"),
				*HeadSocketName.ToString());
		}
	}
	else if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
	{
		BaseOffset.Z = -Capsule->GetScaledCapsuleHalfHeight(); // 캡슐 중심 → 바닥
	}

	VehicleMesh->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);

	const FVector Offset = BaseOffset + FVector(
		GetParam(*Row, TEXT("MeshOffsetX"), 0.f),
		GetParam(*Row, TEXT("MeshOffsetY"), 0.f),
		GetParam(*Row, TEXT("MeshOffsetZ"), 0.f));
	VehicleMesh->SetRelativeLocationAndRotation(Offset, FRotator(0.f, GetParam(*Row, TEXT("MeshYaw"), 0.f), 0.f));
	VehicleMesh->SetRelativeScale3D(FVector(GetParam(*Row, TEXT("MeshScale"), 1.f)));

	VehicleMesh->SetStaticMesh(Mesh);
	VehicleMesh->SetVisibility(true);
}

void UPTBVehicleComponent::SetCharacterHidden(bool bHidden)
{
	bCharacterHidden = bHidden;

	// 캐릭터 메시만 숨김 (자식으로 붙은 탈것 메시는 영향 없음: bPropagateToChildren=false)
	if (ACharacter* Character = GetCharacter())
	{
		if (USkeletalMeshComponent* CharMesh = Character->GetMesh())
		{
			CharMesh->SetVisibility(!bHidden, false);
		}
	}
}
