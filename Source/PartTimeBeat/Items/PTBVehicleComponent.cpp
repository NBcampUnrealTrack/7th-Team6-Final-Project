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
#include "Engine/LocalPlayer.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedPlayerInput.h"
#include "InputAction.h"

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

	// 비행 속도 보정이 이동 계산보다 먼저 적용되도록 CharacterMovement가 이 컴포넌트 Tick 뒤에 돌게 함
	const_cast<UCharacterMovementComponent*>(Move)->PrimaryComponentTick.AddPrerequisite(this, PrimaryComponentTick);
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
		FlyLiftSpeed = 0.f;
		FlyMaxHeight = 0.f;
		UpdateVehicleMesh(nullptr);
		SetCharacterHidden(false);
	}
	else
	{
		CurrentParams = Row.NumericParams;
		ApplyVehicleRow(Row);
		SpinSpeed = GetParam(Row, TEXT("SpinSpeed"), 0.f);
		FlyLiftSpeed = FMath::Max(0.f, GetParam(Row, TEXT("FlyLiftSpeed"), 0.f));
		FlyMaxHeight = FMath::Max(0.f, GetParam(Row, TEXT("FlyMaxHeight"), 0.f));
		UpdateVehicleMesh(&Row);
		SetCharacterHidden(GetParam(Row, TEXT("HideCharacter"), 0.f) > 0.f && VehicleMesh && VehicleMesh->IsVisible());
	}

	bFlyWasFalling = false;

	// 부스터 키 확인, 프로펠러 회전, 비행이 필요할 때만 Tick
	SetComponentTickEnabled(BoostMultiplier > 1.f || !FMath::IsNearlyZero(SpinSpeed) || FlyLiftSpeed > 0.f);

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

	ACharacter* Character = GetCharacter();
	const APlayerController* PC = Character ? Cast<APlayerController>(Character->GetController()) : nullptr;
	if (!PC)
	{
		return;
	}

	if (FlyLiftSpeed > 0.f)
	{
		UpdateFly(Character, PC);
	}

	if (BoostMultiplier <= 1.f)
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

bool UPTBVehicleComponent::IsJumpHeld(const APlayerController* PC) const
{
	// 1순위: Enhanced Input의 점프 액션 값 (키 설정을 바꿔도 따라감)
	if (JumpInputAction)
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (const UEnhancedInputLocalPlayerSubsystem* Input = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				if (const UEnhancedPlayerInput* PlayerInput = Input->GetPlayerInput())
				{
					return PlayerInput->GetActionValue(JumpInputAction).Get<bool>();
				}
			}
		}
	}

	// 2순위: 지정한 키 직접 확인
	for (const FKey& Key : FlyKeys)
	{
		if (PC->IsInputKeyDown(Key))
		{
			return true;
		}
	}
	return false;
}

void UPTBVehicleComponent::UpdateFly(ACharacter* Character, const APlayerController* PC)
{
	UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	if (!Move)
	{
		return;
	}

	const bool bHeld = IsJumpHeld(PC);
	const bool bFalling = Move->IsFalling();
	const float CurrentZ = Character->GetActorLocation().Z;

	// 땅에서 떨어지는 순간의 높이를 기준으로 최대 높이 계산
	if (bFalling && !bFlyWasFalling)
	{
		FlyAnchorZ = CurrentZ;
	}
	bFlyWasFalling = bFalling;

	if (!bHeld)
	{
		return; // 키를 떼면 GravityScale대로 천천히 낙하
	}

	if (!bFalling)
	{
		// 땅에서 누르고 있으면 이륙 (착지 후 계속 누르고 있는 경우 포함)
		Character->LaunchCharacter(FVector(0.f, 0.f, FlyLiftSpeed), false, true);
		FlyAnchorZ = CurrentZ;
		return;
	}

	// 공중: 누르고 있는 동안 일정 속도로 상승, 최대 높이에서는 제자리 비행
	float TargetZVelocity = FlyLiftSpeed;
	if (FlyMaxHeight > 0.f && CurrentZ - FlyAnchorZ >= FlyMaxHeight)
	{
		TargetZVelocity = 0.f;
	}
	Move->Velocity.Z = TargetZVelocity;
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

	// 붙일 위치: 머리 뼈(AttachToHead) 또는 캐릭터 메시 원점(발밑)
	// 둘 다 캐릭터 메시 기준이라, PIE에서 PTBVehicleMesh의 Location/Rotation/Scale 값이
	// DataTable의 MeshOffsetX/Y/Z, MeshPitch/Yaw/Roll, MeshScale과 그대로 일치함
	const bool bAttachToHead = GetParam(*Row, TEXT("AttachToHead"), 0.f) > 0.f;
	USkeletalMeshComponent* CharMesh = Character->GetMesh();
	USceneComponent* Parent = CharMesh ? static_cast<USceneComponent*>(CharMesh) : Character->GetRootComponent();
	FName Socket = NAME_None;

	if (bAttachToHead && CharMesh)
	{
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

	VehicleMesh->AttachToComponent(Parent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);

	const FVector Offset(
		GetParam(*Row, TEXT("MeshOffsetX"), 0.f),
		GetParam(*Row, TEXT("MeshOffsetY"), 0.f),
		GetParam(*Row, TEXT("MeshOffsetZ"), 0.f));
	const FRotator Rotation(
		GetParam(*Row, TEXT("MeshPitch"), 0.f),
		GetParam(*Row, TEXT("MeshYaw"), 0.f),
		GetParam(*Row, TEXT("MeshRoll"), 0.f));
	VehicleMesh->SetRelativeLocationAndRotation(Offset, Rotation);
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
