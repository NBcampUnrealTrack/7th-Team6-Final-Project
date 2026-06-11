#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"
#include "PTBDWMiniGame.generated.h"

class UPTBDWMiniGameRuleSet;
class UStaticMesh;
class APTBDWCharacter;

/** 노트 비주얼 추적 + 음악 시간 보간 상태 (NoteId 기준) */
USTRUCT()
struct FDWNoteView
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AActor> Marker = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> Obstacle = nullptr;

	float SpawnVisualMs = 0.f;
	float TargetMs      = 0.f;
	float InvDuration   = 0.f;
	FVector SpawnPos    = FVector::ZeroVector;
	FVector LinePos     = FVector::ZeroVector;
	FVector ObstacleOffsetVec = FVector::ZeroVector;
	FVector MarkerBaseScale = FVector(1.0f);
};


UCLASS()
class PARTTIMEBEAT_API APTBDWMiniGame : public APTBBaseMiniGame
{
	GENERATED_BODY()

protected:
	virtual void Tick(float DeltaTime) override;
	virtual void StartMiniGame() override;
	virtual void BuildRuntimeState() override;
	virtual void HandleNoteCue(FPTBNoteEvent Note) override;
	virtual void HandleJudgementResult(FPTBJudgementResult Result) override;

	const UPTBDWMiniGameRuleSet* GetDWRuleSet() const;

private:
	void SpawnNoteView(const FPTBNoteEvent& Note);
	void RecycleNoteView(int32 NoteId);
	void ClearAllNoteViews();

	void SpawnCharactersIfNeeded();
	static bool IsSupportedAction(EPTBActionType Action);

	float GetObstacleOffset(EPTBActionType Action) const;
	FLinearColor GetObstacleColor(EPTBActionType Action) const;
	FVector GetObstacleScale(EPTBActionType Action) const;
	UStaticMesh* GetObstacleMesh(EPTBActionType Action);

	UPROPERTY()
	TMap<int32, FDWNoteView> ActiveNoteViews;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CachedObstacleMesh = nullptr;

	UPROPERTY()
	TObjectPtr<APTBDWCharacter> Dog = nullptr;

	UPROPERTY()
	TObjectPtr<APTBDWCharacter> Protagonist = nullptr;
};
