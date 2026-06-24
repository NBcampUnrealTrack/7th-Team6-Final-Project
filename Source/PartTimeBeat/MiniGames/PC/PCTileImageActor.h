
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCTileImageActor.generated.h"

UCLASS()
class PARTTIMEBEAT_API APCTileImageActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APCTileImageActor();

	void SetTexture(UTexture2D* Texture);


	UFUNCTION(BlueprintCallable)
	void ActivateGlow();

	UPROPERTY(EditAnywhere)
	UMaterialInterface* GlowMaterial; // 에디터에서 오버레이 머티리얼 연결


private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* PlaneMesh;

	UPROPERTY()
	UMaterialInstanceDynamic* DynamicMaterial;

protected:
	virtual void BeginPlay() override;
};
