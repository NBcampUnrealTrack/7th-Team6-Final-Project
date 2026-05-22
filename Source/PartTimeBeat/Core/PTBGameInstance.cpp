#include "Core/PTBGameInstance.h"
#include "Core/PTBSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PTBMainTitleWidget.h"

// 내부에서 SaveGame 오브젝트를 보관할 멤버가 헤더에 없으므로
// 헤더에 아래를 추가하는 것을 권장합니다:
//   UPROPERTY() UPTBSaveGame* CurrentSaveGame;

void UPTBGameInstance::InitPTBSystems()
{
	// 1) 세이브 로드 시도
	if (!LoadGame())
	{
		// 세이브 없음 → 새로 생성
		UPTBSaveGame* NewSave = Cast<UPTBSaveGame>(
			UGameplayStatics::CreateSaveGameObject(UPTBSaveGame::StaticClass()));
		if (NewSave)
		{
			NewSave->InitializeDefaultSave();
			// CurrentSaveGame = NewSave;
			CachedSettings = NewSave->Settings;
			SaveGame();
		}
	}

	// 2) AudioManager 초기화
	// AudioManager가 UObject 파생이라면:
	// AudioManager = NewObject<UPTBWwiseAudioManager>(this);
	// AudioManager->Initialize();

	// 3) 캐시된 설정 적용
	ApplyUserSettings(CachedSettings);

	// 4) 초기 플로우 상태
	CurrentFlowState = EGameFlowState::MainMenu;
	OnFlowStateChanged.Broadcast(CurrentFlowState);

	UE_LOG(LogTemp, Log, TEXT("PTBSystems initialized"));

	// 타이틀 위젯 생성 및 표시
	if (TitleWidgetClass)
	{
		if (TitleWidgetInstance)
		{
			TitleWidgetInstance->RemoveFromParent();
			TitleWidgetInstance = nullptr;
		}

		// PlayerController로 생성
		APlayerController* PC = GetFirstLocalPlayerController();
		if (PC)
		{
			TitleWidgetInstance = CreateWidget<UPTBMainTitleWidget>(PC, TitleWidgetClass);
			if (TitleWidgetInstance)
				TitleWidgetInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[PTBGameInstance] InitPTBSystems: PlayerController is null, widget not created"));
		}
	}
}

void UPTBGameInstance::ShutdownPTBSystems()
{
	// 최종 저장
	SaveGame();

	// 오디오 매니저 정리
	// if (AudioManager) { AudioManager->Shutdown(); }

	UE_LOG(LogTemp, Log, TEXT("PTBSystems shut down"));
}

bool UPTBGameInstance::LoadProfile(const FString& ProfileId)
{
	FGuid TargetGuid;
	if (!FGuid::Parse(ProfileId, TargetGuid))
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadProfile: Invalid GUID format [%s]"), *ProfileId);
		return false;
	}

	// CurrentSaveGame에서 프로필 검색
	// UPTBSaveGame* Save = CurrentSaveGame;
	// if (!Save) return false;
	//
	// for (int32 i = 0; i < Save->Profiles.Num(); i++)
	// {
	//     if (Save->Profiles[i].ProfileId == TargetGuid)
	//     {
	//         ActiveProfileId = ProfileId;
	//         ActiveProfile = Save->Profiles[i];
	//         Save->ActiveProfileIndex = i;
	//         OnProfileChanged.Broadcast(ActiveProfile);
	//         UE_LOG(LogTemp, Log, TEXT("Profile loaded: %s (%s)"),
	//             *ActiveProfile.Nickname, *ProfileId);
	//         return true;
	//     }
	// }

	UE_LOG(LogTemp, Warning, TEXT("LoadProfile: Profile not found [%s]"), *ProfileId);
	return false;
}

FString UPTBGameInstance::CreateProfile(const FPTBProfileData& Data)
{
	FPTBProfileData NewData = Data;
	NewData.ProfileId = FGuid::NewGuid();
	NewData.CreatedAt = FDateTime::Now();
	NewData.TotalEarnedMoney = 0;
	NewData.BestScoresByMiniGame.Empty();
	NewData.EarnedStarsByMiniGame.Empty();

	// SaveGame에 등록
	// if (CurrentSaveGame)
	// {
	//     CurrentSaveGame->UpsertProfile(NewData);
	// }

	FString NewIdStr = NewData.ProfileId.ToString();

	// 생성 직후 활성화
	LoadProfile(NewIdStr);
	SaveGame();

	UE_LOG(LogTemp, Log, TEXT("Profile created: %s (%s)"),
		*NewData.Nickname, *NewIdStr);

	return NewIdStr;
}

void UPTBGameInstance::ApplyUserSettings(const FPTBUserSettings& InSettings)
{
	CachedSettings = InSettings;

	// Wwise 볼륨 반영
	// if (AudioManager)
	// {
	//     AudioManager->SetMasterVolume(InSettings.MasterVolume);
	//     AudioManager->SetBGMVolume(InSettings.BGMVolume);
	//     AudioManager->SetSFXVolume(InSettings.SFXVolume);
	// }

	// 판정/입력 오프셋은 RhythmConductor에서 읽어가므로 캐시만 갱신
	// (미니게임 시작 시 GameContext.UserSettings로 전달됨)

	// 화면 모드
	if (InSettings.bFullscreen)
	{
		// UGameUserSettings 등으로 전체화면 전환
	}

	// SaveGame에도 반영
	// if (CurrentSaveGame)
	// {
	//     CurrentSaveGame->Settings = InSettings;
	// }

	SaveGame();

	UE_LOG(LogTemp, Log, TEXT("UserSettings applied — Master:%.2f BGM:%.2f SFX:%.2f Offset:%.1fms"),
		InSettings.MasterVolume, InSettings.BGMVolume,
		InSettings.SFXVolume, InSettings.JudgementOffsetMs);
}

void UPTBGameInstance::SaveGame()
{
	// if (CurrentSaveGame)
	// {
	//     UGameplayStatics::SaveGameToSlot(
	//         CurrentSaveGame,
	//         CurrentSaveGame->SaveSlotName,
	//         CurrentSaveGame->UserIndex);
	// }
}

bool UPTBGameInstance::LoadGame()
{
	const FString SlotName = TEXT("PTBSave");
	const int32 UserIdx = 0;

	if (!UGameplayStatics::DoesSaveGameExist(SlotName, UserIdx))
	{
		return false;
	}

	USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIdx);
	UPTBSaveGame* PTBSave = Cast<UPTBSaveGame>(Loaded);
	if (!PTBSave)
	{
		UE_LOG(LogTemp, Error, TEXT("LoadGame: Cast failed"));
		return false;
	}

	// CurrentSaveGame = PTBSave;
	CachedSettings = PTBSave->Settings;

	// 마지막 활성 프로필 복원
	if (PTBSave->Profiles.IsValidIndex(PTBSave->ActiveProfileIndex))
	{
		ActiveProfile = PTBSave->Profiles[PTBSave->ActiveProfileIndex];
		ActiveProfileId = ActiveProfile.ProfileId.ToString();
	}

	UE_LOG(LogTemp, Log, TEXT("SaveGame loaded from slot [%s]"), *SlotName);
	return true;
}

void UPTBGameInstance::AutoSave()
{
	UE_LOG(LogTemp, Log, TEXT("AutoSave triggered"));
	SaveGame();
}