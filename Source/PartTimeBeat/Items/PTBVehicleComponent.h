#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "Items/PTBItemTypes.h"
#include "PTBVehicleComponent.generated.h"

class ACharacter;
class APlayerController;
class UInputAction;
class UStaticMeshComponent;

/**
 * 장착한 탈것 아이템 효과를 캐릭터 이동에 실시간 적용.
 *
 * 사용법: 허브 캐릭터 BP에 컴포넌트로 추가 (PTB Hair Color와 같은 캐릭터)
 *  - 장착이 바뀌거나 프로필이 바뀌면 자동으로 다시 적용
 *  - 미지정(기본 아이템)이면 캐릭터 원래 이동값 그대로
 *
 * 탈것 효과는 DataTable 행의 NumericParams로 지정 (없는 키는 기본값, 조합 가능)
 *  SpeedMultiplier      걷기 속도 배율            (기본 1)    예: 자동차 1.5
 *  ExtraJumpCount       공중 추가 점프 횟수       (기본 0)    예: 이단 점프 1
 *  BoostSpeedMultiplier 부스트 키 누르는 동안 속도 배율 (기본 1, 1이면 부스트 없음) 예: 부스터 1.8
 *  JumpZMultiplier      점프 높이 배율            (기본 1)
 *  JumpHoldTime         점프 키를 누르고 있으면 계속 상승하는 최대 시간(초) (기본 0) 예: 대나무 헬리콥터 1.5
 *  GravityScale         중력 배율                 (기본 = 캐릭터 원래값) 예: 대나무 헬리콥터 0.5 (천천히 낙하)
 *  FlyLiftSpeed         비행: 점프 키를 누르고 있는 동안 상승 속도(cm/s). 떨어지는 중에 다시 누르면 다시 상승 (기본 0 = 비행 없음) 예: 대나무 헬리콥터 400
 *  FlyMaxHeight         비행: 이륙 지점 기준 최대 높이(cm). 도달하면 누르고 있는 동안 제자리 비행 (기본 0 = 제한 없음) 예: 800
 *  AirControl           공중 조작감               (기본 = 캐릭터 원래값)
 *  DisableJump          1이면 점프 불가           (예: 자동차)
 *
 * 외형 (행의 Asset에 Static Mesh 지정)
 *  HideCharacter        1이면 캐릭터를 숨기고 탈것 메시만 표시 (뚜껑 닫힌 자동차 등 "통째로 치환")
 *  AttachToHead         1이면 머리 뼈(HeadSocketName)에 붙임 (대나무 헬리콥터 프로펠러 등). 0이면 캐릭터 메시 원점(발밑)
 *  MeshOffsetX/Y/Z      위치 보정 (cm)
 *  MeshPitch/Yaw/Roll   회전 보정 (도)  예: 자동차 앞뒤가 반대면 MeshYaw 180, 머리 뼈 축 때문에 옆으로 누우면 Pitch/Roll 90
 *  MeshScale            크기 배율 (기본 1)
 *  SpinSpeed            초당 회전 각도, 메시 자체의 Z축 기준 (예: 프로펠러 1080)
 *
 * 값 맞추는 법: PIE → F8(Eject) → Outliner에서 캐릭터 선택 → Details에서 PTBVehicleMesh 선택 →
 *              Transform을 직접 조절 → 보이는 Location/Rotation/Scale 값을 DataTable에 그대로 옮기기
 *              (DataTable 수정은 PIE를 다시 시작해야 반영됨)
 *
 * 애니메이션: ABP에서 GetVehicleParam("HoverPose") 등으로 읽어서 처리 (예: 헬리콥터 비행 중 공중 대기 포즈)
 */
