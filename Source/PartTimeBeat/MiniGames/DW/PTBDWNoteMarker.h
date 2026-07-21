#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWNoteMarker.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class PARTTIMEBEAT_API APTBDWNoteMarker : public AActor
{
	GENERATED_BODY()

public:
	APTBDWNoteMarker();

	/** 메시/베이스 머티리얼/색/스케일 지정. */
	void Configure(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale);

	/** 메시만 교체(머티리얼/스케일 유지) — 실패 시 부서진 메시로 스왑. */
	void SetMesh(UStaticMesh* InMesh);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<UStaticMeshComponent> Mesh;
};
