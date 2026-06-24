

#include "MiniGames/PC/PCTileImageActor.h"
#include "Materials/MaterialInstanceDynamic.h"

APCTileImageActor::APCTileImageActor()
{
    PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
    RootComponent = PlaneMesh;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube"));
    if (Cube.Succeeded())
        PlaneMesh->SetStaticMesh(Cube.Object);
}

void APCTileImageActor::SetTexture(UTexture2D* Texture)
{
    if (!Texture) return;

    if (!DynamicMaterial)
    {
        DynamicMaterial = PlaneMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
    DynamicMaterial->SetTextureParameterValue(TEXT("Texture"), Texture);
}

void APCTileImageActor::ActivateGlow()
{
    if (GlowMaterial)
    {
        PlaneMesh->SetOverlayMaterial(GlowMaterial);
    }
}

void APCTileImageActor::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("TileImageActor BeginPlay 호출됨"));
}
