#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBPCMiniGameRuleSet.generated.h"

/**
 * 
 */
UCLASS()
class PARTTIMEBEAT_API UPTBPCMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/** 미니게임 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FName MiniGameId = "PC";

	/** 미니게임 코드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FName MiniGameCode = "Temporary";

	/** UI 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FText DisplayName;

	/** 지원하는 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Rule")
	TArray<EPTBActionType> SupportedActions;

	/** 액션별 의미 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Rule")
	TMap<EPTBActionType, FName> ActionTags;

	/** 난이도별 채보 Asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Chart")
	TMap<EPTBDifficulty, TObjectPtr<UPTBRhythmChartAsset>> ChartAssetsByDifficulty;

	/** 비주얼 큐 선행 Beat 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing", meta = (ClampMin = "0", UIMin = "0"))
	float LookAheadBeats = 2.0f;

	/** 판정 등록 선행 시간 override 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing")
	bool bUseArmLeadTimeOverride = false;

	/** 판정 등록 선행 시간 override(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing", meta = (ClampMin = "0", UIMin = "0"))
	float ArmLeadTimeMsOverride = 120.0f;

	/** 미스 제한으로 실패 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure")
	bool bFailOnMissLimit = false;

	/** 헛입력 처리 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure")
	EPTBEmptyInputPolicy EmptyInputPolicy = EPTBEmptyInputPolicy::Ignore;

	/** 헛입력 시 해당 Action 잠금 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure")
	bool bUseEmptyInputActionLock = false;

	/** 헛입력 Action 잠금 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (ClampMin = "0", UIMin = "0"))
	float EmptyInputActionLockMs = 0.0f;

	/** 실패 처리할 최대 미스 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (ClampMin = "1", UIMin = "1"))
	int32 MaxMissCount = 10;

	/** 판정별 SFX 이벤트 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TMap<EPTBJudgementType, FName> JudgementSFXKeys;

	/** 기본 성공 SFX 이벤트 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	FName SuccessSFXKey = NAME_None;

	/** 기본 실패 SFX 이벤트 키 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	FName FailSFXKey = NAME_None;

	/** 입력 액션 지원 여부 */
	bool SupportsAction(EPTBActionType Action) const;

	/** 액션 의미 태그 조회 */
	FName GetActionTag(EPTBActionType Action) const;

	/** 난이도별 채보 Asset 조회 */
	UPTBRhythmChartAsset* ResolveChartAsset(EPTBDifficulty Difficulty) const;

	/** 판정 SFX 이벤트 키 조회 */
	FName GetJudgementSFXKey(EPTBJudgementType JudgementType) const;

	/** 미스 수 기준 실패 여부 */
	bool ShouldFailForMissCount(int32 MissCount) const;

	/** 헛입력을 Miss로 처리할지 여부 */
	bool ShouldTreatEmptyInputAsMiss() const;

	/** 헛입력 시 Action을 잠글지 여부 */
	bool ShouldLockActionOnEmptyInput() const;
};
