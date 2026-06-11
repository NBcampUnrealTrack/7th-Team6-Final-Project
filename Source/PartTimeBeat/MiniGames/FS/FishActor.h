// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "FishActor.generated.h"

UCLASS()
class PARTTIMEBEAT_API AFishActor : public AActor
{
	GENERATED_BODY()

public:
	AFishActor();
 
	// PTBFishingMiniGame에서 판정 결과에 따라 호출
	// NewLocation = 현재 위치에서 StepDistance만큼 캐릭터 방향으로 이동한 목표 위치
	UFUNCTION(BlueprintCallable, Category = "Fish")
	void SetTargetLocation(FVector NewLocation);
 
	// 현재 TargetLocation 반환 (PTBFishingMiniGame에서 StepDistance 계산에 사용)
	UFUNCTION(BlueprintPure, Category = "Fish")
	FVector GetFishTargetLocation() const { return TargetLocation; }
 
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
 
	// 물고기 스켈레탈 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fish")
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
 
	// 보간 속도 — 에디터에서 조정 가능
	UPROPERTY(EditDefaultsOnly, Category = "Fish")
	float InterpSpeed = 1.0f;
 
private:
	// Tick에서 현재 위치 → TargetLocation으로 VInterpTo 보간
	FVector TargetLocation;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Anim",meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> SwimMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish|Anim",meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> IdleMontage;
};
