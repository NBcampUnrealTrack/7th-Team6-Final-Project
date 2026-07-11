#include "MiniGames/DW/PTBDWNoteToken.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

APTBDWNoteToken::APTBDWNoteToken()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void APTBDWNoteToken::Configure(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale)
{
	ConfigureNotes(1, InMesh, InBaseMaterial, InColor, InScale);
}

void APTBDWNoteToken::ConfigureNotes(int32 Count, UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale)
{
	Count = FMath::Max(1, Count);

	UStaticMesh* UseMesh = InMesh;
	if (!UseMesh)
	{
		UseMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	}

	while (NoteMeshes.Num() < Count)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this);
		if (!Comp)
		{
			break;
		}
		Comp->SetupAttachment(Root);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Comp->RegisterComponent();
		NoteMeshes.Add(Comp);
		NoteMIDs.Add(nullptr);
	}

	for (int32 i = 0; i < Count; ++i)
	{
		UStaticMeshComponent* Comp = NoteMeshes[i];
		if (!Comp)
		{
			continue;
		}
		Comp->SetVisibility(true);
		if (UseMesh)
		{
			Comp->SetStaticMesh(UseMesh);
		}
		Comp->SetRelativeScale3D(InScale);

		UMaterialInterface* Base = InBaseMaterial ? InBaseMaterial : Comp->GetMaterial(0);
		UMaterialInstanceDynamic* MID = Comp->CreateDynamicMaterialInstance(0, Base);
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), InColor);
		}
		NoteMIDs[i] = MID;
	}
}

void APTBDWNoteToken::ConfigureTail(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor)
{
	if (!TailMesh)
	{
		TailMesh = NewObject<UStaticMeshComponent>(this);
		if (TailMesh)
		{
			TailMesh->SetupAttachment(Root);
			TailMesh->SetMobility(EComponentMobility::Movable);
			TailMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			TailMesh->RegisterComponent();
		}
	}
	if (!TailMesh)
	{
		return;
	}

	UStaticMesh* UseMesh = InMesh;
	if (!UseMesh)
	{
		UseMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	}
	if (UseMesh)
	{
		TailMesh->SetStaticMesh(UseMesh);
	}

	UMaterialInterface* Base = InBaseMaterial ? InBaseMaterial : TailMesh->GetMaterial(0);
	TailMID = TailMesh->CreateDynamicMaterialInstance(0, Base);
	if (TailMID)
	{
		TailMID->SetVectorParameterValue(TEXT("Color"), InColor);
	}
	TailMesh->SetVisibility(false);
}

int32 APTBDWNoteToken::GetNoteCount() const
{
	return NoteMeshes.Num();
}

void APTBDWNoteToken::SetTailSegment(const FVector& HeadWorld, const FVector& TailEndWorld, float Thickness)
{
	if (!TailMesh)
	{
		return;
	}
	const FVector Delta = TailEndWorld - HeadWorld;
	const float Len = Delta.Size();
	if (Len < 1.0f)
	{
		TailMesh->SetVisibility(false);
		return;
	}
	TailMesh->SetVisibility(true);
	TailMesh->SetWorldLocation((HeadWorld + TailEndWorld) * 0.5f);
	TailMesh->SetWorldRotation(FRotationMatrix::MakeFromZ(Delta / Len).Rotator());
	const float Th = FMath::Max(1.0f, Thickness);
	TailMesh->SetWorldScale3D(FVector(Th / 100.0f, Th / 100.0f, Len / 100.0f));
}

void APTBDWNoteToken::HideTail()
{
	if (TailMesh)
	{
		TailMesh->SetVisibility(false);
	}
}

void APTBDWNoteToken::SetColor(FLinearColor InColor)
{
	for (UMaterialInstanceDynamic* MID : NoteMIDs)
	{
		if (MID)
		{
			MID->SetVectorParameterValue(TEXT("Color"), InColor);
		}
	}
	if (TailMID)
	{
		TailMID->SetVectorParameterValue(TEXT("Color"), InColor);
	}
}

void APTBDWNoteToken::SetEmissive(float Strength)
{
	for (UMaterialInstanceDynamic* MID : NoteMIDs)
	{
		if (MID)
		{
			MID->SetScalarParameterValue(TEXT("EmissiveStrength"), Strength);
		}
	}
}
