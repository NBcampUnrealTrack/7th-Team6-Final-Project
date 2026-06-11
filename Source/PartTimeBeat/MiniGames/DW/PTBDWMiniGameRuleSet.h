#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBDWMiniGameRuleSet.generated.h"

class UMaterialInterface;
class UStaticMesh;
class USkeletalMesh;
class UAnimSequenceBase;

UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBDWMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	// ── 장애물 ──────────────────────────────────────────────
	/** 액션별 장애물 오프셋 α. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, float> ObstacleOffsetByAction;

	/** 액션별 장애물 색. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FLinearColor> ObstacleColorByAction;

	/** 액션별 장애물 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> ObstacleMeshByAction;

	/** 액션별 장애물 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstacleScaleByAction;

	/** 장애물 색용 베이스 머티리얼. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TObjectPtr<UMaterialInterface> ObstacleMaterial;

	// ── 노트 마커(판정 지점) ────────────────────────────────
	/** 반투명 마커 머티리얼. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	TObjectPtr<UMaterialInterface> MarkerMaterial;

	/** 마커 색(알파=투명도). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	FLinearColor MarkerColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.35f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	FVector MarkerScale = FVector(0.3f);

	// ── 캐릭터(스켈레탈) ────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	TObjectPtr<USkeletalMesh> ProtagonistMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	FVector ProtagonistScale = FVector(1.0f);

	/** 주인공 메시 방향 보정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	float ProtagonistYaw = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	TObjectPtr<USkeletalMesh> DogMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	FVector DogScale = FVector(1.0f);

	/** 개 메시 방향 보정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Character")
	float DogYaw = 180.0f;

	// ── 캐릭터 애니메이션 ───────────────────────────────────
	/** 주인공 달리기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> ProtagonistRunAnim;

	/** Action A 성공 시 재생 후 달리기 복귀. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> ProtagonistJumpAnim;

	/** 개 달리기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> DogRunAnim;

	// ── 기타 ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float DogCueMaxBeats = 0.5f;

	/** 마커 스폰 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float MarkerSpawnDistance = 1200.0f;
};
