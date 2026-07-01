#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBDWMiniGame.generated.h"

class UPTBDWMiniGameRuleSet;
class UStaticMesh;
class APTBDWCharacter;
class UNiagaraSystem;

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
};

/** 실패 시 분리되어 을 도는 장애물. */
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
};


UCLASS()
class PARTTIMEBEAT_API APTBDWMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void StartMiniGame() override;
	virtual void BuildRuntimeState() override;
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

	const UPTBDWMiniGameRuleSet* GetDWRuleSet() const;

private:
	void SpawnNoteView(const FPTBNoteEvent& Note);
	void RecycleNoteView(int32 NoteId);
	void ClearAllNoteViews();

	/** 실패 장애물을 분리해 부서진 메시 교체/placeholder. */
	void ResolveObstacleFail(AActor* Obstacle, EPTBActionType Action);

	/** 부서진 메시가 없을 때 placeholder. */
	void SpawnSplitHalves(AActor* Obstacle, EPTBActionType Action);

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
};
