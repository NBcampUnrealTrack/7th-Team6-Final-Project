#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/PTBStructEnums.h"
#include "PTBScoreCalculator.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreComboBreak, int32, FinalCombo);

/**
 * 판정 결과를 누적해 실시간 점수와 최종 라운드 결과를 생성하는 점수 계산기입니다.
 *
 * 점수 체계 (11/17 결정):
 *  - 100만점 정규화: 점수 = floor(1,000,000 × 누적 가중치 ÷ (총 노트 수 × 최대 가중치))
 *    → 채보 길이와 상관없이 전부 Perfect면 정확히 1,000,000점
 *  - High Perfect = Perfect (가중치 동일). High Perfect는 개수·MAX 표시로만 구분
 *  - 콤보 점수 없음 (콤보 수는 연출/기록용으로만 계속 셈)
 *  - 정확도 = 점수 ÷ 1,000,000 (점수와 정확도가 같은 기준)
 *  - % 표시 = 점수 ÷ 10,000, 소수점 2자리 버림 (965,239점 → 96.52%)
 *
 * 총 노트 수는 플레이 시작 시 SetExpectedNoteCount로 받는다.
 * 실제 판정 수가 더 많으면(빈 입력 Miss 등) 판정 수를 분모로 써서 100만을 넘지 않게 한다.
 *
 * 골드(EarnedMoney)와 별(StarCount)은 아직 기준 미확정이라 예전 방식(판정 기본점수 × 콤보 배율 누적)을 그대로 유지.
 */
UCLASS()
class PARTTIMEBEAT_API UPTBScoreCalculator : public UObject
{
	GENERATED_BODY()

public:
	/** 만점 */
	static constexpr int32 MaxScore = 1000000;

	/** 판정별 가중치 (정수: 오차 없이 계산). Perfect = High Perfect = 최대 가중치 */
	static constexpr int32 WeightPerfect = 10;
	static constexpr int32 WeightGood = 7;

	/** 콤보 중단 이벤트 */
	UPROPERTY(BlueprintAssignable, Category = "Rhythm|Score")
	FOnScoreComboBreak OnComboBreak;

	/** 실시간 점수 (0 ~ 1,000,000) */
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

	/** 누적 판정 노트 수 (롱노트 틱 포함) */
	int32 TotalNoteCount = 0;

	/** 롱노트 틱: 누르고 있어서 성공한 수 (판정 개수 표시와 별도로 집계, 실패한 틱은 MissCount에 포함) */
	int32 HoldTickHitCount = 0;

	/** 채보의 총 노트 수 (플레이 시작 시 설정, 점수 분모) */
	int32 ExpectedNoteCount = 0;

	/** 누적 가중치 */
	int64 WeightSum = 0;

	/** 골드·별 계산용 예전 방식 점수 (기본점수 × 콤보 배율, 100만 상한) */
	int32 LegacyScore = 0;

	/** 점수 분모가 될 총 노트 수 설정. Reset 후 플레이 시작 직전에 호출 */
	void SetExpectedNoteCount(int32 InExpectedNoteCount);

	/** 점수와 콤보 반영 후 증가 점수 반환 */
	int32 AddJudgementScore(const FPTBJudgementResult& Result);

	/**
	 * 롱노트 틱 1개 반영. 성공 = Perfect 가중치 + 콤보 +1, 실패 = Miss(콤보 끊김).
	 * 판정 글자/효과음 없이 점수·콤보만 바뀜.
	 */
	void AddHoldTick(bool bHit);

	/** 최종 라운드 결과 생성 */
	FPTBRoundResult BuildRoundResult(const FString& ProfileId, FName MiniGameId,
			EPTBDifficulty Difficulty, const FPTBMiniGameResultPayload& Payload) const;

	/** 별점 계산 (예전 방식 점수 기준, 기준 미확정이라 그대로 유지) */
	int32 CalculateStarRating(int32 Score) const;

	/** 등급 계산 */
	EPTBGradeType CalculateGrade() const;

	/** 점수 → % (소수점 2자리 버림, 예: 965239 → 96.52) */
	static float ScoreToPercent(int32 Score);

	/** 점수 → "96.52%" */
	static FText ScoreToPercentText(int32 Score);

	/** 누적 상태 초기화 (ExpectedNoteCount는 유지) */
	void Reset();

private:
	static int32 GetJudgementWeight(EPTBJudgementType JudgementType);

	/** 점수 분모 노트 수: max(총 노트 수, 판정 수) */
	int32 GetScoreDenominatorNoteCount() const;

	/** 현재 누적 가중치로 점수 재계산 */
	int32 ComputeNormalizedScore() const;
};
