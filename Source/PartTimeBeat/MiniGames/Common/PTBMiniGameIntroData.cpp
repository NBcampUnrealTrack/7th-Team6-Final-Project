#include "MiniGames/Common/PTBMiniGameIntroData.h"

UDataTable* UPTBMiniGameIntroData::GetTable(EPTBMiniGameScenePhase Phase) const
{
	switch (Phase)
	{
	case EPTBMiniGameScenePhase::IntroStory: return IntroStory;
	case EPTBMiniGameScenePhase::Tutorial:   return Tutorial;
	case EPTBMiniGameScenePhase::OutroStory: return OutroStory;
	default:                                 return nullptr;
	}
}

int32 UPTBMiniGameIntroData::GetVersion(EPTBMiniGameScenePhase Phase) const
{
	switch (Phase)
	{
	case EPTBMiniGameScenePhase::IntroStory: return IntroStoryVersion;
	case EPTBMiniGameScenePhase::Tutorial:   return TutorialVersion;
	case EPTBMiniGameScenePhase::OutroStory: return OutroStoryVersion;
	default:                                 return 0;
	}
}

bool UPTBMiniGameIntroData::HasContent(EPTBMiniGameScenePhase Phase) const
{
	const UDataTable* Table = GetTable(Phase);
	return Table && Table->GetRowMap().Num() > 0;
}

FName UPTBMiniGameIntroData::MakeSceneKey(FName MiniGameId, EPTBMiniGameScenePhase Phase)
{
	const UEnum* PhaseEnum = StaticEnum<EPTBMiniGameScenePhase>();
	const FString PhaseName = PhaseEnum
		? PhaseEnum->GetNameStringByValue(static_cast<int64>(Phase))
		: FString::FromInt(static_cast<int32>(Phase));

	return FName(*FString::Printf(TEXT("%s.%s"), *MiniGameId.ToString(), *PhaseName));
}
