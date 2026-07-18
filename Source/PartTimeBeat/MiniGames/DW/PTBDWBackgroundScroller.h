#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PTBDWBackgroundScroller.generated.h"


UCLASS()
class PARTTIMEBEAT_API APTBDWBackgroundScroller : public AActor
{
	GENERATED_BODY()

public:
	APTBDWBackgroundScroller();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/** 청크 한 개의 진행축 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float ChunkLength = 2000.0f;

	/** 동시 활성 청크 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	int32 ChunkCount = 5;

	/** 캐릭터 뒤로 미리 깔 청크 수. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	int32 ChunksBehind = 2;

	/** 스크롤 속도 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float ScrollSpeed = 600.0f;

	/** 청크 프리팹들 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	TArray<TSubclassOf<AActor>> ChunkClasses;

	/** BeginPlay에 자동 스크롤 시작 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	bool bAutoStart = true;

	UFUNCTION(BlueprintCallable, Category = "PTB|DW")
	void SetRunning(bool bInRunning) { bRunning = bInRunning; }

	/** 난이도 배속 × 배경 계수 적용: ScrollSpeed = 기본 × Scale × Multiplier. */
	UFUNCTION(BlueprintCallable, Category = "PTB|DW")
	void SetSpeedScale(float Scale, float Multiplier = 1.0f) { ScrollSpeed = BaseScrollSpeed * FMath::Max(0.01f, Scale) * FMath::Max(0.0f, Multiplier); }

	/** 현재 스크롤 속도. */
	UFUNCTION(BlueprintPure, Category = "PTB|DW")
	float GetScrollSpeed() const { return ScrollSpeed; }

private:
	/** 기본 스크롤 속도. */
	float BaseScrollSpeed = 600.0f;
	void SpawnChunks();
	TSubclassOf<AActor> PickChunkClass() const;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> ActiveChunks;

	TArray<float> ChunkOffsets;
	FVector BaseLocation = FVector::ZeroVector;
	FVector LaneDir = FVector::ForwardVector;
	bool bRunning = false;
};
