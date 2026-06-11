#include "MiniGames/JJ/PTBJJMiniGameRuleSet.h"

int32 UPTBJJMiniGameRuleSet::ResolveCharacterIndex(EPTBActionType Action) const
{
	if (const int32* FoundIndex = ActionToCharacterIndex.Find(Action))
	{
		return *FoundIndex;
	}

	return 0; // 매핑 미설정 시 좌측 기본
}
