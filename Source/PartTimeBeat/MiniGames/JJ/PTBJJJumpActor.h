#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Core/PTBStructEnums.h" 
#include "PTBJJJumpActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * JumpJump 미니게임에서 박자에 맞춰 점프하는 캐릭터 Actor입니다.
 *
 * 물리 점프가 아니라 "체공시간(AirtimeMs)을 명시적으로 받아" 그 시간 동안
 * 정확히 포물선을 그리며 떴다가 내려오는 연출 점프입니다.
 * 따라서 StartJump() 호출 후 정확히 AirtimeMs 뒤에 바닥(Z=0 오프셋)에 착지합니다.
 *
 * 리듬게임 특성상 "착지 시각 = 노트 정시점"이 보장되어야 하므로,
 * 미니게임 쪽에서 (정시점 - 체공시간) 시점에 StartJump를 호출하면
 * 노트 정시점에 자동으로 착지가 맞아떨어집니다.
 *
 * 발밑의 JudgePlane은 판정 결과에 따라 색을 짧게 플래시한 뒤 기본색으로 복귀합니다.
 */
UCLASS()
class PARTTIMEBEAT_API APTBJJJumpActor : public AActor
{
	GENERATED_BODY()

public:
	APTBJJJumpActor();


	/**
	 * 점프 시작.
	 * @param AirtimeMs   체공시간(ms). 이 시간 뒤에 정확히 착지한다.
	 * @param HeightScale 점프 높이 배율(1.0 = JumpHeight 그대로).
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void StartJump(float AirtimeMs, float HeightScale = 1.0f);

	/** 점프 진행 중인지 */
	UFUNCTION(BlueprintPure, Category = "PTB|JumpJump")
	bool IsJumping() const { return bIsJumping; }

	/** 판정 타입에 따라 Plane 색을 플래시(짧게 표시 후 기본색 복귀) */
	UFUNCTION(BlueprintCallable, Category = "PTB|JumpJump")
	void FlashJudgementColor(EPTBJudgementType JudgementType);

	/** 착지 시 BP 연출 훅(SFX, 스쿼시 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JumpJump")
	void OnLanded();

	/** 점프 시작 시 BP 연출 훅 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JumpJump")
	void OnJumpStarted();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 고정 루트(안 움직임). JumpRoot와 JudgePlane의 공통 부모 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 점프 시각적 루트(이 컴포넌트의 Z를 움직여 점프 연출) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TObjectPtr<USceneComponent> JumpRoot;

	/** 판정 색을 표시할 바닥 Plane (루트 직속 → 점프해도 바닥 고정) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TObjectPtr<UStaticMeshComponent> JudgePlane;

	/** 최대 점프 높이(언리얼 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0"))
	float JumpHeight = 200.0f;

	/** 판정 색 유지 + 페이드 총 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0"))
	float FlashDuration = 0.35f;

	/** 기본(평상시) Plane 색 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump")
	FLinearColor DefaultPlaneColor = FLinearColor(0.05f, 0.05f, 0.05f);

	/** Plane 머티리얼의 색 파라미터 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump")
	FName ColorParamName = TEXT("BaseColor");

private:
	// 점프 상태
	bool  bIsJumping = false;
	float Elapsed = 0.0f;       // 점프 시작 후 경과(초)
	float Duration = 0.0f;      // 총 체공시간(초)
	float CurrentHeight = 0.0f; // 이번 점프의 실제 최대 높이
	FVector BaseLocation = FVector::ZeroVector; // 점프 전 루트 로컬 위치

	// 판정 색 플래시 상태
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PlaneMaterial;

	FLinearColor FlashColor = FLinearColor::White; // 이번 플래시 시작색
	float FlashElapsed = 0.0f;                     // 플래시 경과(초)
	bool  bFlashing = false;
};