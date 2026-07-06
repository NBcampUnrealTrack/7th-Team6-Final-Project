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

	// ── A·B 실패 2조각 V자 갈라짐 ──
	/** 왼쪽 조각 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> BrokenPieceLeftByAction;
	/** 오른쪽 조각 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> BrokenPieceRightByAction;
	/** 왼쪽 조각 최종 회전. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	FRotator BreakSplitEndRotLeft = FRotator(80.f, -15.f, 0.f);
	/** 오른쪽 조각 최종 회전. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	FRotator BreakSplitEndRotRight = FRotator(80.f, 15.f, 0.f);
	/** 두 조각이 좌우로 벌어지는 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	float BreakSplitSpreadY = 40.0f;

	/** 롱 노트 시각 길이 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float HoldVisualScale = 0.5f;

	/** 장애물 전용 롱 길이 추가 배율 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float ObstacleHoldScale = 0.5f;

	/** 액션별 장애물 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstacleScaleByAction;

	/** 액션별 장애물 피벗 보정. 메시 원점이 바닥중앙이 아닐 때 눈으로 맞춤. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstaclePivotOffsetByAction;

	/** 액션별 롱노트 장애물 축별 길이 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstacleHoldLengthScaleByAction;

	/** E(별) 장애물 제자리 회전 속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	float StarSpinDegPerSec = 120.0f;

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

	/** 액션별 실패 파괴 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TMap<EPTBActionType, TObjectPtr<UNiagaraSystem>> BreakVFXByAction;

	// ── broken 메시 넘어짐 연출(A·B 실패) ──
	/** 넘어지는 각도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	float BreakFallAngleDeg = 85.0f;
	/** 넘어지는 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	float BreakFallDuration = 0.45f;
	/** 넘어진 뒤 사라지는 시간(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	float BreakFadeDuration = 0.25f;
	/** broken 메시가 뒤로 흘러 사라질 거리(화면 밖). 스크롤 속도로 이동. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	float BreakScrollDespawnDistance = 3500.0f;

	/** A 롱 성공 시 물첨벙 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> SplashVFX;

	/** B 성공 시 슬라이드 먼지 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> SlideDustVFX;

	// ── D 발차기 런치 파라미터 ────────────────
	/** 발차기 중력. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickGravity = 2000.0f;

	/** 단일 노트 성공 연출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickSpeedSingle = 900.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickAngleSingle = 15.0f;

	/** 롱노트 성공 연출. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickSpeedLong = 1400.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickAngleLong = 40.0f;

	/** 실패 연출(빗맞음). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickFailSpeed = 450.0f;

	/** 성공 공 수명(초) 후 사라짐. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickLifetime = 1.2f;
	/** 실패 공 수명(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickFailLifetime = 5.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	float KickRestitution = 0.4f;

	/** 발차기 SFX / 단일·롱 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	FName KickSFXKey = NAME_None;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	TObjectPtr<UNiagaraSystem> KickVFX;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Kick")
	TObjectPtr<UNiagaraSystem> KickVFXLong;

	// ── 기타 ────────────────────────────────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float DogCueMaxBeats = 0.5f;

	/** 마커 스폰 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float MarkerSpawnDistance = 1200.0f;

	/** 난이도별 접근 속도(LookAheadBeats). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	TMap<EPTBDifficulty, float> LookAheadBeatsByDifficulty;

	// ── 난이도별 속도 동기화 ──
	/** 스크롤 1배(기본 ScrollSpeed) 기준 LookAheadBeats. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float ScrollSyncBaseLookAheadBeats = 10.0f;
	/** 배경 전용 속도 계수. 최종 배경속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float BackgroundScrollMultiplier = 1.0f;
	/** 캐릭터 모션 완충 계수. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float AnimSpeedInfluence = 0.3f;
};
