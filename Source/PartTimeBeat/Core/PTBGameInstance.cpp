#include "Core/PTBGameInstance.h"
#include "Core/PTBSaveGame.h"
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

	// 타이틀 위젯 생성 및 표시
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UPTBGameInstance::CreateTitleWidget));
	}
	else
	{
		PTB_WARNING(LogPTBCore, TEXT("[PTBGameInstance] InitPTBSystems: World is null, widget creation skipped"));
	}
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

	PTB_RECORD(LogPTBCore, TEXT("UserSettings applied — Master:%.2f BGM:%.2f SFX:%.2f Offset:%.1fms"),
		InSettings.MasterVolume, InSettings.BGMVolume,
		InSettings.SFXVolume, InSettings.JudgementOffsetMs);
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