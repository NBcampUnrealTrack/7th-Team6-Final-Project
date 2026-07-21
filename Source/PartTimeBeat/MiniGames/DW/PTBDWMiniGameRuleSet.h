#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "Templates/SubclassOf.h"
#include "Core/PTBStructEnums.h"
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

	// ── 기타 ────────────────────────────────────────────────
	/** 마커 스폰 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float MarkerSpawnDistance = 2000.0f;

	/** 난이도별 접근 속도(LookAheadBeats). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	TMap<EPTBDifficulty, float> LookAheadBeatsByDifficulty;


	// ── 난이도별 속도 동기화 ──────────────────────────────────
	/** 스크롤 1배(기본 ScrollSpeed) 기준 LookAheadBeats. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float ScrollSyncBaseLookAheadBeats = 8.0f;

	/** 배경 전용 속도 계수. 최종 배경속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float BackgroundScrollMultiplier = 1.0f;

	/** 캐릭터 모션 완충 계수.(Insane난이도 애님 배속) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW")
	float AnimSpeedInfluence = 0.3f;


	// ── 장애물 ──────────────────────────────────────────────
	/** 액션별 장애물 선행 시간. 노트보다 몇 ms 앞세울지. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, float> ObstacleLeadMsByAction;

	/** 롱노트 전용 장애물 선행 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, float> ObstacleLongLeadMsByAction;

	/** 롱 전용, 실패 시 장애물 연출 지연. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, float> ObstacleLongFailBreakDelayMsByAction;

	// 액션별 장애물 색. (제거해도 됨/이제 원본 머티리얼 씀)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FLinearColor> ObstacleColorByAction;

	/** 액션별 장애물 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> ObstacleMeshByAction;

	/** 롱 노트 시각 길이 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float HoldVisualScale = 1.0f;

	/** 액션별 장애물 스케일. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstacleScaleByAction;

	/** 액션별 장애물 피벗 보정. 메시 원점이 바닥중앙이 아닐 때 눈으로 맞춤. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstaclePivotOffsetByAction;

	/** 액션별 롱노트 장애물 scale 배율. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TMap<EPTBActionType, FVector> ObstacleHoldLengthScaleByAction;

	/** 장애물 색용 베이스 머티리얼.(제거해도 됨/이제 안씀) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle")
	TObjectPtr<UMaterialInterface> ObstacleMaterial;


	// ── A 실패 2조각 갈라짐 ───────────────────────────────
	/** 왼쪽 조각 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> BrokenPieceLeftByAction;

	/** 오른쪽 조각 메시. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	TMap<EPTBActionType, TObjectPtr<UStaticMesh>> BrokenPieceRightByAction;

	/** 왼쪽 조각 최종 회전. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	FRotator BreakSplitEndRotLeft = FRotator(0.f, -80.f, -20.f);

	/** 오른쪽 조각 최종 회전. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	FRotator BreakSplitEndRotRight = FRotator(0.f, -80.f, 20.f);

	/** 두 조각이 좌우로 벌어지는 거리. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	float BreakSplitSpreadY = 10.0f;

	/** broken 조각 파묻힘 보정용. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	float BreakBrokenGroundLiftZ = 20.0f;

	/** 넘어지는 시간. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	float BreakFallDuration = 0.1f;

	/** broken 메시가 뒤로 흘러 사라질 거리(화면 밖으로). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActA")
	float BreakScrollDespawnDistance = 3500.0f;


	// ── B 실패 런치 파라미터 ────────────────
	/** 장애물 날아가는 방향. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActB")
	FVector ActBFailLaunchDir = FVector(-1.0f, 0.0f, 0.5f);

	/** 장애물 날아가는 속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActB", meta = (ClampMin = "0.0"))
	float ActBFailLaunchSpeed = 700.0f;

	/** 장애물 회전. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActB")
	float ActBFailSpinDegPerSec = 540.0f;

	/** 장애물 수명(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActB", meta = (ClampMin = "0.1"))
	float ActBFailLifetime = 4.0f;

	/** 장애물 탄성계수. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActB", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ActBFailRestitution = 0.35f;


	// ── D 발차기 런치 파라미터 ────────────────
	/** 발차기 중력. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDGravity = 500.0f;

	/** 단일 노트 성공 연출.(속도/각도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDSpeedSingle = 1200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDAngleSingle = 15.0f;

	/** 롱노트 성공 연출.(속도/각도) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDSpeedLong = 1600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDAngleLong = 40.0f;

	/** 실패 연출(빗맞음). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDFailSpeed = 600.0f;

	/** 성공 공 수명(초) 후 사라짐. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDLifetime = 1.2f;
	/** 실패 공 수명(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDFailLifetime = 2.0f;
	/** 공 탄성계수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActD")
	float ActDRestitution = 0.4f;


	// ── E 별 회전 ────────────────
	/** ActionE(별) 장애물 제자리 회전 속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Obstacle|ActE")
	float ActESpinDegPerSec = 120.0f;


	// ── Actors(개/사람) ────────────────────────────────────
	/** Judge 액터 스폰 오프셋. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Judge")
	FVector JudgeSpawnOffset = FVector(0.0f, 0.0f, 0.0f);

	/** Judge 액터 Mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Judge")
	TObjectPtr<USkeletalMesh> JudgeMesh;

	/** Judge 액터 Mesh Scale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Judge")
	FVector JudgeScale = FVector(1.5f);

	/** Judge 액터 방향 보정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Judge")
	float JudgeYaw = -90.0f;
	 
	/** Preview 액터 스폰 오프셋. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Preview")
	FVector PreviewSpawnOffset = FVector(400.0f, -180.0f, -100.0f);

	/** Preview 액터 Mesh(skeleton 공유라 메시만 교체(AnimClass/RunAnim 공용)). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Preview")
	TMap<EPTBGender, TObjectPtr<USkeletalMesh>> PreviewMeshByGender;

	/** Preview 액터 Mesh Scale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Preview")
	FVector PreviewScale = FVector(1.3f);

	/** Preview 액터 방향 보정. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Actors|Preview")
	float PreviewYaw = -90.0f;


	// ── 액터 애니메이션 ───────────────────────────────────
	// ── Judge 액터 ───────────────────────────────────
	/** Judge 액터 AnimBP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TSubclassOf<UAnimInstance> JudgeAnimClass;

	/** Judge 액터(개) 달리기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> JudgeRunAnim;

	/** Action A 성공 애님. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> ActAAnim;

	/** Action A 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.01"))
	float ActAAnimPlayRate = 1.0f;

	/** Action B 성공 애님. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> ActBAnim;

	/** Action B 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.01"))
	float ActBAnimPlayRate = 1.0f;

	/** Action B/E 실패 애님. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> JudgeFailAnim;

	/** Action B/E 실패 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.01"))
	float JudgeFailAnimPlayRate = 1.0f;

	/** Action D 성공/실패 애님. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> ActDAnim;

	/** Action D 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.01"))
	float ActDAnimPlayRate = 1.0f;

	/** Action E 성공 애님. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	TObjectPtr<UAnimSequenceBase> ActEAnim;

	/** Action E 성공 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.01"))
	float ActEAnimPlayRate = 1.0f;

	/** Judge 액터 리액션 상체 Blend 슬롯 이름. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge")
	FName ReactionUpperSlotName = FName(TEXT("UpperBody"));

	/** 리액션 몽타주 blend-in(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.0"))
	float ReactionBlendIn = 0.06f;

	/** 리액션 몽타주 blend-out(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Judge", meta = (ClampMin = "0.0"))
	float ReactionBlendOut = 0.06f;


	// ── Preview 액터 ───────────────────────────────────
	/** Preview 액터 AnimBP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview")
	TSubclassOf<UAnimInstance> PreviewAnimClass;

	/** Preview 액터(사람) 달리기. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview")
	TObjectPtr<UAnimSequenceBase> PreviewRunAnim;

	/** Preview 액터 소리치기 애님(Cue마다 상체 슬롯 재생). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview")
	TObjectPtr<UAnimSequenceBase> PreviewShoutAnim;

	/** Preview 액터 리액션 상체 Blend 슬롯 이름. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview")
	FName PreviewShoutSlotName = FName(TEXT("UpperBody"));

	/** 소리치기 blend-in 시간(초). 연속 Cue 자연스럽게 튜닝. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview", meta = (ClampMin = "0"))
	float PreviewShoutBlendInSec = 0.2f;

	/** 소리치기 blend-out 시간(초). 단발 Cue 후 달리기 복귀 튜닝. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview", meta = (ClampMin = "0"))
	float PreviewShoutBlendOutSec = 0.25f;

	/** 소리치기 애님 배속. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Anim|Preview", meta = (ClampMin = "0.01"))
	float PreviewShoutPlayRate = 1.0f;


	// ── 노트 토큰(변경된 방식) ───────────────────────────────────
	/** Action별 노트 색상. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	TMap<EPTBActionType, FLinearColor> NoteColorByAction;

	/** 단일 노트 Scale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	FVector NoteSingleScale = FVector(0.35f, 0.35f, 0.35f);

	/** 롱 노트 Scale. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	FVector NoteLongScale = FVector(0.5f, 0.5f, 0.5f);

	/** 도착점(끝) 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	FVector NoteTargetHeadOffset = FVector(10.0f, 0.0f, 150.0f);

	/** 생성점(시작) 오프셋 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	FVector NoteCueSpawnOffset = FVector(100.0f, 0.0f, 250.0f);

	/** 예고 리드인(beat). 이 박자 전에 음표 생성. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note", meta = (ClampMin = "1.0"))
	float NoteLeadInBeats = 4.0f;

	/** 시각 동기 보정. 노트 그림이 판정보다 늦게 도착하는 lag(smoothed vs raw 재생시계 차)을 상쇄. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note", meta = (ClampMin = "-100.0", ClampMax = "100.0"))
	float VisualSyncOffsetMs = 30.0f;

	/** 음표 포물선 아치 높이. beat마다 튀어오르는 높이. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	float NoteArcHeight = 120.0f;

	/** 단일 노트 Mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	TObjectPtr<UStaticMesh> NoteSingleMesh;

	/** 롱 노트 Mesh. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	TObjectPtr<UStaticMesh> NoteLongMesh;

	/** 노트 Mesh의 Material. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	TObjectPtr<UMaterialInterface> NoteMaterial;

	/** 롱 노트 Hold 발광 기본 강도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	float NoteHoldEmissiveBase = 1.0f;

	/** 롱 노트 Hold 발광 진폭. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	float NoteHoldEmissiveAmp = 0.3f;

	/** 롱 노트 Hold 발광 속도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Note")
	float NoteHoldEmissivePulseSpeed = 6.0f;


	// ── 연출 VFX (Niagara) ──────────────────────────────────
	/** 성공 시 음표 토큰 위치에서 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> NoteDespawnVFX;

	/** 롱노트 혜성 꼬리 리본(Niagara). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> NoteTailRibbonVFX;

	/** Action E 장애물 성공 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> ActESuccessVFX;

	/** Action E 장애물 실패 VFX. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|VFX")
	TObjectPtr<UNiagaraSystem> ActEFailVFX;


	// ── 판정 TextPopUp(PERFECT, GOOD, MISS), 간격 등은 BP_DW_TextPopup ──────────────
	/** 판정 텍스트 팝업 Actor 클래스. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Text")
	TSubclassOf<AActor> JudgePopupClass;

	/** 판정 텍스트 표시 여부. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Text")
	bool bShowJudgeText = true;

	/** 판정 텍스트 위치 오프셋(월드). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Text")
	FVector JudgeTextOffset = FVector(0.0f, 0.0f, 250.0f);

	/** 팝업 회전(카메라 정면 정렬용). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Text")
	FRotator JudgeTextRotation = FRotator::ZeroRotator;


	// ── 실패 시 카메라 흔들림 ────────────────────────────
	/** 카메라 흔들림 강도. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Camera|Fail", meta = (ClampMin = "0"))
	float FailCameraShakeIntensity = 0.15f;

	/** 카메라 흔들림 지속시간(초). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Camera|Fail", meta = (ClampMin = "0"))
	float FailCameraShakeDuration = 0.3f;

	/** 카메라 흔들림 주파수(초당 진동수). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|DW|Camera|Fail", meta = (ClampMin = "0.1"))
	float FailCameraShakeFrequency = 15.0f;
};
