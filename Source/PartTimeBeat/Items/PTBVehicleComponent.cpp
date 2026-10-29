#include "Items/PTBVehicleComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
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
		SpeedMultiplier = 1.f;
		BoostMultiplier = 1.f;
		UpdateVehicleMesh(nullptr);
	}
	else
	{
		ApplyVehicleRow(Row);
		UpdateVehicleMesh(&Row);
	}

	// 부스터가 있을 때만 키 입력 확인
	SetComponentTickEnabled(BoostMultiplier > 1.f);

	PTB_RECORD(LogPTBCharacters, TEXT("[Vehicle] Apply: item=%s none=%d speed=x%.2f boost=x%.2f"),
		*VehicleId.ToString(), bIsNone ? 1 : 0, SpeedMultiplier, BoostMultiplier);

	OnVehicleChanged(VehicleId, Row, bIsNone);
}

float UPTBVehicleComponent::GetParam(const FPTBItemRow& Row, const TCHAR* Key, float Default) const
{
	const float* Found = Row.NumericParams.Find(FName(Key));
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
}

void UPTBVehicleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

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
		VehicleMesh->RegisterComponent();

		USkeletalMeshComponent* CharMesh = Character->GetMesh();
		const bool bHasSocket = CharMesh && !AttachSocketName.IsNone() && CharMesh->DoesSocketExist(AttachSocketName);
		VehicleMesh->AttachToComponent(CharMesh ? static_cast<USceneComponent*>(CharMesh) : Character->GetRootComponent(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, bHasSocket ? AttachSocketName : NAME_None);
	}

	VehicleMesh->SetStaticMesh(Mesh);
	VehicleMesh->SetVisibility(true);
}
