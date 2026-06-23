

#include "MiniGames/PC/PCTileImageActor.h"
#include "Materials/MaterialInstanceDynamic.h"

APCTileImageActor::APCTileImageActor()
{
    PlaneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMesh"));
    RootComponent = PlaneMesh;

    // 기본 플레인 메시 설정
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube"));
    if (Cube.Succeeded())
        PlaneMesh->SetStaticMesh(Cube.Object);
}

void APCTileImageActor::SetTexture(UTexture2D* Texture)
{
    if (!Texture) return;

    // 에디터에서 BP_PCTileImageActor에 머티리얼 연결 필요
    if (!DynamicMaterial)
    {
        DynamicMaterial = PlaneMesh->CreateAndSetMaterialInstanceDynamic(0);
    }
    DynamicMaterial->SetTextureParameterValue(TEXT("Texture"), Texture);
}

void APCTileImageActor::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Warning, TEXT("TileImageActor BeginPlay 호출됨"));
}
