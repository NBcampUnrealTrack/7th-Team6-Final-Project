#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBMiniGameRuleSet.generated.h"

/**
 * 공통 미니게임 라운드에서 채보 노트를 각 미니게임 규칙으로 해석하기 위한 기본 RuleSet DataAsset입니다
 *
 * 이 Asset은 채보 시간, BPM, 노트 배열을 소유하지 않고 미니게임별 입력 해석, 실패 조건, 피드백 SFX 같은 런타임 규칙만 제공합니다
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBMiniGameRuleSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** 미니게임 식별자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FName MiniGameId = NAME_None;

	/** 미니게임 코드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FName MiniGameCode = NAME_None;

	/** UI 표시 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|MiniGame")
	FText DisplayName;

	/** 지원하는 입력 액션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Rule")
	TArray<EPTBActionType> SupportedActions;

	/** 액션별 의미 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Rule")
	TMap<EPTBActionType, FName> ActionTags;

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

	/** 판정 SFX 이벤트 키 조회 */
	FName GetJudgementSFXKey(EPTBJudgementType JudgementType) const;

	/** 미스 수 기준 실패 여부 */
	bool ShouldFailForMissCount(int32 MissCount) const;
};
