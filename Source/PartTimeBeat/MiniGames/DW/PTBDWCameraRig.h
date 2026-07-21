#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWCameraRig.generated.h"

class USpringArmComponent;
class UCameraComponent;


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDWOnIntroMoveFinished);

UCLASS()
class PARTTIMEBEAT_API APTBDWCameraRig : public AActor
{
	GENERATED_BODY()

public:
	APTBDWCameraRig();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** 시작 연출 카메라 이동. */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW|Intro")
	void StartIntroMove();

	/** 이동 없이 인트로 포즈(하늘 뷰)로 즉시 스냅. 대기 화면용. */
	void SnapToIntroPose();

	/** 실패 연출 카메라 흔들림. */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW")
	void PlayFailShake(float Intensity, float Duration, float Frequency);

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

	// ── 인트로 카메라 이동 ──
	/** 인트로 시작 포즈. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroArmLength = 1400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroPitch = -70.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroYaw = -90.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroFieldOfView = 60.0f;
	/** 이동 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroMoveDuration = 2.0f;
	/** ease-in-out 강도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	float IntroEaseExp = 2.0f;
	/** BeginPlay에 인트로 포즈로 대기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Intro")
	bool bStartAtIntroPose = true;

	/** 인트로 이동 완료. */
	UPROPERTY(BlueprintAssignable, Category = "PTB|DW|Intro")
	FDWOnIntroMoveFinished OnIntroMoveFinished;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "PTB|DW")
	TObjectPtr<UCameraComponent> Camera;

private:
	void ApplyPose(float InArm, float InPitch, float InYaw, float InFOV);
	bool bIntroMoving = false;
	float IntroElapsed = 0.0f;

	// ── 실패 흔들림 상태 ──
	float ShakeTimer = 0.0f;
	float ShakeDuration = 0.0f;
	float ShakeIntensity = 0.0f;
	float ShakeFrequency = 30.0f;
	float ShakePhase = 0.0f;
};
