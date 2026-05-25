#include "MiniGames/Common/PTBMiniGameRuleSet.h"

bool UPTBMiniGameRuleSet::SupportsAction(EPTBActionType Action) const
{
	if (Action == EPTBActionType::None)
	{
		return false;
	}

	if (SupportedActions.IsEmpty())
	{
		return true;
	}

	return SupportedActions.Contains(Action);
}

FName UPTBMiniGameRuleSet::GetActionTag(EPTBActionType Action) const
{
	if (const FName* FoundTag = ActionTags.Find(Action))
	{
		return *FoundTag;
	}

	return NAME_None;
}

FName UPTBMiniGameRuleSet::GetJudgementSFXKey(EPTBJudgementType JudgementType) const
{
	if (const FName* FoundKey = JudgementSFXKeys.Find(JudgementType))
	{
		return *FoundKey;
	}

	return JudgementType == EPTBJudgementType::Miss ? FailSFXKey : SuccessSFXKey;
}

bool UPTBMiniGameRuleSet::ShouldFailForMissCount(int32 MissCount) const
{
	return bFailOnMissLimit && MissCount >= MaxMissCount;
}
