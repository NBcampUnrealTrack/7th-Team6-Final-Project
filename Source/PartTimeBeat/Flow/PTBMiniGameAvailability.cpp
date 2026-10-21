// Fill out your copyright notice in the Description page of Project Settings.

#include "Flow/PTBMiniGameAvailability.h"

UPTBMiniGameAvailabilitySettings::UPTBMiniGameAvailabilitySettings()
{
	DefaultComingSoonMessage = NSLOCTEXT("PTBAvailability", "DefaultComingSoon", "Coming Soon\n아직 준비 중인 알바예요!");
}

const FPTBMiniGameAvailabilityEntry* UPTBMiniGameAvailabilitySettings::FindEntry(FName MiniGameId) const
{
	if (MiniGameId.IsNone())
	{
		return nullptr;
	}
	return MiniGames.FindByPredicate([MiniGameId](const FPTBMiniGameAvailabilityEntry& Entry)
	{
		return Entry.MiniGameId == MiniGameId;
	});
}

EPTBMiniGameAvailability UPTBMiniGameAvailabilityLibrary::GetMiniGameAvailability(FName MiniGameId)
{
	const UPTBMiniGameAvailabilitySettings* Settings = GetDefault<UPTBMiniGameAvailabilitySettings>();
	const FPTBMiniGameAvailabilityEntry* Entry = Settings ? Settings->FindEntry(MiniGameId) : nullptr;
	return Entry ? Entry->Availability : EPTBMiniGameAvailability::Available;
}

bool UPTBMiniGameAvailabilityLibrary::IsMiniGamePlayable(FName MiniGameId)
{
	if (GetMiniGameAvailability(MiniGameId) == EPTBMiniGameAvailability::Available)
	{
		return true;
	}

#if WITH_EDITOR
	if (const UPTBMiniGameAvailabilitySettings* Settings = GetDefault<UPTBMiniGameAvailabilitySettings>())
	{
		return Settings->bAllowLockedMiniGamesInEditor;
	}
#endif
	return false;
}

bool UPTBMiniGameAvailabilityLibrary::IsMiniGameComingSoon(FName MiniGameId)
{
	return GetMiniGameAvailability(MiniGameId) == EPTBMiniGameAvailability::ComingSoon;
}

bool UPTBMiniGameAvailabilityLibrary::IsMiniGameHidden(FName MiniGameId)
{
	return GetMiniGameAvailability(MiniGameId) == EPTBMiniGameAvailability::Hidden;
}

FText UPTBMiniGameAvailabilityLibrary::GetMiniGameLockMessage(FName MiniGameId)
{
	const UPTBMiniGameAvailabilitySettings* Settings = GetDefault<UPTBMiniGameAvailabilitySettings>();
	if (!Settings)
	{
		return FText::GetEmpty();
	}
	const FPTBMiniGameAvailabilityEntry* Entry = Settings->FindEntry(MiniGameId);
	return Entry && !Entry->Message.IsEmpty() ? Entry->Message : Settings->DefaultComingSoonMessage;
}
