#include "Core/PTBGameInstance.h"


void UPTBGameInstance::InitPTBSystems() 
{
}

void UPTBGameInstance::ShutdownPTBSystems() 
{

}

bool UPTBGameInstance::LoadProfile(const FString& ProfileId) 
{
	return true;
}

FString UPTBGameInstance::CreateProfile(const FPTBProfileData& Data) 
{
	return FString();
}

void UPTBGameInstance::ApplyUserSettings(const FPTBUserSettings& InSettings)
{
}

void UPTBGameInstance::SaveGame() 
{
}
bool UPTBGameInstance::LoadGame()
{
	return true;
}

void UPTBGameInstance::AutoSave() 
{
}