UCLASS(ClassGroup = (PTB), meta = (BlueprintSpawnableComponent))
class PARTTIMEBEAT_API UPTBVehicleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPTBVehicleComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** 현재 장착 탈것으로 다시 적용 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Vehicle")
	void ReapplyVehicle();

	/** 현재 적용 중인 탈것 아이템 ID */
	UFUNCTION(BlueprintPure, Category = "PTB|Vehicle")
	FName GetCurrentVehicleId() const { return CurrentVehicleId; }

	/** 부스트 중인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|Vehicle")
	bool IsBoosting() const { return bBoosting; }

	/** 현재 탈것의 NumericParams 값 (없으면 Default). ABP에서 애니메이션 분기용 (예: HoverPose) */
	UFUNCTION(BlueprintPure, Category = "PTB|Vehicle")
	float GetVehicleParam(FName Key, float Default = 0.f) const;

	/** 현재 캐릭터를 숨기고 탈것만 보이는 상태인지 (HideCharacter) */
	UFUNCTION(BlueprintPure, Category = "PTB|Vehicle")
	bool IsCharacterHidden() const { return bCharacterHidden; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** 탈것이 바뀐 직후 (BP: 애니메이션 교체, 효과음 등). bIsNone=true면 탈것 해제 상태 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Vehicle")
	void OnVehicleChanged(FName VehicleId, const FPTBItemRow& VehicleData, bool bIsNone);

	/** 부스트 시작/종료 (BP: 이펙트, 효과음 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Vehicle")
	void OnBoostChanged(bool bNowBoosting);

	/** 부스트 키 (부스터 탈것일 때만 동작) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Vehicle")
	TArray<FKey> BoostKeys = { EKeys::LeftShift, EKeys::RightShift };

	/**
	 * 비행(FlyLiftSpeed)용 점프 입력. 캐릭터 점프에 쓰는 Input Action(예: IA_Jump)을 지정하면 키 설정 변경도 따라감.
	 * 비워 두면 FlyKeys로 확인
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Vehicle")
	TObjectPtr<UInputAction> JumpInputAction;

	/** JumpInputAction이 비었을 때 비행 상승에 쓸 키 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Vehicle")
	TArray<FKey> FlyKeys = { EKeys::SpaceBar, EKeys::Gamepad_FaceButton_Bottom };

	/** AttachToHead=1인 탈것을 붙일 머리 뼈/소켓 이름 (PTB_Girl/PTB_Boy는 Bip001 계열 뼈 이름) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Vehicle")
	FName HeadSocketName = TEXT("Bip001-Head");

private:
	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	void CacheBaseMovement();
	void RestoreBaseMovement();
	void ApplyVehicleRow(const FPTBItemRow& Row);
	void UpdateVehicleMesh(const FPTBItemRow* Row);
	void SetCharacterHidden(bool bHidden);
	void UpdateFly(ACharacter* Character, const APlayerController* PC);
	bool IsJumpHeld(const APlayerController* PC) const;
	void SetBoosting(bool bInBoosting);
	float GetParam(const FPTBItemRow& Row, const TCHAR* Key, float Default) const;

	ACharacter* GetCharacter() const;

	// 캐릭터 원래 이동값 (BeginPlay 시점)
	bool bBaseCached = false;
	float BaseMaxWalkSpeed = 0.f;
	float BaseJumpZVelocity = 0.f;
	float BaseGravityScale = 1.f;
	float BaseAirControl = 0.f;
	float BaseJumpMaxHoldTime = 0.f;
	int32 BaseJumpMaxCount = 1;

	// 현재 탈것 적용값
	FName CurrentVehicleId = NAME_None;
	TMap<FName, float> CurrentParams;
	float SpinSpeed = 0.f;
	bool bCharacterHidden = false;

	// 비행
	float FlyLiftSpeed = 0.f;
	float FlyMaxHeight = 0.f;
	float FlyAnchorZ = 0.f;
	bool bFlyWasFalling = false;
	float SpeedMultiplier = 1.f;
	float BoostMultiplier = 1.f;
	bool bBoosting = false;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> VehicleMesh;
};
