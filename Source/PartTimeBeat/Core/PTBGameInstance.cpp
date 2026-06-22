#include "Core/PTBGameInstance.h"
#include "Core/PTBSaveGame.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PTBMainTitleWidget.h"
#include "Debug/PTBTeamLog.h"

// 내부에서 SaveGame 오브젝트를 보관할 멤버가 헤더에 없으므로
// 헤더에 아래를 추가하는 것을 권장합니다:
//   UPROPERTY() UPTBSaveGame* CurrentSaveGame;

void UPTBGameInstance::Init()
{
	Super::Init();
	InitPTBSystems();
}


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
			CurrentSaveGame = NewSave;
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

	PTB_RECORD(LogPTBCore, TEXT("PTBSystems initialized"));
}

void UPTBGameInstance::ShutdownPTBSystems()
{
	// 최종 저장
	SaveGame();

	// 오디오 매니저 정리
	// if (AudioManager) { AudioManager->Shutdown(); }

	PTB_RECORD(LogPTBCore, TEXT("PTBSystems shut down"));
}

void UPTBGameInstance::ApplyUserSettings(const FPTBUserSettings& InSettings)
{
	CachedSettings = InSettings;

	// TODO: Wwise RTPC 연결 후 활성화
	// if (AudioManager) { AudioManager->ApplySettings(InSettings); }

	// 창 모드·해상도·그래픽 품질 반영
	if (UGameUserSettings* GUS = UGameUserSettings::GetGameUserSettings())
	{
		// 창 모드
		EWindowMode::Type WinMode;
		switch (InSettings.WindowMode)
		{
		case EPTBWindowMode::Fullscreen:         WinMode = EWindowMode::Fullscreen;         break;
		case EPTBWindowMode::WindowedFullscreen: WinMode = EWindowMode::WindowedFullscreen; break;
		default:                                  WinMode = EWindowMode::Windowed;           break;
		}
		GUS->SetFullscreenMode(WinMode);

		// 해상도 (PTBResolution 네임스페이스 공유 유틸리티 사용)
		GUS->SetScreenResolution(PTBResolution::GetPreset(InSettings.ResolutionPresetIndex));

		// 그래픽 품질 (0=낮음, 1=중간, 2=높음, 3=최고)
		const int32 ClampedGraphicsQuality = FMath::Clamp(InSettings.GraphicsQuality, 0, 3);
		GUS->SetOverallScalabilityLevel(ClampedGraphicsQuality);

		GUS->ApplySettings(false);
	}

	SaveGame();

	PTB_RECORD(LogPTBCore, TEXT("UserSettings applied — Master:%.2f BGM:%.2f SFX:%.2f Offset:%.1fms / WinMode:%d / Resolution:%d / Graphics:%d"),
		InSettings.MasterVolume, InSettings.BGMVolume, InSettings.SFXVolume, InSettings.JudgementOffsetMs,
		static_cast<int32>(InSettings.WindowMode), InSettings.ResolutionPresetIndex, InSettings.GraphicsQuality);
}

void UPTBGameInstance::SaveGame()
{
	if (!CurrentSaveGame)
	{
		PTB_WARNING(LogPTBCore, TEXT("SaveGame: CurrentSaveGame is null, skipped"));
		return;
	}
	
	CurrentSaveGame->Settings = CachedSettings;

	UGameplayStatics::SaveGameToSlot(
		CurrentSaveGame,
		CurrentSaveGame->SaveSlotName,
		CurrentSaveGame->UserIndex);
	PTB_RECORD(LogPTBCore, TEXT("SaveGame: saved to slot [%s]"), *CurrentSaveGame->SaveSlotName);
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
		PTB_ERROR(LogPTBCore, TEXT("LoadGame: Cast failed"));
		return false;
	}

	CurrentSaveGame = PTBSave;
	CachedSettings = PTBSave->Settings;

	PTB_RECORD(LogPTBCore, TEXT("SaveGame loaded from slot [%s]"), *SlotName);
	return true;
}

void UPTBGameInstance::AutoSave()
{
	PTB_RECORD(LogPTBCore, TEXT("AutoSave triggered"));
	SaveGame();
}

void UPTBGameInstance::CreateTitleWidget()
{
	PTB_RECORD(LogPTBCore, TEXT("[PTBGameInstance] CreateTitleWidget called"));

	if (!TitleWidgetClass)
	{
		PTB_WARNING(LogPTBCore, TEXT("[PTBGameInstance] TitleWidgetClass is null"));
		return;
	}

	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC)
	{
		PTB_WARNING(LogPTBCore, TEXT("[PTBGameInstance] PlayerController is null"));
		return;
	}

	PTB_RECORD(LogPTBCore, TEXT("[PTBGameInstance] Creating widget..."));
	TitleWidgetInstance = CreateWidget<UPTBMainTitleWidget>(PC, TitleWidgetClass);
	if (TitleWidgetInstance)
	{
		PTB_RECORD(LogPTBCore, TEXT("[PTBGameInstance] Widget created successfully"));
		TitleWidgetInstance->AddToViewport();
	}
}