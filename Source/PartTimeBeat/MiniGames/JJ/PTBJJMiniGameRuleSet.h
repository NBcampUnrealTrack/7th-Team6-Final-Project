#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/PTBMiniGameRuleSet.h"
#include "PTBJJMiniGameRuleSet.generated.h"

/**
 * 공용 리듬 코어를 점프/착지 연출로 해석하는 JumpJump 규칙을 정의하는 RuleSet DataAsset입니다
 *
 * 채보 노트를 좌/중/우 캐릭터 점프로 매핑하고, 판정 결과를 착지 피드백으로 연출하기 위한 런타임 규칙을 제공합니다
 */
UCLASS(BlueprintType)
class PARTTIMEBEAT_API UPTBJJMiniGameRuleSet : public UPTBMiniGameRuleSet
{
	GENERATED_BODY()

public:
	/** Action → 캐릭터 인덱스(좌0 / 중1 / 우2) 매핑 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TMap<EPTBActionType, int32> ActionToCharacterIndex;

	/** 캐릭터별 기본 점프 속도 배율(좌/중/우) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	TArray<float> DefaultJumpSpeeds;

	/** 착지 피드백 허용 시간(ms) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump", meta = (ClampMin = "0", UIMin = "0"))
	int32 LandingWindowMs = 120;

	/** 변속(점프 속도 가변) 구간 사용 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	bool bUseVariableJumpSpeed = false;

	/** 점프 SFX 이벤트 키(Play_SFX_JJ_Jump) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	FName JumpSFXKey = NAME_None;

	/** 착지 성공 SFX 이벤트 키(Play_SFX_JJ_Land) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	FName LandSFXKey = NAME_None;

	/** 착지 실패 SFX 이벤트 키(Play_SFX_JJ_Fail) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	FName FailJumpSFXKey = NAME_None;

	/** 노트 디버그 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	bool bLogNoteEvent = false;

	/** 판정 디버그 로그 출력 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	bool bLogJudgement = false;

	/** 결과 Payload에 JJ 통계 포함 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|JumpJump")
	bool bIncludeDebugPayload = true;

	/** Action → 캐릭터 인덱스 조회(매핑 없으면 좌0 기본) */
	int32 ResolveCharacterIndex(EPTBActionType Action) const;
};