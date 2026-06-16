#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/PTBStructEnums.h"
#include "PTBMiniGameRuleSet.generated.h"

class UPTBRhythmChartAsset;
class UPTBWwiseEventMapAsset;

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
	virtual void PostLoad() override;

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

	/** 난이도별 채보 Asset */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Chart")
	TMap<EPTBDifficulty, TObjectPtr<UPTBRhythmChartAsset>> ChartAssetsByDifficulty;

	/** Wwise 이벤트 매핑 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Audio")
	TObjectPtr<UPTBWwiseEventMapAsset> AudioEventSet;

	/** 시작 연출 시간을 사용할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Intro/Outro")
	bool bUseIntroTime = false;

	/** 시작 연출 시간(ms). 이 시간이 지난 뒤 Audio와 Chart가 시작됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Intro/Outro", meta = (EditCondition = "bUseIntroTime", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float IntroTimeMs = 0.0f;

	/** 종료 연출 시간을 사용할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Intro/Outro")
	bool bUseOutroTime = false;

	/** 종료 연출 시간(ms). 이 시간이 지난 뒤 결과 전달과 맵 전환이 시작됩니다. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Intro/Outro", meta = (EditCondition = "bUseOutroTime", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float OutroTimeMs = 0.0f;

	/** 비주얼 큐 선행 시간 기준 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing")
	EPTBCueLeadTimeMode CueLeadTimeMode = EPTBCueLeadTimeMode::Beat;

	/** 비주얼 큐 선행 Beat 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing", meta = (ClampMin = "0", UIMin = "0"))
	float LookAheadBeats = 2.0f;

	/** 비주얼 큐 선행 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing", meta = (ClampMin = "0", UIMin = "0"))
	float CueLeadTimeMs = 2000.0f;

	/** 판정 등록 선행 시간 override 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing")
	bool bUseArmLeadTimeOverride = false;

	/** 판정 등록 선행 시간 override(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Timing", meta = (EditCondition = "bUseArmLeadTimeOverride", ClampMin = "0", UIMin = "0"))
	float ArmLeadTimeMsOverride = 120.0f;

	/** 판정 범위 override 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement")
	bool bUseJudgementWindowOverride = false;

	/** High Perfect 판정 허용 범위(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement", meta = (EditCondition = "bUseJudgementWindowOverride", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float HitWindowHighPerfectMsOverride = 21.0f;

	/** Perfect 판정 허용 범위(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement", meta = (EditCondition = "bUseJudgementWindowOverride", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float HitWindowPerfectMsOverride = 50.0f;

	/** Good 판정 허용 범위(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement", meta = (EditCondition = "bUseJudgementWindowOverride", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float HitWindowGoodMsOverride = 120.0f;

	/** Miss 입력 소비 허용 범위(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement", meta = (EditCondition = "bUseJudgementWindowOverride", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float HitWindowMissMsOverride = 250.0f;

	/** 노트 판정 범위에서 다른 Action 입력 시 해당 노트를 Miss 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement")
	bool bMissNoteOnWrongInput = false;

	/** 같은 타이밍의 여러 Action 노트 동시 입력 지원. false면 오입력 Miss가 동시노트 그룹을 임의 소비하지 않음 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement")
	bool bSupportsSimultaneousInputs = true;

	/** 동시노트로 간주할 시간 차(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Judgement", meta = (ClampMin = "0", UIMin = "0"))
	float SimultaneousNoteToleranceMs = 1.0f;

	/** 초보모드에서 미스 제한으로 실패 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "초보모드 미스 제한 실패"))
	bool bFailOnMissLimitEasy = false;

	/** 초보모드 실패 처리 최대 미스 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "초보모드 최대 미스 수", EditCondition = "bFailOnMissLimitEasy", EditConditionHides, ClampMin = "1", UIMin = "1"))
	int32 MaxMissCountEasy = 10;

	/** 통상모드에서 미스 제한으로 실패 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "통상모드 미스 제한 실패"))
	bool bFailOnMissLimitStandard = false;

	/** 통상모드 실패 처리 최대 미스 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "통상모드 최대 미스 수", EditCondition = "bFailOnMissLimitStandard", EditConditionHides, ClampMin = "1", UIMin = "1"))
	int32 MaxMissCountStandard = 10;

	/** 발광모드에서 미스 제한으로 실패 처리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "발광모드 미스 제한 실패"))
	bool bFailOnMissLimitInsane = false;

	/** 발광모드 실패 처리 최대 미스 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Failure", meta = (DisplayName = "발광모드 최대 미스 수", EditCondition = "bFailOnMissLimitInsane", EditConditionHides, ClampMin = "1", UIMin = "1"))
	int32 MaxMissCountInsane = 10;

	/** 헛입력 처리 방식 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Empty Input Policy")
	EPTBEmptyInputPolicy EmptyInputPolicy = EPTBEmptyInputPolicy::Ignore;

	/** 헛입력 시 해당 Action 잠금 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Empty Input Policy")
	bool bUseEmptyInputActionLock = false;

	/** 헛입력 Action 잠금 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Empty Input Policy", meta = (EditCondition = "bUseEmptyInputActionLock", EditConditionHides, ClampMin = "0", UIMin = "0"))
	float EmptyInputActionLockMs = 0.0f;

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

	/** 난이도와 미스 수 기준 실패 여부 */
	bool ShouldFailForMissCount(EPTBDifficulty Difficulty, int32 MissCount) const;

	/** 헛입력을 Miss로 처리할지 여부 */
	bool ShouldTreatEmptyInputAsMiss() const;

	/** 헛입력 시 Action을 잠글지 여부 */
	bool ShouldLockActionOnEmptyInput() const;

private:
	/** 레거시 공통 미스 제한 실패 여부, 기존 DataAsset 마이그레이션용 */
	UPROPERTY()
	bool bFailOnMissLimit = false;

	/** 레거시 공통 최대 미스 수, 기존 DataAsset 마이그레이션용 */
	UPROPERTY()
	int32 MaxMissCount = 10;

	/** 레거시 미스 제한 설정을 난이도별 필드로 옮겼는지 여부 */
	UPROPERTY()
	bool bHasMigratedDifficultyMissLimitSettings = false;
};
