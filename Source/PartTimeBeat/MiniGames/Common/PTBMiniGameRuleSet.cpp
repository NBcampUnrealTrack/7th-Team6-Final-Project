#include "MiniGames/Common/PTBMiniGameRuleSet.h"

void UPTBMiniGameRuleSet::PostLoad()
{
	Super::PostLoad();

	if (!bHasMigratedDifficultyMissLimitSettings)
	{
		constexpr bool bDefaultFailOnMissLimit = false;
		constexpr int32 DefaultMaxMissCount = 10;
		const bool bHasLegacyMissLimitSettings = bFailOnMissLimit != bDefaultFailOnMissLimit
			|| MaxMissCount != DefaultMaxMissCount;

		if (bHasLegacyMissLimitSettings)
		{
			const bool bEasySettingsAreDefault = bFailOnMissLimitEasy == bDefaultFailOnMissLimit
				&& MaxMissCountEasy == DefaultMaxMissCount;
			if (bEasySettingsAreDefault)
			{
				bFailOnMissLimitEasy = bFailOnMissLimit;
				MaxMissCountEasy = MaxMissCount;
			}

			const bool bStandardSettingsAreDefault = bFailOnMissLimitStandard == bDefaultFailOnMissLimit
				&& MaxMissCountStandard == DefaultMaxMissCount;
			if (bStandardSettingsAreDefault)
			{
				bFailOnMissLimitStandard = bFailOnMissLimit;
				MaxMissCountStandard = MaxMissCount;
			}

			const bool bInsaneSettingsAreDefault = bFailOnMissLimitInsane == bDefaultFailOnMissLimit
				&& MaxMissCountInsane == DefaultMaxMissCount;
			if (bInsaneSettingsAreDefault)
			{
				bFailOnMissLimitInsane = bFailOnMissLimit;
				MaxMissCountInsane = MaxMissCount;
			}
		}

		bHasMigratedDifficultyMissLimitSettings = true;
	}
}

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

UPTBRhythmChartAsset* UPTBMiniGameRuleSet::ResolveChartAsset(EPTBDifficulty Difficulty) const
{
	if (const TObjectPtr<UPTBRhythmChartAsset>* FoundChartAsset = ChartAssetsByDifficulty.Find(Difficulty))
	{
		return FoundChartAsset->Get();
	}

	if (Difficulty != EPTBDifficulty::Standard)
	{
		if (const TObjectPtr<UPTBRhythmChartAsset>* StandardChartAsset = ChartAssetsByDifficulty.Find(EPTBDifficulty::Standard))
		{
			return StandardChartAsset->Get();
		}
	}

	return nullptr;
}

FName UPTBMiniGameRuleSet::GetJudgementSFXKey(EPTBJudgementType JudgementType) const
{
	if (const FName* FoundKey = JudgementSFXKeys.Find(JudgementType))
	{
		return *FoundKey;
	}

	return JudgementType == EPTBJudgementType::Miss ? FailSFXKey : SuccessSFXKey;
}

bool UPTBMiniGameRuleSet::ShouldFailForMissCount(EPTBDifficulty Difficulty, int32 MissCount) const
{
	switch (Difficulty)
	{
	case EPTBDifficulty::Easy:
		return bFailOnMissLimitEasy && MissCount >= MaxMissCountEasy;

	case EPTBDifficulty::Insane:
		return bFailOnMissLimitInsane && MissCount >= MaxMissCountInsane;

	case EPTBDifficulty::Standard:
	default:
		return bFailOnMissLimitStandard && MissCount >= MaxMissCountStandard;
	}
}

bool UPTBMiniGameRuleSet::ShouldTreatEmptyInputAsMiss() const
{
	return EmptyInputPolicy == EPTBEmptyInputPolicy::Miss;
}

bool UPTBMiniGameRuleSet::ShouldLockActionOnEmptyInput() const
{
	return bUseEmptyInputActionLock && EmptyInputActionLockMs > 0.0f;
}
