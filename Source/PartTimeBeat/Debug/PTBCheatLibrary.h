#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Core/PTBStructEnums.h"
#include "PTBCheatLibrary.generated.h"

class APlayerController;

/** QA 치트 사용 가능 빌드인지 (Shipping 제외) */
#define PTB_WITH_CHEATS (!UE_BUILD_SHIPPING)

/**
 * QA 치트 (Development / DebugGame / 에디터 전용).
 * Shipping 빌드에서는 함수 몸체가 빠져서 호출해도 아무 일도 안 함 + 콘솔 명령어·단축키도 등록 안 됨.
 *
 * 사용법
 *  - 콘솔(~): PTB.Cheat.Help 로 명령어 목록
 *  - 단축키(기본 F10): Project Settings → PTB 진행·업적 설정 → Cheat Panel Widget Class 의 WBP를 여닫음
 *    WBP 버튼에서 아래 BP 함수(PTB|Cheat)를 호출하도록 꾸미면 됨
 */
UCLASS()
class PARTTIMEBEAT_API UPTBCheatLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** 치트 사용 가능 빌드인지 (Shipping이면 false) */
	UFUNCTION(BlueprintPure, Category = "PTB|Cheat")
	static bool IsCheatAvailable();

	// ── 알바비 ──

	/** 현재 소지금을 이 값으로 (누적 수입은 그대로 두고 사용 금액으로 조절 → 엔딩 판정에 영향 없음) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatSetMoney(const UObject* WorldContextObject, int32 CurrentMoney);

	/** 소지금 더하기 (음수면 빼기) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatAddMoney(const UObject* WorldContextObject, int32 Amount);

	/** 누적 수입(엔딩 판정 기준) 설정. 소지금은 그대로 유지 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatSetTotalEarnedMoney(const UObject* WorldContextObject, int32 TotalEarned);

	// ── 아이템 ──

	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatGrantItem(const UObject* WorldContextObject, FName ItemId);

	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatGrantAllItems(const UObject* WorldContextObject);

	/** 보유·장착 아이템 초기화 (기본 아이템만 남음) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatResetItems(const UObject* WorldContextObject);

	// ── 난이도 해금 / 기록 ──

	/** 조건과 무관하게 해금 (MiniGameId=None이면 진행 대상 미니게임 전체) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatUnlockDifficulty(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty = EPTBDifficulty::Insane);

	/** 치트로 해금한 것만 되돌림 (실제 조건 달성분은 유지) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatResetForcedUnlocks(const UObject* WorldContextObject);

	/**
	 * 플레이 기록 추가: 해당 난이도를 Times번 이 등급으로 플레이한 것으로 기록
	 * (플레이·클리어 횟수, 최고 등급, 최고 점수·별). 알바비는 바뀌지 않음
	 * Score <= 0이면 등급에 맞는 테스트용 점수를 자동으로 넣음 (올퍼펙트 1,000,000 / 풀콤보 950,000 / S 900,000 ...)
	 * 최고 점수·별은 기존 기록보다 높을 때만 갱신
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatAddRecord(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty, EPTBGradeType Grade, int32 Times = 1, int32 Score = 0);

	/** 플레이·클리어 횟수, 최고 등급·점수·별 기록 초기화 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatResetRecords(const UObject* WorldContextObject);

	// ── 업적 ──

	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatUnlockAchievement(const UObject* WorldContextObject, FName AchievementId);

	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatUnlockAllAchievements(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatResetAchievements(const UObject* WorldContextObject);

	// ── 기타 ──

	/** 활성 프로필 상태를 Output Log에 출력 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat", meta = (WorldContext = "WorldContextObject"))
	static void CheatPrintStatus(const UObject* WorldContextObject);

	/** 치트 패널 여닫기 (단축키에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Cheat")
	static void ToggleCheatPanel(APlayerController* PlayerController);

	/** 치트 패널 단축키인지 (PlayerController에서 사용) */
	static bool IsCheatPanelKey(const FKey& Key);

private:
	/** 활성 프로필 (수정용). 없으면 nullptr + 로그 */
	static FPTBProfileData* GetActiveProfileMutable(const UObject* WorldContextObject, class UPTBProfileSubsystem*& OutProfiles);

	/** 소지금·아이템 UI 갱신 알림 */
	static void NotifyMoneyChanged(const UObject* WorldContextObject);
};
