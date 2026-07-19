#include "MiniGames/DW/PTBDWNoteMarker.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

APTBDWNoteMarker::APTBDWNoteMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		Mesh->SetStaticMesh(CubeMesh.Object);
	}
	Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 0.05f));
}

void APTBDWNoteMarker::Configure(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale)
{
	if (InMesh)
	{
		Mesh->SetStaticMesh(InMesh);
	}
	Mesh->SetRelativeScale3D(InScale);

	UMaterialInterface* Base = InBaseMaterial ? InBaseMaterial : Mesh->GetMaterial(0);
	if (UMaterialInstanceDynamic* MID = Mesh->CreateDynamicMaterialInstance(0, Base))
	{
		MID->SetVectorParameterValue(TEXT("Color"), InColor);
	}
}

void APTBDWNoteMarker::SetMesh(UStaticMesh* InMesh)
{
	if (InMesh)
	{
		Mesh->SetStaticMesh(InMesh);
	}
}
