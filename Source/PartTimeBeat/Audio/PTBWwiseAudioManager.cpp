#include "Audio/PTBWwiseAudioManager.h"

#include "AkAudioDevice.h"
#include "AkAudioEvent.h"
#include "AkComponent.h"
#include "AkGameplayStatics.h"
#include "Audio/PTBWwiseEventMapAsset.h"
#include "Debug/PTBLogChannels.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"

namespace PTBWwiseAudioManagerInternal
{
	void EnsureEventDataLoaded(UAkAudioEvent* Event)
	{
		if (!Event || Event->IsLoaded())
		{
			return;
		}

		Event->LoadData();
	}
}

UPTBWwiseAudioManager::UPTBWwiseAudioManager()
{
	MainAkComponent = nullptr;
	MasterBusID = 0;
	CurrentBGMPlayingId = 0;
	bIsBGMPlaying = false;
}

void UPTBWwiseAudioManager::SetMainAkComponent(UAkComponent* InAkComponent)
{
	MainAkComponent = InAkComponent;
}

void UPTBWwiseAudioManager::ApplyEventMapAsset(UPTBWwiseEventMapAsset* InEventMapAsset)
{
	EventMapAsset = InEventMapAsset;
	EventMap.Reset();
	JudgementEventMap.Reset();

	if (!EventMapAsset)
	{
		return;
	}

	for (const TPair<FName, TObjectPtr<UAkAudioEvent>>& EventPair : EventMapAsset->BGMEvents)
	{
		RegisterEvent(EventPair.Key, EventPair.Value.Get());
	}

	for (const TPair<FName, TObjectPtr<UAkAudioEvent>>& EventPair : EventMapAsset->UIEvents)
	{
		RegisterEvent(EventPair.Key, EventPair.Value.Get());
	}

	for (const TPair<FName, TObjectPtr<UAkAudioEvent>>& EventPair : EventMapAsset->MiniGameSFXMap)
	{
		RegisterEvent(EventPair.Key, EventPair.Value.Get());
	}

	for (const TPair<EPTBJudgementType, TObjectPtr<UAkAudioEvent>>& EventPair : EventMapAsset->JudgeEventMap)
	{
		if (EventPair.Value)
		{
			JudgementEventMap.Add(EventPair.Key, EventPair.Value);
		}
	}
}

void UPTBWwiseAudioManager::RegisterEvent(FName EventKey, UAkAudioEvent* Event)
{
	if (EventKey.IsNone() || !Event)
	{
		return;
	}

	EventMap.Add(EventKey, Event);
}

bool UPTBWwiseAudioManager::LoadSoundBank(FName BankName)
{
	if (BankName.IsNone())
	{
		return false;
	}

	if (LoadedBanks.Contains(BankName))
	{
		return true;
	}

	FAkAudioDevice* AudioDevice = FAkAudioDevice::Get();
	if (!AudioDevice)
	{
		return false;
	}

	AkBankID BankId = AK_INVALID_BANK_ID;
	const AKRESULT Result = AudioDevice->LoadBank(BankName.ToString(), BankId);
	if (Result == AK_Success)
	{
		LoadedBanks.Add(BankName);
		return true;
	}

	return false;
}

void UPTBWwiseAudioManager::UnloadSoundBank(FName BankName)
{
	if (BankName.IsNone() || !LoadedBanks.Contains(BankName))
	{
		return;
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->UnloadBank(BankName.ToString());
	}

	LoadedBanks.Remove(BankName);
}

void UPTBWwiseAudioManager::UnloadAllBanks()
{
	for (const FName& BankName : LoadedBanks)
	{
		if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
		{
			AudioDevice->UnloadBank(BankName.ToString());
		}
	}

	LoadedBanks.Reset();
}

int32 UPTBWwiseAudioManager::PostEvent(FName EventKey, AActor* Target)
{
	const TObjectPtr<UAkAudioEvent>* FoundEvent = EventMap.Find(EventKey);
	if (!FoundEvent || !FoundEvent->Get())
	{
		return 0;
	}

	if (Target)
	{
		return UAkGameplayStatics::PostEvent(FoundEvent->Get(), Target, 0, FOnAkPostEventCallback(), false);
	}

	if (MainAkComponent)
	{
		return MainAkComponent->PostAkEvent(FoundEvent->Get(), 0, FOnAkPostEventCallback());
	}

	return 0;
}

int32 UPTBWwiseAudioManager::PostBGMEvent(FName EventKey)
{
	const TObjectPtr<UAkAudioEvent>* FoundEvent = EventMap.Find(EventKey);
	if (!FoundEvent || !FoundEvent->Get() || !MainAkComponent)
	{
		UE_LOG(LogWwise, Warning, TEXT("PostBGMEvent failed before post. Key=%s Event=%s AkComponent=%s"),
			*EventKey.ToString(),
			FoundEvent && FoundEvent->Get() ? *GetNameSafe(FoundEvent->Get()) : TEXT("None"),
			*GetNameSafe(MainAkComponent.Get()));
		return 0;
	}

	UAkAudioEvent* Event = FoundEvent->Get();
	PTBWwiseAudioManagerInternal::EnsureEventDataLoaded(Event);

	if (!Event->IsLoaded() || !Event->IsDataFullyLoaded())
	{
		UE_LOG(LogWwise, Warning, TEXT("PostBGMEvent event data is not ready. Key=%s Event=%s ShortId=%u Loaded=%d FullyLoaded=%d"),
			*EventKey.ToString(),
			*GetNameSafe(Event),
			Event->GetShortID(),
			Event->IsLoaded() ? 1 : 0,
			Event->IsDataFullyLoaded() ? 1 : 0);
	}

	FOnAkPostEventCallback Callback;
	Callback.BindDynamic(this, &UPTBWwiseAudioManager::HandleBGMPostEventCallback);

	const int32 CallbackMask = AK_EndOfEvent | AK_EnableGetSourcePlayPosition;
	CurrentBGMPlayingId = MainAkComponent->PostAkEvent(Event, CallbackMask, Callback);
	bIsBGMPlaying = CurrentBGMPlayingId != 0;

	return CurrentBGMPlayingId;
}

