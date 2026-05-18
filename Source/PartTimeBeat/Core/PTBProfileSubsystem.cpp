#include "Core/PTBProfileSubsystem.h"


FPTBProfileData UPTBProfileSubsystem::CreateProfile(const FString& Nickname, int32 Gender, FDateTime Birthday)
{
	return FPTBProfileData();
}
bool UPTBProfileSubsystem::DeleteProfile(const FString& ProfileId)
{
	return true;
}
FPTBProfileData UPTBProfileSubsystem::GetProfile(const FString& ProfileId) const
{
	return FPTBProfileData();
}

int32 UPTBProfileSubsystem::GetProfileCount() const
{
	return 0;
}
bool UPTBProfileSubsystem::ValidateNickname(const FString& Name) const
{
	return true;
}
bool UPTBProfileSubsystem::IsFirstTimeProfile(const FString& ProfileId) const
{
	return true;
}
void UPTBProfileSubsystem::MarkTutorialComplete(const FString& ProfileId, FName GameId)
{

}