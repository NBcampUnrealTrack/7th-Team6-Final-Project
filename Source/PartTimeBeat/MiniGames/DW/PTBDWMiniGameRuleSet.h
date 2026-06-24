#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "Templates/SubclassOf.h"
#include "PTBDWMiniGameRuleSet.generated.h"

class UMaterialInterface;
class UStaticMesh;
class USkeletalMesh;
class UAnimSequenceBase;
class UNiagaraSystem;
class UAnimInstance;

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

	/** 액션별 "부서진" 장애물 메시. 없으면 코드 placeholder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> BrokenObstacleMeshByAction;

	/** 롱 노트 시각 길이 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float HoldVisualScale = 0.5f;

	/** 장애물 전용 롱 길이 추가 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float ObstacleHoldScale = 0.5f;

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
	FLinearColor MarkerColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.85f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	FVector MarkerScale = FVector(0.3f);

	/** 스폰 시 정사각형 한 변. 바닥 납작 마커. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	float MarkerSquareSize = 0.6f;

	/** 판정선 도달 시 진행축(X) 두께. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	float MarkerThinX = 0.08f;

	/** 마커 Z 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	float MarkerFlatZ = 0.05f;

	/** 롱 노트 꼬리 색·투명도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	FLinearColor MarkerTailColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.20f);

	/** 판정선 고정 타깃 바 표시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	bool bShowJudgeTarget = true;

	/** 고정 타깃 바 색. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Marker")
	FLinearColor JudgeTargetColor = FLinearColor(1.0f, 0.85f, 0.2f, 0.9f);

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

	/** Action B 성공. 없으면 placeholder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> ProtagonistSlideAnim;

	/** 실패 리액션. 없으면 placeholder. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> ProtagonistFailAnim;

	/** 주인공 AnimBP. 비우면 싱글노드 폴백. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TSubclassOf<UAnimInstance> ProtagonistAnimClass;

	/** 개 달리기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim")
	TObjectPtr<UAnimSequenceBase> DogRunAnim;

	// ── 연출 SFX 키 ──────────────────────────────

	/** 개 예고음(Cue 시점, 개 위치). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Audio")
	FName DogCueSFXKey = NAME_None;

	/** A 성공 도약음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Audio")
	FName JumpSFXKey = NAME_None;

	/** A 롱 성공 시 추가 물첨벙음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Audio")
	FName SplashSFXKey = NAME_None;

	/** B 성공 슬라이드음. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Audio")
	FName SlideSFXKey = NAME_None;

	/** 실패 리액션음. 베이스 FailSFXKey와 별개. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Audio")
	FName FailReactionSFXKey = NAME_None;

	// ── 연출 VFX (Niagara) ──────────────────────────────────

	/** 액션별 실패 파괴 VFX. ex) A 허들 파괴. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TMap<EPTBActionType, TObjectPtr<UNiagaraSystem>> BreakVFXByAction;

	/** A 롱 성공 시 물첨벙 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> SplashVFX;

	/** B 성공 시 슬라이드 먼지 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> SlideDustVFX;

	// ── 기타 ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float DogCueMaxBeats = 0.5f;

	/** 마커 스폰 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float MarkerSpawnDistance = 1200.0f;
};
