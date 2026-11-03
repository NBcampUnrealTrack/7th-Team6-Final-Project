#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Achievements/PTBAchievementTypes.h"
#include "PTBAchievementSubsystem.generated.h"

class UDataTable;
class UPTBAchievementBackend;
class UPTBProfileSubsystem;
struct FPTBProfileData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPTBAchievementUnlocked, FName, AchievementId, const FPTBAchievementView&, View);

/**
 * 업적: 정의(DataTable) 로드, 달성 판정, 프로필별 기록, 플랫폼 알림.
 *
 * 흐름
 *  - 라운드 결과 반영 후 GameMode가 EvaluateAchievements 호출 → 새로 달성한 업적 기록·저장·OnAchievementUnlocked
 *  - 프로필을 불러올 때 다시 판정 + 이미 달성한 업적을 플랫폼에 동기화 (플랫폼 연동 전에 달성한 것도 반영됨)
 *
 * 업적 목록은 Project Settings → PTB 진행·업적 설정 → Achievement Table (DT 한 곳에서 관리)
 * 달성 기록은 프로필별 (FPTBProfileData::UnlockedAchievements). 플랫폼 업적은 계정 단위라 어느 프로필이든 달성하면 플랫폼에 반영됨
 */
UCLASS()
class PARTTIMEBEAT_API UPTBAchievementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** 조건을 만족한 업적을 모두 달성 처리. 새로 달성한 업적 ID 반환 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Achievement")
	TArray<FName> EvaluateAchievements();

	/** 직접 달성 (Manual 조건 업적, 이벤트성 업적). 이미 달성했거나 없는 ID면 false */
	UFUNCTION(BlueprintCallable, Category = "PTB|Achievement")
	bool UnlockAchievement(FName AchievementId);

	UFUNCTION(BlueprintPure, Category = "PTB|Achievement")
	bool IsAchievementUnlocked(FName AchievementId) const;

	/** 업적 목록 (폐기 제외, SortOrder 순). 업적 화면 UI용 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Achievement")
	TArray<FPTBAchievementView> GetAchievementViews() const;

	/** 업적 하나 조회 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Achievement")
	bool GetAchievementView(FName AchievementId, FPTBAchievementView& OutView) const;

	/** 달성 수 / 전체 수 (폐기 제외). 업적 진행도 표시용 */
	UFUNCTION(BlueprintPure, Category = "PTB|Achievement")
	void GetAchievementCounts(int32& OutUnlocked, int32& OutTotal) const;

	/** 새로 달성했을 때 (토스트 알림 등) */
	UPROPERTY(BlueprintAssignable, Category = "PTB|Achievement")
	FOnPTBAchievementUnlocked OnAchievementUnlocked;

	// ── QA 치트용 (UPTBCheatLibrary에서 호출) ──

	/** 활성 프로필의 업적 기록 초기화 (+ 플랫폼 초기화 요청) */
	void ResetActiveProfileAchievements();

	/** 모든 업적 달성 (폐기 제외) */
	void UnlockAllAchievements();

	/** 업적 ID 목록 (폐기 포함) */
	TArray<FName> GetAllAchievementIds() const { return SortedIds; }

private:
	void LoadTable();
	void CreateBackend();
	void HandleActiveProfileChanged(const FPTBProfileData& NewProfile);

	UPTBProfileSubsystem* GetProfiles() const;

	/** 조건 진행 수치 */
	int32 ComputeProgress(const FPTBProfileData& Profile, FName AchievementId, const FPTBAchievementRow& Row) const;

	/** 기록 + 플랫폼 알림 (저장·이벤트는 호출한 쪽에서) */
	bool UnlockInProfile(FPTBProfileData& Profile, FName AchievementId, const FPTBAchievementRow& Row);

	/** 이미 달성한 업적을 플랫폼에 다시 알림 */
	void SyncUnlockedToPlatform(const FPTBProfileData& Profile);

	FPTBAchievementView MakeView(const FPTBProfileData* Profile, FName AchievementId, const FPTBAchievementRow& Row) const;

	UPROPERTY(Transient)
	TObjectPtr<UDataTable> AchievementTable;

	UPROPERTY(Transient)
	TObjectPtr<UPTBAchievementBackend> Backend;

	TMap<FName, FPTBAchievementRow> AchievementsById;

	/** SortOrder → Row Name 순 */
	TArray<FName> SortedIds;

	FDelegateHandle ProfileChangedHandle;
};
