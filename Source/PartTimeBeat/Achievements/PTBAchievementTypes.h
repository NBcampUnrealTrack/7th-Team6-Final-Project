#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Core/PTBStructEnums.h"
#include "PTBAchievementTypes.generated.h"

class UTexture2D;

/** 업적 달성 조건 종류 */
UENUM(BlueprintType)
enum class EPTBAchievementCondition : uint8
{
	/** 클리어한 미니게임 수 (난이도·미니게임 필터 적용, 같은 미니게임은 1개로 셈) */
	ClearMiniGames     UMETA(DisplayName = "미니게임 클리어 (개수)"),
	/** Min Grade 이상 등급을 받은 미니게임 수 (같은 미니게임은 1개로 셈) */
	GradeMiniGames     UMETA(DisplayName = "등급 달성 (개수)"),
	/** 총 플레이 횟수 (필터 적용) */
	PlayCount          UMETA(DisplayName = "플레이 횟수"),
	/** 지금까지 번 알바비 총액 */
	TotalEarnedMoney   UMETA(DisplayName = "누적 알바비"),
	/** 조건 판정 없음. 코드·BP에서 UnlockAchievement로 직접 달성 (숨은 업적·특수 이벤트용) */
	Manual             UMETA(DisplayName = "수동 (코드에서 직접 달성)")
};

/** 난이도 필터 */
UENUM(BlueprintType)
enum class EPTBAchievementDifficultyMatch : uint8
{
	Any      UMETA(DisplayName = "난이도 무관"),
	Exactly  UMETA(DisplayName = "이 난이도만"),
	AtLeast  UMETA(DisplayName = "이 난이도 이상 (상위 난이도 클리어도 인정)")
};

/**
 * 업적 정의 (DataTable Row Structure).
 * Row Name = 업적 ID. 세이브와 플랫폼(STOVE/Steam) 연동 키라서 출시 후 바꾸지 말 것.
 *
 * 출시 후 수정 가이드
 *  - 이름·설명·아이콘·정렬: 언제든 수정 가능 (달성 기록에 영향 없음)
 *  - 업적 추가: 새 행 추가 (+ 플랫폼 관리 페이지에 같은 ID 등록)
 *  - 업적 제거: 행을 지우지 말고 bRetired 체크 (목록·판정에서 빠지고, 이미 달성한 기록은 세이브에 남음)
 *  - 조건 변경: 가능하지만 이미 달성한 사람은 그대로 유지됨 (조건을 어렵게 바꿔도 회수 안 함)
 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBAchievementRow : public FTableRowBase
{
	GENERATED_BODY()

	// ── 표시 ──

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	/** 달성 방법 설명 (조건 수치를 바꾸면 같이 수정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display", meta = (MultiLine = true))
	FText Description;

	/** 달성 아이콘 (플랫폼 등록용 원본은 256×256 이상 권장) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 미달성 아이콘 (비우면 UI에서 Icon을 흑백 처리 등) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	TSoftObjectPtr<UTexture2D> LockedIcon;

	/** 숨은 업적: 달성 전에는 이름·설명이 "???"로 표시 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	bool bHidden = false;

	/** 목록 정렬 (작을수록 위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	int32 SortOrder = 0;

	/** 폐기: 목록·판정에서 제외 (행은 지우지 말 것) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	bool bRetired = false;

	// ── 조건 ──

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EPTBAchievementCondition Condition = EPTBAchievementCondition::ClearMiniGames;

	/** 필요한 수 (개수 / 횟수 / 금액) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (ClampMin = "1"))
	int32 RequiredCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	EPTBAchievementDifficultyMatch DifficultyMatch = EPTBAchievementDifficultyMatch::Any;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (EditCondition = "DifficultyMatch != EPTBAchievementDifficultyMatch::Any"))
	EPTBDifficulty Difficulty = EPTBDifficulty::Easy;

	/** GradeMiniGames 조건의 최소 등급 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition", meta = (EditCondition = "Condition == EPTBAchievementCondition::GradeMiniGames"))
	EPTBGradeType MinGrade = EPTBGradeType::S;

	/** 특정 미니게임만 (None = 모든 미니게임) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Condition")
	FName MiniGameId = NAME_None;

	// ── 플랫폼 연동 (비우면 Row Name 사용) ──

	/** STOVE 업적 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	FString StoveAchievementId;

	/** Steam 업적 API Name */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform")
	FString SteamAchievementName;
};

/** 업적 UI 표시용 */
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBAchievementView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	FName AchievementId;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	FPTBAchievementRow Data;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	bool bUnlocked = false;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	FDateTime UnlockedAt;

	/** 숨은 업적이고 아직 미달성 (UI에서 "???"로 표시) */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	bool bConcealed = false;

	/** 진행 상황 (예: 3 / 7). Manual 조건은 0 / 1 */
	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	int32 CurrentCount = 0;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Achievement")
	int32 RequiredCount = 1;
};
