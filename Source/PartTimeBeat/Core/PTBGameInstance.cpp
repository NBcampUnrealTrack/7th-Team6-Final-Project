#include "Core/PTBGameInstance.h"
#include "Audio/PTBWwiseAudioManager.h"
#include "Core/PTBSaveGame.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "UI/PTBMainTitleWidget.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/TextLocalizationManager.h"
#include "HAL/IConsoleManager.h"
#include "Core/PTBInputPollSubsystem.h"

namespace
{
	/** 그래픽 자동 설정 로직 버전. 로직을 바꿔서 다시 측정하게 하려면 올린다 */
	constexpr int32 GraphicsAutoDetectLogicVersion = 1;
}

// 내부에서 SaveGame 오브젝트를 보관할 멤버가 헤더에 없으므로
// 헤더에 아래를 추가하는 것을 권장합니다:
//   UPROPERTY() UPTBSaveGame* CurrentSaveGame;

void UPTBGameInstance::Init()
{
	Super::Init();
	// 첫 실행 언어: OS 언어가 지원 언어(ko/en)면 그대로, 아니면 영어 (패키징/Standalone만. 에디터는 영향 없음)
	ApplyDefaultLanguage();
	InitPTBSystems();
}

void UPTBGameInstance::Shutdown()
{
#if WITH_EDITOR
	// PIE 종료: 게임 언어 미리보기를 켰었다면 해제 (에디터는 원래 상태로)
	if (GIsEditor && !EditorPreviewCulture.IsEmpty())
	{
		FTextLocalizationManager::Get().DisableGameLocalizationPreview();
		EditorPreviewCulture.Reset();
	}
#endif
	Super::Shutdown();
}

void UPTBGameInstance::ApplyDefaultLanguage()
{
	// 유저가 옵션에서 고른 적이 있으면 엔진이 이미 그 언어로 시작했으므로 그대로 둠
	FString Saved;
	if (GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Saved, GGameUserSettingsIni)
		&& !Saved.IsEmpty())
	{

		return;
	}

	// 처음 실행: 엔진이 OS 언어로 시작함. 지원하지 않는 언어면 영어로 대체 (저장하지 않음)
	if (!GIsEditor && !GetSupportedLanguages().Contains(GetGameLanguage()))
	{
		FInternationalization::Get().SetCurrentCulture(TEXT("en"));
	}
}


void UPTBGameInstance::InitPTBSystems()
{
	// 1) 세이브 로드 시도
	const bool bLoadedExistingSave = LoadGame();
	if (!bLoadedExistingSave)
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

	// 1-1) 첫 실행이면 사양에 맞춰 기본 그래픽 품질 결정
	AutoDetectGraphicsIfNeeded(!bLoadedExistingSave);

	// 2) AudioManager 초기화
	AudioManager = NewObject<UPTBWwiseAudioManager>(this);

	// 3) 캐시된 설정 적용
	ApplyUserSettings(CachedSettings);

	// 4) 초기 플로우 상태
	CurrentFlowState = EGameFlowState::MainMenu;
	OnFlowStateChanged.Broadcast(CurrentFlowState);

	PTB_RECORD(LogPTBCore, TEXT("PTBSystems initialized"));
}

void UPTBGameInstance::AutoDetectGraphicsIfNeeded(bool bIsNewSave)
{
	if (CachedSettings.GraphicsAutoDetectVersion >= GraphicsAutoDetectLogicVersion)
	{
		return; // 이미 자동 설정함 → 플레이어 설정 유지
	}

	if (!bIsNewSave || GIsEditor)
	{
		// 기존 세이브(이미 플레이하던 사람) 또는 에디터: 현재 값을 유지하고 측정한 것으로 표시
		CachedSettings.GraphicsQuality = FMath::Clamp(CachedSettings.GraphicsQuality, 0, MaxGraphicsQuality);
		CachedSettings.GraphicsAutoDetectVersion = GraphicsAutoDetectLogicVersion;
		PTB_RECORD(LogPTBCore, TEXT("[Graphics] 자동 설정 건너뜀 (%s) → 현재 품질 %d 유지"),
			GIsEditor ? TEXT("에디터") : TEXT("기존 세이브"), CachedSettings.GraphicsQuality);
		return;
	}

	const int32 Recommended = RunGraphicsBenchmark();
	CachedSettings.GraphicsQuality = Recommended;
	CachedSettings.DetectedGraphicsQuality = Recommended;
	CachedSettings.bAntiAliasingEnabled = true;
	CachedSettings.GraphicsAutoDetectVersion = GraphicsAutoDetectLogicVersion;
}

