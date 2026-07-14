#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWNoteToken.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** 노트마커(음표) 액터. 단일=음표 1개, 판정선으로 포물선 이동. */
UCLASS()
class PARTTIMEBEAT_API APTBDWNoteToken : public AActor
{
	GENERATED_BODY()

public:
	APTBDWNoteToken();

	/** 단일 음표 구성. */
	void Configure(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale);

	/** 음표 Count개 구성(롱노트 머리 뭉치용). */
	void ConfigureNotes(int32 Count, UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor, FVector InScale);

	/** 혜성 꼬리 메시 구성(롱노트용). */
	void ConfigureTail(UStaticMesh* InMesh, UMaterialInterface* InBaseMaterial, FLinearColor InColor);

	/** 음표 개수. */
	int32 GetNoteCount() const;

	/** 꼬리를 머리~꼬리끝 사이로 늘림. */
	void SetTailSegment(const FVector& HeadWorld, const FVector& TailEndWorld, float Thickness);

	/** 꼬리 숨김. */
	void HideTail();

	/** 전체 음표 색 갱신(발광·VFX 등). */
	void SetColor(FLinearColor InColor);

	/** hold 중 음표 mesh 발광 강도. */
	void SetEmissive(float Strength);

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW|Note")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW|Note")
	TArray<TObjectPtr<UStaticMeshComponent>> NoteMeshes;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW|Note")
	TObjectPtr<UStaticMeshComponent> TailMesh;

private:
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> NoteMIDs;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> TailMID;
};
