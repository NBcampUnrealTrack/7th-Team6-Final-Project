#include "Platform/PTBStoveSettings.h"

FString UPTBStoveSettings::GetActiveGameId() const
{
	const UPTBStoveKeySettings* Keys = GetDefault<UPTBStoveKeySettings>();
	if (!Keys)
	{
		return FString();
	}
	return GetActiveEnvironment() == EPTBStoveEnvironment::Live ? Keys->LiveGameId : Keys->SandboxGameId;
}

FString UPTBStoveSettings::GetActiveAppKey() const
{
	const UPTBStoveKeySettings* Keys = GetDefault<UPTBStoveKeySettings>();
	if (!Keys)
	{
		return FString();
	}
	return GetActiveEnvironment() == EPTBStoveEnvironment::Live ? Keys->LiveAppKey : Keys->SandboxAppKey;
}
