#pragma once

#include "CoreMinimal.h"
#include "Core/PTBStructEnums.h"
#include "GameFramework/Actor.h"
#include "PTBLCLogisticBox.generated.h"


/** 박스의 현재 상태를 나타내는 Enum */
UENUM(BlueprintType)
enum class EPTBLCLogisticBoxState : uint8
{
	UnpackagedRed = 0 UMETA(DisplayName = "Unpackaged Red"),
	UnpackagedBlue = 1 UMETA(DisplayName = "Unpackaged Blue"),
	UnpackagedYellow = 2 UMETA(DisplayName = "Unpackaged Yellow"),
	RedBox = 3 UMETA(DisplayName = "RedBox"),
	BlueBox = 4 UMETA(DisplayName = "BlueBox"),
	YellowBox = 5 UMETA(DisplayName = "YellowBox"),
};

/**
 * LC 미니게임에서 사용되는 물류 박스 Actor입니다.
 * 박스는 언패키징된 상태(색상별 삼각형 메시)와 패키징된 상태(색상별 박스 메시)로 구분됩니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBLCLogisticBox : public AActor
{
	GENERATED_BODY()

public:
	APTBLCLogisticBox();
	
	virtual void Tick(float DeltaTime) override;
	
	EPTBLCLogisticBoxState GetLogisticBoxState() const;
	
	void SetLogisticBoxState(EPTBLCLogisticBoxState NewState);
	
	UStaticMeshComponent* GetStaticMeshComponent() const;
	
	UMaterialInstance* GetRedMaterialInstance() const;
	
	UMaterialInstance* GetBlueMaterialInstance() const;
	
	UMaterialInstance* GetYellowMaterialInstance() const;

	void SetBoxMaterlalInstanceByActionType(EPTBActionType ActionType);
	
	void SetBoxStateByActionType(EPTBActionType ActionType);
	
	bool GetIsPackaged() const;
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "PTB|LC|Mesh")
	void StopMovingAndEnablePhysics();
	
	void ChangeMeshToBox();
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMeshComponent> BaseMeshComponent;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> BoxMeshAsset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UStaticMesh> TriangleMeshAsset;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> RedBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> BlueBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|Mesh")
	TObjectPtr<UMaterialInstance> YellowBoxMaterialInstance;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|LC|State")
	EPTBLCLogisticBoxState BoxState;
private:
	
	UPROPERTY(EditAnywhere, Category = "PTB|LC|Param")
	float MovingSpeed = 300.f;
	
	bool bIsMoving = true;
	
	bool bIsPackaged = false;
};
