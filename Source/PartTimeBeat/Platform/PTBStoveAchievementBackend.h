#pragma once

#include "CoreMinimal.h"
#include "Achievements/PTBAchievementBackend.h"
#include "PTBStoveAchievementBackend.generated.h"

class UPTBStoveSubsystem;

/**
 * STOVE 도전과제 연동.
 * STOVE 도전과제는 "연결된 스탯 값 ≥ 목표값"으로 달성되므로, 게임 업적 달성 시 해당 스탯을 RequiredCount로 설정
 *
 * 스탯 ID = 업적 DT 행의 StoveAchievementId (비어 있으면 행 이름)
 * STOVE 스튜디오 등록 규칙: 스탯 갱신 방식 MAX, 목표값 = DT의 Required Count
 *
 * 사용: Project Settings → PTB 진행·업적 설정 → Achievement Backend Class = PTBStoveAchievementBackend
 */
UCLASS()
class PARTTIMEBEAT_API UPTBStoveAchievementBackend : public UPTBAchievementBackend
{
	GENERATED_BODY()

public:
	virtual void ReportUnlocked_Implementation(FName AchievementId, const FPTBAchievementRow& Row) override;
	virtual void ReportProgress_Implementation(FName AchievementId, const FPTBAchievementRow& Row, int32 CurrentCount, int32 RequiredCount) override;

private:
	UPTBStoveSubsystem* GetStove() const;
	static FString GetStatId(FName AchievementId, const FPTBAchievementRow& Row);
};
