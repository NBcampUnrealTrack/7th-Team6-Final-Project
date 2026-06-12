#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBJJJumpActor.generated.h"

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

	/** 착지 시 BP 연출 훅(SFX, 스쿼시 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JumpJump")
	void OnLanded();

	/** 점프 시작 시 BP 연출 훅 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|JumpJump")
	void OnJumpStarted();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 점프 시각적 루트(이 컴포넌트의 Z를 움직여 점프 연출) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TObjectPtr<USceneComponent> JumpRoot;

	/** 최대 점프 높이(언리얼 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|JumpJump", meta = (ClampMin = "0"))
	float JumpHeight = 200.0f;

private:
	bool  bIsJumping = false;
	float Elapsed = 0.0f;       // 점프 시작 후 경과(초)
	float Duration = 0.0f;      // 총 체공시간(초)
	float CurrentHeight = 0.0f; // 이번 점프의 실제 최대 높이
	FVector BaseLocation = FVector::ZeroVector; // 점프 전 루트 로컬 위치
};
