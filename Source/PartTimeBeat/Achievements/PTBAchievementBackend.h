#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Achievements/PTBAchievementTypes.h"
#include "PTBAchievementBackend.generated.h"

/**
 * 플랫폼 업적 연동 (STOVE / Steam).
 * 게임 안 업적(달성 판정·기록·UI)은 PTBAchievementSubsystem이 담당하고, 이 클래스는 "플랫폼에 알리기"만 함.
 *
 * 플랫폼 붙이는 법: 이 클래스를 상속한 C++ 클래스(또는 BP)를 만들어 ReportUnlocked 등을 구현
 *   → Project Settings → PTB 진행·업적 설정 → Achievement Backend Class에 지정
 * 플랫폼 ID는 Row의 StoveAchievementId / SteamAchievementName (비어 있으면 AchievementId)
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBAchievementBackend : public UObject
{
	GENERATED_BODY()

public:
	/** 생성 직후 1회 (SDK 초기화 확인 등) */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Achievement")
	void InitializeBackend();
	virtual void InitializeBackend_Implementation() {}

	/**
	 * 업적 달성 알림. 새로 달성할 때와 프로필을 불러올 때(기존 달성 동기화) 모두 호출되므로
	 * 이미 달성된 업적이 다시 들어와도 문제없게 구현할 것 (플랫폼 API도 대부분 중복 호출 허용)
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Achievement")
	void ReportUnlocked(FName AchievementId, const FPTBAchievementRow& Row);
	virtual void ReportUnlocked_Implementation(FName AchievementId, const FPTBAchievementRow& Row) {}

	/** 진행 상황 알림 (플랫폼이 진행도 표시를 지원할 때. 지원 안 하면 비워 둠) */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Achievement")
	void ReportProgress(FName AchievementId, const FPTBAchievementRow& Row, int32 CurrentCount, int32 RequiredCount);
	virtual void ReportProgress_Implementation(FName AchievementId, const FPTBAchievementRow& Row, int32 CurrentCount, int32 RequiredCount) {}

	/** 플랫폼 업적 초기화 (QA용. 실제 계정에서는 플랫폼이 막을 수 있음) */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Achievement")
	void ResetAll();
	virtual void ResetAll_Implementation() {}
};

/** 기본 백엔드: 플랫폼 연동 없이 로그만 남김 (개발·플랫폼 SDK 연동 전) */
UCLASS()
class PARTTIMEBEAT_API UPTBLogAchievementBackend : public UPTBAchievementBackend
{
	GENERATED_BODY()

public:
	virtual void ReportUnlocked_Implementation(FName AchievementId, const FPTBAchievementRow& Row) override;
	virtual void ResetAll_Implementation() override;
};
