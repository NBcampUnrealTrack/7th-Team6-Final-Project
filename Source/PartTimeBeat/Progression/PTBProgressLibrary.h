#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/PTBStructEnums.h"
#include "PTBProgressLibrary.generated.h"

struct FPTBProfileData;

/**
 * 난이도별 플레이 기록 조회 + 난이도 해금 판정 (활성 프로필 기준).
 * 난이도 선택 UI: Insane 버튼 Is Enabled ← IsDifficultyUnlocked, Tool Tip Text ← GetDifficultyUnlockConditionText
 */
UCLASS()
class PARTTIMEBEAT_API UPTBProgressLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 이 미니게임의 해당 난이도를 플레이할 수 있는지 (Easy/Standard는 항상 true) */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress", meta = (WorldContext = "WorldContextObject"))
	static bool IsDifficultyUnlocked(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty);

	/**
	 * 해금 조건 안내 문구 (진행 상황 포함). 이미 해금됐으면 빈 텍스트.
	 * 예) "발광모드 해금 조건 (하나만 달성)\n· 플레이 12/30회\n· 클리어 5/15회\n· 통상모드 S 이상 (최고: A)"
	 */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress", meta = (WorldContext = "WorldContextObject"))
	static FText GetDifficultyUnlockConditionText(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty);

	/** 플레이 횟수 (결과 화면까지 간 판) */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress", meta = (WorldContext = "WorldContextObject"))
	static int32 GetPlayCount(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty);

	/** 클리어 횟수 (등급 Fail 제외) */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress", meta = (WorldContext = "WorldContextObject"))
	static int32 GetClearCount(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty);

	/** 최고 등급. 플레이 기록이 없으면 bOutHasRecord=false */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress", meta = (WorldContext = "WorldContextObject"))
	static EPTBGradeType GetBestGrade(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty, bool& bOutHasRecord);

	/** Grade가 Threshold 이상인지 (PerfectFullCombo > FullCombo > S > A > B > C > Clear > Fail) */
	UFUNCTION(BlueprintPure, Category = "PTB|Progress")
	static bool IsGradeAtLeast(EPTBGradeType Grade, EPTBGradeType Threshold);

	// ── C++ 공용 ──

	/** "MiniGameId_Difficulty" 키 (프로필의 난이도별 기록 키) */
	static FName MakeDifficultyKey(FName MiniGameId, EPTBDifficulty Difficulty);

	/** 키를 미니게임 ID와 난이도로 분리 */
	static bool ParseDifficultyKey(FName Key, FName& OutMiniGameId, EPTBDifficulty& OutDifficulty);

	/** 프로필 데이터 기준 해금 판정 (치트·업적 등 C++에서 프로필을 직접 다룰 때) */
	static bool IsDifficultyUnlockedInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty);

	/** 활성 프로필 (없으면 nullptr) */
	static const FPTBProfileData* GetActiveProfile(const UObject* WorldContextObject);

	static int32 GetPlayCountInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty);
	static int32 GetClearCountInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty);
	static bool GetBestGradeInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty, EPTBGradeType& OutGrade);
};
