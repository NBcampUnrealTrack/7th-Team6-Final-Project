#include "Audio/PTBWwiseAudioManager.h"

bool UPTBWwiseAudioManager::LoadSoundBank(FName BankName)
{
	return true;
}
void UPTBWwiseAudioManager::UnloadSoundBank(FName BankName)
{

}
void UPTBWwiseAudioManager::UnloadAllBanks()
{

}

int32 UPTBWwiseAudioManager::PostEvent(FName EventKey, AActor* Target = nullptr)
{
	return 0;
}
int32 UPTBWwiseAudioManager::PostBGMEvent(FName EventKey)
{
	return 0;
}
int32 UPTBWwiseAudioManager::PostSFXEvent(FName EventKey, AActor* Target = nullptr)
{
	return 0;
}
void UPTBWwiseAudioManager::StopBGM(float FadeOutMs)
{
}
void UPTBWwiseAudioManager::PauseBGM()
{
}
void UPTBWwiseAudioManager::ResumeBGM()
{
}


void UPTBWwiseAudioManager::SetRTPC(FName Name, float Value, AActor* Target = nullptr)
{

}
void UPTBWwiseAudioManager::SetState(FName Group, FName State)
{

}

void UPTBWwiseAudioManager::SetSwitch(FName Group, FName Switch, AActor* Target)
{

}

void UPTBWwiseAudioManager::ApplySettings(const FPTBUserSettings& Settings)
{

}

float UPTBWwiseAudioManager::GetPlaybackPositionMs(int32 PlayingId) const
{
	return 0;
}
bool UPTBWwiseAudioManager::IsEventPlaying(int32 PlayingId) const
{
	return true;
}