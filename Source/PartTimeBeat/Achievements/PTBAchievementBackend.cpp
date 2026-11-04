#include "Achievements/PTBAchievementBackend.h"

#include "Debug/PTBTeamLog.h"

void UPTBLogAchievementBackend::ReportUnlocked_Implementation(FName AchievementId, const FPTBAchievementRow& Row)
{
	PTB_VERBOSE(LogPTBProfile, TEXT("[Achievement] (플랫폼 미연동) Report: %s Stove=%s Steam=%s"),
		*AchievementId.ToString(), *Row.StoveAchievementId, *Row.SteamAchievementName);
}

void UPTBLogAchievementBackend::ResetAll_Implementation()
{
	PTB_RECORD(LogPTBProfile, TEXT("[Achievement] (플랫폼 미연동) ResetAll"));
}
