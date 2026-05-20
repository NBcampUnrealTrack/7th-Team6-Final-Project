#include "Core/PTBSaveGame.h"

void UPTBSaveGame::InitializeDefaultSave()
{
	Profiles.Empty();
	ActiveProfileIndex = 0;
	StageProgress.Empty();
	StoryProgress.Empty();
	TutorialFlags.Empty();

	// 기본 설정값
	Settings.MasterVolume = 1.0f;
	Settings.BGMVolume = 0.8f;
	Settings.SFXVolume = 0.8f;
	Settings.JudgementOffsetMs = 0.0f;
	Settings.InputLatencyMs = 0.0f;
	Settings.bFullscreen = true;
	Settings.bVibrationEnabled = true;
}

bool UPTBSaveGame::UpsertProfile(const FPTBProfileData& Data)
{
	// 기존 프로필 검색 (ProfileId == FGuid)
	for (int32 i = 0; i < Profiles.Num(); i++)
	{
		if (Profiles[i].ProfileId == Data.ProfileId)
		{
			Profiles[i] = Data;
			return true; // 갱신 완료
		}
	}

	// 새 프로필 추가
	Profiles.Add(Data);
	return true;
}

void UPTBSaveGame::ApplyRewardSummary(const FString& ProfileId, const FPTBRewardSummary& Reward)
{
	FGuid TargetGuid;
	if (!FGuid::Parse(ProfileId, TargetGuid))
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyRewardSummary: Invalid ProfileId format [%s]"), *ProfileId);
		return;
	}

	for (auto& Profile : Profiles)
	{
		if (Profile.ProfileId == TargetGuid)
		{
			Profile.TotalEarnedMoney += Reward.EarnedMoney;

			// 미니게임별 별 갱신은 MiniGameId가 필요하므로
			// 호출부에서 StageProgress도 함께 갱신하도록 권장
			// (여기서는 전역 보상만 반영)
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ApplyRewardSummary: Profile not found [%s]"), *ProfileId);
}

void UPTBSaveGame::MarkTutorialDone(const FString& ProfileId, FName GameId)
{
	// 전역 플래그(프로필별 관리가 필요하면 키를 ProfileId+GameId로 조합)
	TutorialFlags.Add(GameId, true);

	UE_LOG(LogTemp, Log, TEXT("Tutorial marked done — Profile: %s, Game: %s"),
		*ProfileId, *GameId.ToString());
}