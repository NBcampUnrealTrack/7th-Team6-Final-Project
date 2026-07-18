#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWNoteToken.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** 노트 액터. 판정선으로 포물선 이동. */
UCLASS()
class PARTTIMEBEAT_API APTBDWNoteToken : public AActor
{
	GENERATED_BODY()

public:
	APTBDWNoteToken();

	/** 노트 구성(단일·롱 공용, 메시 1개). */
	void Configure(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale);

	/** hold 중 롱 노트 mesh 발광 강도. */
	void SetEmissive(float Strength);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW|Note")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW|Note")
	TArray<TObjectPtr<UStaticMeshComponent>> NoteMeshes;
private:
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> NoteMIDs;
};