int32 UPTBGameInstance::RunGraphicsBenchmark()
{
	UGameUserSettings* GUS = UGameUserSettings::GetGameUserSettings();
	if (!GUS)
	{
		return MaxAutoGraphicsQuality;
	}

	// CPU/GPU 짧게 측정 (1~2초). 결과는 엔진 품질 단계(0~4)로 각 항목에 채워짐
	GUS->RunHardwareBenchmark();

	// 체감 성능에 영향이 큰 항목 중 가장 낮은 단계를 전체 품질로 사용 (보수적으로)
	int32 Level = FMath::Min3(GUS->GetGlobalIlluminationQuality(), GUS->GetShadowQuality(), GUS->GetPostProcessingQuality());
	Level = FMath::Min(Level, GUS->GetVisualEffectQuality());
	const int32 Recommended = FMath::Clamp(Level, 0, MaxAutoGraphicsQuality);

	PTB_RECORD(LogPTBCore, TEXT("[Graphics] 하드웨어 측정 CPU=%.1f GPU=%.1f → 엔진 단계 %d → 권장 품질 %d"),
		GUS->GetLastCPUBenchmarkResult(), GUS->GetLastGPUBenchmarkResult(), Level, Recommended);
	return Recommended;
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

	if (AudioManager) { AudioManager->ApplySettings(InSettings); }

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
		const int32 ClampedGraphicsQuality = FMath::Clamp(InSettings.GraphicsQuality, 0, MaxGraphicsQuality);
		CachedSettings.GraphicsQuality = ClampedGraphicsQuality;
		GUS->SetOverallScalabilityLevel(ClampedGraphicsQuality);

		// 프레임 제한 / 수직 동기화 (GameUserSettings.ini에도 저장됨)
		GUS->SetFrameRateLimit(static_cast<float>(FMath::Max(0, InSettings.FrameRateLimit)));
		GUS->SetVSyncEnabled(InSettings.bVSyncEnabled);

		GUS->ApplySettings(false);
	}

	// GameUserSettings는 낮은 우선순위로 값을 넣어서, 로딩 화면(SetByCode)·프로젝트 설정에 밀릴 수 있으므로 직접 한 번 더 설정
	if (IConsoleVariable* MaxFPSVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS")))
	{
		MaxFPSVar->Set(static_cast<float>(FMath::Max(0, InSettings.FrameRateLimit)), ECVF_SetByCode);
	}
	if (IConsoleVariable* VSyncVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.VSync")))
	{
		VSyncVar->Set(InSettings.bVSyncEnabled ? 1 : 0, ECVF_SetByGameOverride);
	}

	// 입력 폴링 레이트·키 바인딩 반영
	if (UPTBInputPollSubsystem* InputPoll = GetSubsystem<UPTBInputPollSubsystem>())
	{
		InputPoll->Configure(InSettings.InputPollingRateHz, InSettings.RhythmKeys);
	}

	// 안티에일리어싱: 켬 = FXAA(1), 끔 = 없음(0). 다른 값(TAA/TSR/MSAA)은 쓰지 않음
	// 우선순위: GameSetting은 프로젝트 설정(DefaultEngine.ini의 r.AntiAliasingMethod=0)보다 낮아 무시되므로
	// GameOverride로 설정한다 (프로젝트 설정·디바이스 프로필보다 높고, 콘솔 직접 입력보다는 낮음)
	if (IConsoleVariable* AAVar = IConsoleManager::Get().FindConsoleVariable(TEXT("r.AntiAliasingMethod")))
	{
		AAVar->Set(InSettings.bAntiAliasingEnabled ? 1 : 0, ECVF_SetByGameOverride);
	}

	SaveGame();

	PTB_RECORD(LogPTBCore, TEXT("UserSettings applied — Master:%.2f BGM:%.2f SFX:%.2f Offset:%.1fms / WinMode:%d / Resolution:%d / Graphics:%d"),
		InSettings.MasterVolume, InSettings.BGMVolume, InSettings.SFXVolume, InSettings.JudgementOffsetMs,
		static_cast<int32>(InSettings.WindowMode), InSettings.ResolutionPresetIndex, CachedSettings.GraphicsQuality);
	PTB_RECORD(LogPTBCore, TEXT("UserSettings applied — AntiAliasing:%s / MaxFPS:%d / VSync:%d / InputPolling:%dHz"),
		InSettings.bAntiAliasingEnabled ? TEXT("FXAA") : TEXT("Off"), InSettings.FrameRateLimit,
		InSettings.bVSyncEnabled ? 1 : 0, InSettings.InputPollingRateHz);
	OnUserSettingsApplied.Broadcast();
}

void UPTBGameInstance::SetGameLanguage(const FString& Culture)
{
#if WITH_EDITOR
	// 에디터(PIE)는 에디터와 같은 프로세스라 SetCurrentCulture가 에디터 UI 언어까지 바꿔버림.
	// → 게임 텍스트만 바꾸는 "게임 언어 미리보기"로 적용 (에디터 언어/설정 파일은 그대로)
	if (GIsEditor)
	{
		FTextLocalizationManager::Get().EnableGameLocalizationPreview(Culture);
		EditorPreviewCulture = Culture;
		return;
	}
#endif

	// 패키징/Standalone: SaveToConfig=true → GameUserSettings.ini에 저장되어 다음 실행부터 이 언어로 시작
	UKismetInternationalizationLibrary::SetCurrentCulture(Culture, /*SaveToConfig*/ true);
}

FString UPTBGameInstance::GetGameLanguage() const
{
#if WITH_EDITOR
	if (GIsEditor)
	{
		// 에디터(PIE)에서는 에디터 언어(GetCurrentCulture)가 아니라 "게임 텍스트가 실제로 보이는 언어"를 반환
		FString PreviewCulture = EditorPreviewCulture;
		FTextLocalizationManager& LocManager = FTextLocalizationManager::Get();
		if (PreviewCulture.IsEmpty() && LocManager.IsGameLocalizationPreviewEnabled())
		{
			// Editor Preferences → Preview Game Language로 켜둔 경우
			PreviewCulture = LocManager.GetConfiguredGameLocalizationPreviewLanguage();
		}
		if (PreviewCulture.IsEmpty())
		{
			// 미리보기가 꺼져 있으면 게임 텍스트는 원문(Native, 이 프로젝트는 ko) 그대로 보임
			PreviewCulture = LocManager.GetNativeCultureName(ELocalizedTextSourceCategory::Game);
		}

		FString Language;
		if (!PreviewCulture.Split(TEXT("-"), &Language, nullptr))
		{
			Language = PreviewCulture;
		}
		if (!Language.IsEmpty())
		{
			return Language;
		}
	}
#endif
	return FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
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