#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "Items/PTBItemTypes.h"
#include "PTBVehicleComponent.generated.h"

class ACharacter;
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
 *  AirControl           공중 조작감               (기본 = 캐릭터 원래값)
 *
 * 외형(선택): 행의 Asset에 Static Mesh를 넣으면 캐릭터에 붙여서 표시 (AttachSocketName 소켓, 없으면 루트)
 * 애니메이션 변경 등은 BP의 OnVehicleChanged 이벤트에서 처리
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

	/** 탈것 외형 메시를 붙일 소켓 (비우거나 없으면 캐릭터 메시 루트) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Vehicle")
	FName AttachSocketName = NAME_None;

private:
	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	void CacheBaseMovement();
	void RestoreBaseMovement();
	void ApplyVehicleRow(const FPTBItemRow& Row);
	void UpdateVehicleMesh(const FPTBItemRow* Row);
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
	float SpeedMultiplier = 1.f;
	float BoostMultiplier = 1.f;
	bool bBoosting = false;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> VehicleMesh;
};
