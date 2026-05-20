#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBScoreCalculator.generated.h"

/**
 * 판정 결과를 누적해 실시간 점수와 최종 라운드 결과를 생성하는 점수 계산기입니다.
 *
 * 이 클래스는 입력 판정 자체를 수행하지 않고 JudgementSystem이 만든 FPTBJudgementResult를 기반으로 점수, 콤보, 등급, 별점을 계산합니다.
 * 저장, 최고 기록 갱신, 보상 지급은 이 클래스 밖의 진행도 시스템에서 처리합니다.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBScoreCalculator : public UObject
{
	GENERATED_BODY()
	
public:
	/** 실시간 점수 */
	int32 CurrentScore = 0;

	/** 현재 콤보 */
	int32 ComboCount = 0;

	/** 최대 콤보 */
	int32 MaxCombo = 0;

	/** High Perfect 누적 수 */
	int32 HighPerfectCount = 0;

	/** Perfect 누적 수 */
	int32 PerfectCount = 0;

	/** Good 누적 수 */
	int32 GoodCount = 0;

	/** Miss 누적 수 */
	int32 MissCount = 0;

	/** 누적 판정 노트 수 */
	int32 TotalNoteCount = 0;

	/** 점수와 콤보 반영 후 증가 점수 반환 */
	int32 AddJudgementScore(const FPTBJudgementResult& Result);
	
	/** 최종 라운드 결과 생성 */
	FPTBRoundResult BuildRoundResult(const FString& ProfileId, FName MiniGameId,
			EPTBDifficulty Difficulty, const FPTBMiniGameResultPayload& Payload) const;
	
	/** 별점 계산 */
	int32 CalculateStarRating(int32 Score) const;
	
	/** 등급 계산 */
	EPTBGradeType CalculateGrade() const;
	
	/** 누적 상태 초기화 */
	void Reset();
};
