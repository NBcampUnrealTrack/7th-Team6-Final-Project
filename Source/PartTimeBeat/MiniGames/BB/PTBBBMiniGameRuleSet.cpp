#include "MiniGames/BB/PTBBBMiniGameRuleSet.h"

FPTBBBDifficultyConfig UPTBBBMiniGameRuleSet::GetDifficultyConfig(EPTBDifficulty Difficulty) const
{
	if (const FPTBBBDifficultyConfig* Found = DifficultyConfigs.Find(Difficulty))
	{
		return *Found;
	}

	// 설정이 없으면 Standard fallback
	if (Difficulty != EPTBDifficulty::Standard)
	{
		if (const FPTBBBDifficultyConfig* Standard = DifficultyConfigs.Find(EPTBDifficulty::Standard))
		{
			return *Standard;
		}
	}

	return FPTBBBDifficultyConfig{};
}
