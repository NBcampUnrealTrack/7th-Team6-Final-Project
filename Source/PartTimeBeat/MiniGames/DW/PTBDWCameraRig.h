#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWCameraRig.generated.h"

class USpringArmComponent;
class UCameraComponent;


UCLASS()
class PARTTIMEBEAT_API APTBDWCameraRig : public AActor
{
	GENERATED_BODY()

public:
	APTBDWCameraRig();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	/** 카메라 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float ArmLength = 1000.0f;

	/** 내려다보는 각. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float Pitch = -20.0f;

	/** 사이드 각. 90이면 정측면 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float Yaw = 65.0f;

	/** 시야각. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float FieldOfView = 70.0f;

	/** BeginPlay에 이 카메라를 플레이어 시점으로. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	bool bAutoPossess = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float BlendTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<UCameraComponent> Camera;
};
