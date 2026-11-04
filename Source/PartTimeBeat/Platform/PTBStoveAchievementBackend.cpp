#include "Platform/PTBStoveAchievementBackend.h"

#include "Engine/GameInstance.h"
#include "Platform/PTBStoveSubsystem.h"

UPTBStoveSubsystem* UPTBStoveAchievementBackend::GetStove() const
{
	// Outer: PTBAchievementSubsystem → GameInstance
	UGameInstance* GI = GetTypedOuter<UGameInstance>();
	return GI ? GI->GetSubsystem<UPTBStoveSubsystem>() : nullptr;
}

FString UPTBStoveAchievementBackend::GetStatId(FName AchievementId, const FPTBAchievementRow& Row)
{
	return Row.StoveAchievementId.IsEmpty() ? AchievementId.ToString() : Row.StoveAchievementId;
}

void UPTBStoveAchievementBackend::ReportUnlocked_Implementation(FName AchievementId, const FPTBAchievementRow& Row)
{
	if (UPTBStoveSubsystem* Stove = GetStove())
	{
		// 스탯 갱신 방식이 MAX라서 여러 번 보내도(프로필 로드 시 재동기화 등) 값이 줄지 않음
		Stove->SetStat(GetStatId(AchievementId, Row), FMath::Max(1, Row.RequiredCount));
	}
}

void UPTBStoveAchievementBackend::ReportProgress_Implementation(FName AchievementId, const FPTBAchievementRow& Row, int32 CurrentCount, int32 RequiredCount)
{
	// 진행형 업적(예: 7개 중 3개)만 의미 있음. 목표 1짜리는 달성 시에만 전송
	if (RequiredCount > 1 && CurrentCount > 0)
	{
		if (UPTBStoveSubsystem* Stove = GetStove())
		{
			Stove->SetStat(GetStatId(AchievementId, Row), FMath::Min(CurrentCount, RequiredCount));
		}
	}
}
