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
	UStaticMesh* UseMesh = InMesh;
	if (!UseMesh)
	{
		UseMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	}

	if (NoteMeshes.Num() == 0)
	{
		UStaticMeshComponent* NewComp = NewObject<UStaticMeshComponent>(this);
		if (NewComp)
		{
			NewComp->SetupAttachment(Root);
			NewComp->SetMobility(EComponentMobility::Movable);
			NewComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			NewComp->RegisterComponent();
			NoteMeshes.Add(NewComp);
			NoteMIDs.Add(nullptr);
		}
	}

	if (NoteMeshes.Num() == 0) { return; }
	UStaticMeshComponent* Comp = NoteMeshes[0];
	if (!Comp) { return; }

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
	NoteMIDs[0] = MID;
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
