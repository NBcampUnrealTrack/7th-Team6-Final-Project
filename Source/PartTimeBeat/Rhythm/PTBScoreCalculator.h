#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBScoreCalculator.generated.h"

UCLASS()
class PARTTIMEBEAT_API UPTBScoreCalculator : public UObject
{
	GENERATED_BODY()
	
public:
	//FPTBScoreRule ScoreRule;// 판정별 점수 / 랭크 기준
	/** 실시간 점수 */
	int32 CurrentScore;
	/**	현재 콤보 */
	int32 ComboCount;
	/** 최대 콤보 */
	int32 MaxCombo;
	int32 HighPerfectCount;
	int32 PerfectCount; 
	int32 GoodCount;
	int32 MissCount;
	/** 채보 전체 노트 수 */
	int32 TotalNoteCount;

	//	점수 + 콤보 반영, 증가량 반환
	int32 AddJudgementScore(const FPTBJudgementResult& Result);
	//	최종 결과 생성
	FPTBRoundResult BuildRoundResult(const FString& ProfileId, FName MiniGameId,
			EPTBDifficulty Difficulty, const FPTBMiniGameResultPayload& Payload) const;
	//	별점 계산(0~3)
	int32 CalculateStarRating(int32 Score) const;
	//	등급 계산
	EPTBGradeType CalculateGrade() const;
	//	초기화
	void Reset();
};
