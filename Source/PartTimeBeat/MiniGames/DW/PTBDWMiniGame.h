#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBDWMiniGame.generated.h"

class UPTBDWMiniGameRuleSet;
class UStaticMesh;
class APTBDWCharacter;
class APTBDWBackgroundScroller;
class APTBDWCameraRig;
class UNiagaraSystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDWOnComboChanged, int32, NewCombo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDWOnNoteResolved, bool, bSuccess, int32, Combo);

/** 노트 비주얼 추적 + 음악 시간 보간 상태 (NoteId 기준) */
USTRUCT()
struct FDWNoteView
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> Marker = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> Obstacle = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> TailMarker = nullptr;

	float SpawnVisualMs = 0.f;
	float TargetMs      = 0.f;
	float ReleaseMs     = 0.f;
	float InvDuration   = 0.f;
	FVector SpawnPos    = FVector::ZeroVector;
	FVector LinePos     = FVector::ZeroVector;
	FVector ObstacleOffsetVec = FVector::ZeroVector;
	FVector ObstaclePivotWorld = FVector::ZeroVector;

	float MarkerSquareSize = 0.6f;
	float MarkerThinX      = 0.08f;
	float MarkerFlatZ      = 0.05f;
	float TailLengthCm     = 0.f;

	/** 판정 결과 연출 분기용. */
	EPTBActionType Action = EPTBActionType::None;

	/** 롱 노트 여부(연출 분기 + 길이 비례 장애물). */
	bool bIsLong = false;

	/** 성공 판정 후 꼬리가 다 소모될 때까지 살려두는 플래그. */
	bool bJudgedSuccess = false;

	/** 판정(성공·실패) 발생 여부. 롱노트는 꼬리 100% 후 이 결과로 연출. */
	bool bJudged = false;
};

/** 실패 시 분리되어 넘어지는 장애물. */
USTRUCT()
struct FDWResolvingObstacle
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> Obstacle = nullptr;

	float Timer = 0.f;
	float Duration = 0.f;
	FVector StartLoc = FVector::ZeroVector;
	FVector StartScale = FVector(1.0f);
	FRotator StartRot = FRotator::ZeroRotator;

	FVector SideDir = FVector::ZeroVector;
	FVector BackDir = FVector::ZeroVector;
	FVector RightAxis = FVector::ZeroVector;
	bool bSplitPiece = false;

	FRotator EndRot = FRotator::ZeroRotator;
	float FallFrac = 0.65f;
	float BackVel = 0.f;
	float SpreadY = 0.f;

	bool bLaunch = false;
	FVector Vel = FVector::ZeroVector;
	float GroundZ = 0.f;
	float Restitution = 0.4f;
	float SpinDegPerSec = 0.f;
};


UCLASS()
class PARTTIMEBEAT_API APTBDWMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

public:
	/** 콤보 숫자 갱신(판정 즉시). BP 위젯이 바인드해 숫자 표시. */
	UPROPERTY(BlueprintAssignable, Category = "PTB|DW|Combo")
	FDWOnComboChanged OnDWComboChanged;

	/** 노트 해결 시(탭=즉시, 롱=꼬리 100%). 콤보 연출용(bSuccess, 현재콤보). */
	UPROPERTY(BlueprintAssignable, Category = "PTB|DW|Combo")
	FDWOnNoteResolved OnDWNoteResolved;

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StartMiniGame() override;
	virtual void BuildRuntimeState() override;
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;
	virtual void ReceiveGameplayStarted_Implementation() override;
	virtual void ReceiveIntroStarted_Implementation() override;

	const UPTBDWMiniGameRuleSet* GetDWRuleSet() const;

private:
	void SpawnNoteView(const FPTBNoteEvent& Note);
	void RecycleNoteView(int32 NoteId);
	void ClearAllNoteViews();

	/** 실패 장애물을 분리해 부서진 메시 교체/placeholder. */
	void ResolveObstacleFail(AActor* Obstacle, EPTBActionType Action);
	/** 성공 시 obstacle을 부수지 않고 온전히 배경 속도로 뒤로 흘려보냄(A·B 허들 넘기). */
	void ScrollObstacleAway(AActor* Obstacle);
	/** 노트 접근 속도(cm/s) = MarkerSpawnDistance / LookAheadMs. 나가는 obstacle도 이 속도로(올 때=나갈 때 일치). */
	float GetNoteApproachSpeedCmS() const;
	void LaunchKickBall(AActor* Ball, bool bSuccess, bool bIsLong);
	void PlayNoteResultEffects(int32 NoteId, EPTBActionType Action, bool bSuccess, bool bIsLong);

	/** 부서진 메시가 없을 때 placeholder. */
	void SpawnSplitHalves(AActor* Obstacle, EPTBActionType Action);
	/** 지정 좌/우 조각 2개를 피벗 기준으로 V자 갈라지게 넘어뜨리고 뒤로 흘림(A·B 실패). */
	void SpawnBrokenPieces(AActor* Obstacle, EPTBActionType Action);

	/** 판정선에 고정 타깃 바를 1회 생성. */
	void SpawnJudgeTargetIfNeeded();

	void SpawnCharactersIfNeeded();
	bool IsSupportedAction(EPTBActionType Action) const;

	/** 연출 SFX 요청(다이제틱). 판정 등급음은 베이스가 처리. */
	void RequestDWSfx(FName Key, AActor* Target);

	/** 연출 VFX 요청(Niagara). */
	void RequestDWVfx(UNiagaraSystem* System, const FVector& Location);

	/** 시작 시 DataAsset 설정 자가 점검. */
	void ValidateDWConfig() const;

	float GetObstacleOffset(EPTBActionType Action) const;
	FLinearColor GetObstacleColor(EPTBActionType Action) const;
	FVector GetObstacleScale(EPTBActionType Action) const;
	UStaticMesh* GetObstacleMesh(EPTBActionType Action);

	UPROPERTY()
	TMap<int32, FDWNoteView> ActiveNoteViews;

	/** 현재 난이도에서 확정된 LookAheadBeats (마커 정속 진입 계산용). BuildRuntimeState에서 설정. */
	float ActiveLookAheadBeats = 2.0f;
	/** 난이도 속도 배율(= 기준LookAhead / ActiveLookAhead). BuildRuntimeState에서 계산. */
	float ActiveSpeedScale = 1.0f;

	UPROPERTY()
	TArray<FDWResolvingObstacle> ResolvingObstacles;

	UPROPERTY()
	TObjectPtr<AActor> JudgeTargetBar = nullptr;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CachedObstacleMesh = nullptr;

	UPROPERTY()
	TObjectPtr<APTBDWCharacter> Dog = nullptr;

	UPROPERTY()
	TObjectPtr<APTBDWCharacter> Protagonist = nullptr;

	/** DW 전용 배경 스크롤러. StartMiniGame에서 탐색·캐시, 게임플레이 시작 시 SetRunning(true). */
	UPROPERTY()
	TObjectPtr<APTBDWBackgroundScroller> BackgroundScroller = nullptr;
	TObjectPtr<APTBDWCameraRig> CameraRig = nullptr;
};
