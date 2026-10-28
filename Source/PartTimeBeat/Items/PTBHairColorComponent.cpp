#include "Items/PTBHairColorComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Items/PTBItemSubsystem.h"
#include "Items/PTBItemTypes.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TimerManager.h"

UPTBHairColorComponent::UPTBHairColorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UPTBHairColorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UPTBItemSubsystem* Items = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UPTBItemSubsystem>() : nullptr)
	{
		Items->OnItemStateChanged.AddUniqueDynamic(this, &UPTBHairColorComponent::HandleItemStateChanged);
	}

	// BP BeginPlay에서 성별에 따라 메시를 바꾸는 경우를 위해 다음 프레임에 적용
	GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		ReapplyHairColor();
	}));
}

void UPTBHairColorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UPTBItemSubsystem* Items = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UPTBItemSubsystem>() : nullptr)
	{
		Items->OnItemStateChanged.RemoveDynamic(this, &UPTBHairColorComponent::HandleItemStateChanged);
	}
	Super::EndPlay(EndPlayReason);
}

void UPTBHairColorComponent::HandleItemStateChanged(FName Category)
{
	// None = 프로필 변경 등 전체 갱신
	if (Category.IsNone() || Category == PTBItemCategory::Hair)
	{
		ReapplyHairColor();
	}
}

void UPTBHairColorComponent::ReapplyHairColor()
{
	if (!PreviewItemId.IsNone())
	{
		ApplyItem(PreviewItemId);
		return;
	}

	const UPTBItemSubsystem* Items = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UPTBItemSubsystem>() : nullptr;
	ApplyItem(Items ? Items->GetEquippedItemId(PTBItemCategory::Hair) : NAME_None);
}

void UPTBHairColorComponent::PreviewHairItem(FName ItemId)
{
	PreviewItemId = ItemId;
	ReapplyHairColor();
}

void UPTBHairColorComponent::ApplyItem(FName ItemId)
{
	UMaterialInstanceDynamic* MID = GetOrCreateHairMID();
	if (!MID)
	{
		return;
	}

	const UPTBItemSubsystem* Items = GetWorld() && GetWorld()->GetGameInstance()
		? GetWorld()->GetGameInstance()->GetSubsystem<UPTBItemSubsystem>() : nullptr;

	bool bFound = false;
	const FPTBItemRow Row = Items ? Items->GetItemData(ItemId, bFound) : FPTBItemRow();

	// 아이템이 없거나 기본 아이템이면 원본 텍스처 그대로
	const bool bUseOriginal = !bFound || Row.bIsDefault;
	MID->SetScalarParameterValue(TintStrengthParamName, bUseOriginal ? 0.f : 1.f);
	if (!bUseOriginal)
	{
		MID->SetVectorParameterValue(ColorParamName, Row.Color);
	}

	PTB_VERBOSE(LogPTBProfile, TEXT("[Hair] Apply item=%s original=%d"), *ItemId.ToString(), bUseOriginal ? 1 : 0);
}

USkeletalMeshComponent* UPTBHairColorComponent::ResolveMesh() const
{
	if (TargetMesh)
	{
		return TargetMesh;
	}
	if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		return Character->GetMesh();
	}
	return GetOwner() ? GetOwner()->FindComponentByClass<USkeletalMeshComponent>() : nullptr;
}

int32 UPTBHairColorComponent::FindHairSlotIndex(USkeletalMeshComponent* Mesh) const
{
	if (!Mesh)
	{
		return INDEX_NONE;
	}

	if (!HairSlotName.IsNone())
	{
		return Mesh->GetMaterialIndex(HairSlotName);
	}

	const TArray<FName> SlotNames = Mesh->GetMaterialSlotNames();
	for (int32 i = 0; i < Mesh->GetNumMaterials(); ++i)
	{
		const FString SlotName = SlotNames.IsValidIndex(i) ? SlotNames[i].ToString() : FString();
		const UMaterialInterface* Material = Mesh->GetMaterial(i);
		const FString MaterialName = Material ? Material->GetName() : FString();

		if (SlotName.Contains(HairKeyword, ESearchCase::IgnoreCase)
			|| MaterialName.Contains(HairKeyword, ESearchCase::IgnoreCase))
		{
			return i;
		}
	}
	return INDEX_NONE;
}

UMaterialInstanceDynamic* UPTBHairColorComponent::GetOrCreateHairMID()
{
	USkeletalMeshComponent* Mesh = ResolveMesh();
	if (!Mesh)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Hair] %s: SkeletalMeshComponent를 찾지 못함"), *GetNameSafe(GetOwner()));
		return nullptr;
	}

	// 같은 메시·같은 에셋이면 기존 MID 재사용 (성별 교체로 에셋이 바뀌면 다시 생성)
	const UObject* MeshAsset = Mesh->GetSkeletalMeshAsset();
	if (HairMID && MIDOwnerMesh.Get() == Mesh && MIDSourceMeshAsset.Get() == MeshAsset
		&& Mesh->GetMaterial(MIDSlotIndex) == HairMID)
	{
		return HairMID;
	}

	const int32 SlotIndex = FindHairSlotIndex(Mesh);
	if (SlotIndex == INDEX_NONE)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Hair] %s: 머리 머티리얼 슬롯을 찾지 못함 (HairSlotName 또는 HairKeyword 확인)"),
			*GetNameSafe(MeshAsset));
		return nullptr;
	}

	HairMID = Mesh->CreateDynamicMaterialInstance(SlotIndex);
	MIDOwnerMesh = Mesh;
	MIDSourceMeshAsset = const_cast<UObject*>(MeshAsset);
	MIDSlotIndex = SlotIndex;

	PTB_RECORD(LogPTBProfile, TEXT("[Hair] MID 생성: mesh=%s slot=%d"), *GetNameSafe(MeshAsset), SlotIndex);
	return HairMID;
}