int32 UPTBWwiseAudioManager::PostSFXEvent(FName EventKey, AActor* Target)
{
	return PostEvent(EventKey, Target);
}

int32 UPTBWwiseAudioManager::PostJudgementEvent(EPTBJudgementType JudgementType, AActor* Target)
{
	const TObjectPtr<UAkAudioEvent>* FoundEvent = JudgementEventMap.Find(JudgementType);
	if (!FoundEvent || !FoundEvent->Get())
	{
		return 0;
	}

	if (Target)
	{
		return UAkGameplayStatics::PostEvent(FoundEvent->Get(), Target, 0, FOnAkPostEventCallback(), false);
	}

	if (MainAkComponent)
	{
		return MainAkComponent->PostAkEvent(FoundEvent->Get(), 0, FOnAkPostEventCallback());
	}

	return 0;
}

void UPTBWwiseAudioManager::StopBGM(float FadeOutMs)
{
	if (CurrentBGMPlayingId == 0)
	{
		return;
	}

	if (FAkAudioDevice* AudioDevice = FAkAudioDevice::Get())
	{
		AudioDevice->StopPlayingID(static_cast<AkPlayingID>(CurrentBGMPlayingId), static_cast<AkTimeMs>(FMath::Max(0.0f, FadeOutMs)));
	}

	bIsBGMPlaying = false;
	CurrentBGMPlayingId = 0;
}

void UPTBWwiseAudioManager::PauseBGM()
{
	if (CurrentBGMPlayingId != 0)
	{
		AK::SoundEngine::ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Pause, static_cast<AkPlayingID>(CurrentBGMPlayingId));
	}
}

void UPTBWwiseAudioManager::ResumeBGM()
{
	if (CurrentBGMPlayingId != 0)
	{
		AK::SoundEngine::ExecuteActionOnPlayingID(AK::SoundEngine::AkActionOnEventType_Resume, static_cast<AkPlayingID>(CurrentBGMPlayingId));
	}
}


void UPTBWwiseAudioManager::SetRTPC(FName Name, float Value, AActor* Target)
{
	if (!Name.IsNone())
	{
		UAkGameplayStatics::SetRTPCValue(nullptr, Value, 0, Target, Name);
	}
}

void UPTBWwiseAudioManager::SetState(FName Group, FName State)
{
	if (!Group.IsNone() && !State.IsNone())
	{
		UAkGameplayStatics::SetState(nullptr, Group, State);
	}
}

void UPTBWwiseAudioManager::SetSwitch(FName Group, FName Switch, AActor* Target)
{
	if (!Group.IsNone() && !Switch.IsNone())
	{
		UAkGameplayStatics::SetSwitch(nullptr, Target, Group, Switch);
	}
}

void UPTBWwiseAudioManager::ApplySettings(const FPTBUserSettings& Settings)
{
	SetRTPC(TEXT("MasterVolume"), Settings.MasterVolume, nullptr);
	SetRTPC(TEXT("BGMVolume"), Settings.BGMVolume, nullptr);
	SetRTPC(TEXT("SFXVolume"), Settings.SFXVolume, nullptr);
}

float UPTBWwiseAudioManager::GetPlaybackPositionMs(int32 PlayingId) const
{
	float PositionMs = 0.0f;
	return TryGetPlaybackPositionMs(PlayingId, PositionMs) ? PositionMs : 0.0f;
}

bool UPTBWwiseAudioManager::TryGetPlaybackPositionMs(int32 PlayingId, float& OutPositionMs) const
{
	OutPositionMs = 0.0f;

	if (PlayingId == 0)
	{
		return false;
	}

	AkTimeMs PositionMs = 0;
	const AKRESULT Result = AK::SoundEngine::GetSourcePlayPosition(static_cast<AkPlayingID>(PlayingId), &PositionMs, true);
	if (Result != AK_Success)
	{
		return false;
	}

	OutPositionMs = static_cast<float>(PositionMs);
	return true;
}

bool UPTBWwiseAudioManager::IsEventPlaying(int32 PlayingId) const
{
	float PositionMs = 0.0f;
	return TryGetPlaybackPositionMs(PlayingId, PositionMs);
}

void UPTBWwiseAudioManager::HandleBGMPostEventCallback(EAkCallbackType CallbackType, UAkCallbackInfo* CallbackInfo)
{
	if (CallbackType != EAkCallbackType::EndOfEvent)
	{
		return;
	}

	const UAkEventCallbackInfo* EventCallbackInfo = Cast<UAkEventCallbackInfo>(CallbackInfo);
	if (!EventCallbackInfo)
	{
		return;
	}

	const int32 FinishedPlayingId = EventCallbackInfo->PlayingID;
	if (FinishedPlayingId == 0 || FinishedPlayingId != CurrentBGMPlayingId)
	{
		return;
	}

	bIsBGMPlaying = false;
	CurrentBGMPlayingId = 0;
	OnBGMFinished.Broadcast(FinishedPlayingId);
}
