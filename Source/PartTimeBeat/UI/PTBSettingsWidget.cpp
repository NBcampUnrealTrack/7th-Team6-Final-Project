// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBSettingsWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Components/Widget.h"
#include "Core/PTBGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetInternationalizationLibrary.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"

// C++ 표시 문구는 LOCTEXT로 작성 → Gather from Text Files(Source/*)로 수집됨
#define LOCTEXT_NAMESPACE "PTBSettings"

UPTBSettingsWidget::UPTBSettingsWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 설정 메뉴는 항상 UI 화면에서 열리므로 닫을 때 UI 입력 모드 유지
	RestoreMode = EPTBMenuRestoreMode::UIOnly;
}

// ── 초기화 ────────────────────────────────────────────────────────

void UPTBSettingsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindWidgetCallbacks();
	InitializeView();
}

void UPTBSettingsWidget::InitializeView()
{
	const UPTBGameInstance* GI = GetGameInstance<UPTBGameInstance>();
	PendingSettings = GI ? GI->CachedSettings : FPTBUserSettings{};
	PendingCulture = UPTBGameInstance::FindSupportedCulture(GetCurrentLanguageCode());

	// ComboBox 옵션 구성 (값 설정은 RefreshAllWidgets에서 일괄 처리)
	BuildComboBoxOptions();

	// 플레이 중(일시정지 상태)에는 언어 변경 비활성화
	// (Format Text로 만든 HUD 텍스트가 이전 언어로 남아 화면에 두 언어가 섞이는 것을 방지)
	const bool bLanguageLocked = UGameplayStatics::IsGamePaused(this);
	if (ComboBoxLanguage) ComboBoxLanguage->SetIsEnabled(!bLanguageLocked);

	// 지원 언어가 1개뿐이면(예: 한국어만 패키징) 언어 항목 전체를 숨김
	const bool bShowLanguage = LanguageCultures.Num() >= 2;
	if (LanguageRow)
	{
		LanguageRow->SetVisibility(bShowLanguage ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (TextLanguageLocked)
	{
		TextLanguageLocked->SetVisibility(bShowLanguage && bLanguageLocked ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (SliderMaster) { SliderMaster->SetMinValue(0.f); SliderMaster->SetMaxValue(1.f); }
	if (SliderBgm) { SliderBgm->SetMinValue(0.f);   SliderBgm->SetMaxValue(1.f); }
	if (SliderSfx) { SliderSfx->SetMinValue(0.f);   SliderSfx->SetMaxValue(1.f); }
	if (SliderSyncOffset) { SliderSyncOffset->SetMinValue(SyncOffsetMinMs); SliderSyncOffset->SetMaxValue(SyncOffsetMaxMs); SliderSyncOffset->SetStepSize(1.f); }

	// 알림 텍스트 / 재바인딩 프롬프트 초기 숨김
	if (TextNotification) TextNotification->SetVisibility(ESlateVisibility::Hidden);
	if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::Hidden);

	// 첫 탭(그래픽)으로 시작
	SwitchToTab(0);

	RefreshAllWidgets();
}

void UPTBSettingsWidget::BuildComboBoxOptions()
{
	// ComboBoxString은 FString만 받으므로 LOCTEXT(...).ToString()으로 현재 언어 문자열을 넣는다.
	// 언어가 바뀌면 이 함수를 다시 호출해야 옵션이 새 언어로 바뀜.
	if (ComboBoxWindowMode)
	{
		ComboBoxWindowMode->ClearOptions();
		ComboBoxWindowMode->AddOption(LOCTEXT("WindowMode_Fullscreen", "전체화면").ToString());
		ComboBoxWindowMode->AddOption(LOCTEXT("WindowMode_WindowedFullscreen", "창 전체화면").ToString());
		ComboBoxWindowMode->AddOption(LOCTEXT("WindowMode_Windowed", "창 모드").ToString());
	}
	if (ComboBoxResolution)
	{
		ComboBoxResolution->ClearOptions();
		for (const FString& Name : PTBResolution::GetPresetNames())
		{
			ComboBoxResolution->AddOption(Name);
		}
	}
	if (ComboBoxGraphics)
	{
		ComboBoxGraphics->ClearOptions();
		ComboBoxGraphics->AddOption(LOCTEXT("Graphics_Low", "낮음").ToString());
		ComboBoxGraphics->AddOption(LOCTEXT("Graphics_Medium", "중간").ToString());
		ComboBoxGraphics->AddOption(LOCTEXT("Graphics_High", "높음").ToString());
		ComboBoxGraphics->AddOption(LOCTEXT("Graphics_Epic", "최고").ToString());
	}
	if (ComboBoxFrameRate)
	{
		ComboBoxFrameRate->ClearOptions();
		for (const int32 Fps : FrameRateOptions)
		{
			if (Fps == FrameRateOptionVSync)
			{
				ComboBoxFrameRate->AddOption(LOCTEXT("FrameRate_VSync", "수직 동기화 (V-Sync)").ToString());
			}
			else
			{
				ComboBoxFrameRate->AddOption(Fps > 0
					? FString::Printf(TEXT("%d FPS"), Fps)
					: LOCTEXT("FrameRate_Unlimited", "무제한").ToString());
			}
		}
	}
	if (ComboBoxPollingRate)
	{
		ComboBoxPollingRate->ClearOptions();
		for (const int32 Hz : PollingRateOptions)
		{
			ComboBoxPollingRate->AddOption(Hz > 0
				? FString::Printf(TEXT("%d Hz"), Hz)
				: LOCTEXT("PollingRate_Off", "끔 (프레임 기준)").ToString());
		}
	}
	if (ComboBoxLanguage)
	{
		// 언어 이름은 번역하지 않음 (어떤 언어 상태에서도 자기 언어를 찾을 수 있도록)
		// 지원 언어를 자동으로 수집해서 각 언어의 자기 언어 이름으로 표시 (한국어, English, 日本語, 中文(简体) ...)
		LanguageCultures = UPTBGameInstance::GetSupportedLanguages();
		ComboBoxLanguage->ClearOptions();
		for (const FString& CultureName : LanguageCultures)
		{
			const FCulturePtr Culture = FInternationalization::Get().GetCulture(CultureName);
			ComboBoxLanguage->AddOption(Culture.IsValid() ? Culture->GetNativeName() : CultureName);
		}
	}
}

FString UPTBSettingsWidget::GetCurrentLanguageCode() const
{
	// 언어 적용/조회는 GameInstance로 통일 (에디터에서는 에디터 언어 대신 게임 언어 미리보기 사용)
	if (const UPTBGameInstance* GI = GetGameInstance<UPTBGameInstance>())
	{
		return GI->GetGameLanguage();
	}
	return FInternationalization::Get().GetCurrentCulture()->GetTwoLetterISOLanguageName();
}

// ── 탭 전환 ───────────────────────────────────────────────────────

void UPTBSettingsWidget::SwitchToTab(int32 TabIndex)
{
	const int32 MaxIndex = TabSwitcher ? FMath::Max(0, TabSwitcher->GetNumWidgets() - 1) : 3;
	const int32 ClampedIndex = FMath::Clamp(TabIndex, 0, MaxIndex);

	if (TabSwitcher)
	{
		TabSwitcher->SetActiveWidgetIndex(ClampedIndex);
	}

	// 탭 인디케이터 라인: 선택된 탭만 표시
	UImage* TabLines[] = { ImgTabLine_Graphics, ImgTabLine_Sounds, ImgTabLine_Keys, ImgTabLine_Game };
	for (int32 i = 0; i < UE_ARRAY_COUNT(TabLines); ++i)
	{
		if (TabLines[i])
		{
			TabLines[i]->SetVisibility(i == ClampedIndex ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		}
	}

	// 탭 전환 시 재바인딩 중이면 취소
	if (PendingRebindAction != EPTBActionType::None)
	{
		const EPTBActionType CancelledAction = PendingRebindAction;
		PendingRebindAction = EPTBActionType::None;
		if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::Hidden);
		OnRebindCancelled(CancelledAction);
	}

	OnTabChanged(ClampedIndex);
}

// ── 설정 적용 / 초기화 ────────────────────────────────────────────

void UPTBSettingsWidget::ApplySettings()
{
	if (UPTBGameInstance* GI = GetGameInstance<UPTBGameInstance>())
	{
		GI->ApplyUserSettings(PendingSettings);

		// 언어 변경: 즉시 적용 + GameUserSettings.ini에 저장 (다음 실행부터 이 언어로 시작)
		if (!PendingCulture.IsEmpty() && PendingCulture != UPTBGameInstance::FindSupportedCulture(GetCurrentLanguageCode()))
		{
			GI->SetGameLanguage(PendingCulture);

			// 디자이너에 입력된 텍스트(탭 이름 등)는 자동 갱신됨.
			// C++에서 문자열로 넣은 ComboBox 옵션/키 이름은 직접 다시 구성.
			BuildComboBoxOptions();
			RefreshAllWidgets();
		}

		OnSettingsApplied(); // 언어 변경 후 호출 → 알림 문구도 새 언어로 표시
	}
}

void UPTBSettingsWidget::ResetToDefaults()
{
	// 자동 설정 정보(측정 결과)는 유지하고, 그래픽 품질은 이 PC의 권장값으로 되돌림
	const FPTBUserSettings Previous = PendingSettings;
	PendingSettings = FPTBUserSettings{};
	PendingSettings.DetectedGraphicsQuality = Previous.DetectedGraphicsQuality;
	PendingSettings.GraphicsAutoDetectVersion = Previous.GraphicsAutoDetectVersion;
	if (Previous.DetectedGraphicsQuality >= 0)
	{
		PendingSettings.GraphicsQuality = Previous.DetectedGraphicsQuality;
	}
	// 언어는 초기화 대상에서 제외 (초기화로 읽을 수 없는 언어가 되는 것을 방지). 선택만 현재 언어로 되돌림.
	PendingCulture = UPTBGameInstance::FindSupportedCulture(GetCurrentLanguageCode());
	RefreshAllWidgets();
	OnSettingsReset();
}

// ── 알림 텍스트 ───────────────────────────────────────────────────

void UPTBSettingsWidget::ShowNotification(const FText& Message)
{
	if (!TextNotification) return;

	TextNotification->SetText(Message);
	TextNotification->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 이전 타이머 취소 후 2초 뒤 숨김
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			NotificationTimerHandle,
			this, &UPTBSettingsWidget::HideNotification,
			2.f, false);
	}
}

void UPTBSettingsWidget::HideNotification()
{
	if (TextNotification)
	{
		TextNotification->SetVisibility(ESlateVisibility::Hidden);
	}
}

// ── BlueprintNativeEvent 기본 구현 ────────────────────────────────

void UPTBSettingsWidget::OnSettingsApplied_Implementation()
{
	ShowNotification(LOCTEXT("Notify_Saved", "설정이 저장되었습니다."));
}

void UPTBSettingsWidget::OnSettingsReset_Implementation()
{
	ShowNotification(LOCTEXT("Notify_Reset", "기본값으로 초기화되었습니다."));
}

// ── 키 입력 처리 ──────────────────────────────────────────────────

FReply UPTBSettingsWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	// 재바인딩 대기 중 ESC → 취소 후 부모의 CloseMenu 차단
	if (PendingRebindAction != EPTBActionType::None && InKeyEvent.GetKey() == EKeys::Escape)
	{
		const EPTBActionType CancelledAction = PendingRebindAction;
		CancelRebind();
		OnRebindCancelled(CancelledAction);
		return FReply::Handled();
	}
	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

// ── 키 재바인딩 ───────────────────────────────────────────────────

void UPTBSettingsWidget::RequestRebind(EPTBActionType Action)
{
	if (Action == EPTBActionType::None) return;
	PendingRebindAction = Action;
	if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::HitTestInvisible);
	SetKeyboardFocus();
	OnRebindStarted(Action);
}

void UPTBSettingsWidget::CancelRebind()
{
	PendingRebindAction = EPTBActionType::None;
	if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::Hidden);
}

FReply UPTBSettingsWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (PendingRebindAction == EPTBActionType::None)
	{
		return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
	}

	const FKey PressedKey = InKeyEvent.GetKey();

	// Escape → 재바인딩 취소
	if (PressedKey == EKeys::Escape)
	{
		const EPTBActionType CancelledAction = PendingRebindAction;
		CancelRebind();
		OnRebindCancelled(CancelledAction);
		return FReply::Handled();
	}

	// 수정자 단독 키는 리듬 게임 입력으로 부적합 → 무시
	if (PressedKey == EKeys::LeftControl || PressedKey == EKeys::RightControl ||
		PressedKey == EKeys::LeftShift || PressedKey == EKeys::RightShift ||
		PressedKey == EKeys::LeftAlt || PressedKey == EKeys::RightAlt)
	{
		return FReply::Handled();
	}

	// 충돌 검사 → 이전 키를 충돌 액션에 자동 스왑
	const EPTBActionType ConflictAction = PendingSettings.RhythmKeys.ResolveKey(PressedKey);
	if (ConflictAction != EPTBActionType::None && ConflictAction != PendingRebindAction)
	{
		const FKey OldKey = PendingSettings.RhythmKeys.ResolveAction(PendingRebindAction);
		PendingSettings.RhythmKeys.SetKey(ConflictAction, OldKey);
		RefreshKeyText(ConflictAction);
		OnKeyConflictDetected(ConflictAction);
	}

	PendingSettings.RhythmKeys.SetKey(PendingRebindAction, PressedKey);
	RefreshKeyText(PendingRebindAction);

	const EPTBActionType CompletedAction = PendingRebindAction;
	PendingRebindAction = EPTBActionType::None;
	if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::Hidden);
	OnRebindCompleted(CompletedAction, PressedKey);

	return FReply::Handled();
}

// ── 위젯 갱신 ────────────────────────────────────────────────────

void UPTBSettingsWidget::RefreshAllWidgets()
{
	if (SliderMaster) SliderMaster->SetValue(PendingSettings.MasterVolume);
	if (SliderBgm)    SliderBgm->SetValue(PendingSettings.BGMVolume);
	if (SliderSfx)    SliderSfx->SetValue(PendingSettings.SFXVolume);
	RefreshVolumeText(TextMasterValue, PendingSettings.MasterVolume);
	RefreshVolumeText(TextBgmValue, PendingSettings.BGMVolume);
	RefreshVolumeText(TextSfxValue, PendingSettings.SFXVolume);
	if (CheckBoxGameplaySfx) CheckBoxGameplaySfx->SetIsChecked(PendingSettings.bGameplaySFXEnabled);

	RefreshSyncOffsetWidgets();

	if (ComboBoxWindowMode)
	{
		const int32 ModeIndex = FMath::Clamp(static_cast<int32>(PendingSettings.WindowMode), 0, 2);
		PendingSettings.WindowMode = static_cast<EPTBWindowMode>(ModeIndex);
		ComboBoxWindowMode->SetSelectedIndex(ModeIndex);
	}
	if (ComboBoxResolution)
	{
		const int32 ResIndex = FMath::Clamp(PendingSettings.ResolutionPresetIndex, 0, PTBResolution::PresetCount - 1);
		PendingSettings.ResolutionPresetIndex = ResIndex;
		ComboBoxResolution->SetSelectedIndex(ResIndex);
	}
	if (ComboBoxGraphics)
	{
		const int32 GraphicsIndex = FMath::Clamp(PendingSettings.GraphicsQuality, 0, UPTBGameInstance::MaxGraphicsQuality);
		PendingSettings.GraphicsQuality = GraphicsIndex;
		ComboBoxGraphics->SetSelectedIndex(GraphicsIndex);
	}
	if (CheckBoxAntiAliasing) CheckBoxAntiAliasing->SetIsChecked(PendingSettings.bAntiAliasingEnabled);
	if (ComboBoxFrameRate && FrameRateOptions.Num() > 0)
	{
		const int32 VSyncIndex = FrameRateOptions.IndexOfByKey(FrameRateOptionVSync);
		if (PendingSettings.bVSyncEnabled && VSyncIndex != INDEX_NONE)
		{
			ComboBoxFrameRate->SetSelectedIndex(VSyncIndex);
		}
		else
		{
			// 목록에 수직 동기화가 없으면 끈 상태로 맞춤
			PendingSettings.bVSyncEnabled = false;
			const int32 Index = FindClosestOptionIndex(FrameRateOptions, PendingSettings.FrameRateLimit);
			PendingSettings.FrameRateLimit = FMath::Max(0, FrameRateOptions[Index]);
			ComboBoxFrameRate->SetSelectedIndex(Index);
		}
	}
	if (ComboBoxPollingRate && PollingRateOptions.Num() > 0)
	{
		const int32 Index = FindClosestOptionIndex(PollingRateOptions, PendingSettings.InputPollingRateHz);
		PendingSettings.InputPollingRateHz = PollingRateOptions[Index];
		ComboBoxPollingRate->SetSelectedIndex(Index);
	}
	if (ComboBoxLanguage)
	{
		int32 LangIndex = LanguageCultures.IndexOfByKey(UPTBGameInstance::FindSupportedCulture(PendingCulture));
		if (LangIndex == INDEX_NONE) LangIndex = LanguageCultures.IndexOfByKey(TEXT("en"));
		if (LangIndex == INDEX_NONE) LangIndex = 0;
		ComboBoxLanguage->SetSelectedIndex(LangIndex);
	}

	for (EPTBActionType Action : {
		EPTBActionType::ActionA, EPTBActionType::ActionB, EPTBActionType::ActionC,
			EPTBActionType::ActionD, EPTBActionType::ActionE })
	{
		RefreshKeyText(Action);
	}
}

void UPTBSettingsWidget::RefreshKeyText(EPTBActionType Action)
{
	const FKey CurrentKey = PendingSettings.RhythmKeys.ResolveAction(Action);
	// 키 이름은 엔진이 언어별로 제공하는 FText 그대로 사용 (예: Space → 스페이스 바)
	const FText KeyName = CurrentKey.GetDisplayName();

	auto SetText = [&](UTextBlock* Text) {
		if (Text) Text->SetText(KeyName);
		};

	switch (Action)
	{
	case EPTBActionType::ActionA: SetText(KeyText_A); break;
	case EPTBActionType::ActionB: SetText(KeyText_B); break;
	case EPTBActionType::ActionC: SetText(KeyText_C); break;
	case EPTBActionType::ActionD: SetText(KeyText_D); break;
	case EPTBActionType::ActionE: SetText(KeyText_E); break;
	default: break;
	}
}

void UPTBSettingsWidget::RefreshVolumeText(UTextBlock* TextWidget, float Value)
{
	if (!TextWidget) return;
	TextWidget->SetText(FText::FromString(
		FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Value * 100.f))));
}

// ── 버튼/슬라이더 델리게이트 바인딩 ────────────────────────────────

void UPTBSettingsWidget::BindWidgetCallbacks()
{
	if (ButtonClose)        ButtonClose->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnCloseClicked);
	if (ButtonTabSounds)    ButtonTabSounds->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnTabSoundsClicked);
	if (ButtonTabGraphics)  ButtonTabGraphics->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnTabGraphicsClicked);
	if (ButtonTabKeys)      ButtonTabKeys->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnTabKeysClicked);
	if (ButtonTabGame)      ButtonTabGame->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnTabGameClicked);

	if (ButtonApply)   ButtonApply->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnApplyClicked);
	if (ButtonReset)   ButtonReset->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnResetClicked);
	if (ButtonRebindA) ButtonRebindA->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindAClicked);
	if (ButtonRebindB) ButtonRebindB->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindBClicked);
	if (ButtonRebindC) ButtonRebindC->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindCClicked);
	if (ButtonRebindD) ButtonRebindD->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindDClicked);
	if (ButtonRebindE) ButtonRebindE->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindEClicked);

	if (SliderMaster)     SliderMaster->OnValueChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnMasterSliderChanged);
	if (SliderBgm)        SliderBgm->OnValueChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnBgmSliderChanged);
	if (CheckBoxGameplaySfx) CheckBoxGameplaySfx->OnCheckStateChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnGameplaySfxCheckChanged);
	if (SliderSfx)        SliderSfx->OnValueChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnSfxSliderChanged);
	if (SliderSyncOffset) SliderSyncOffset->OnValueChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnSyncOffsetSliderChanged);
	if (EditSyncOffset)
	{
		EditSyncOffset->OnTextCommitted.AddUniqueDynamic(this, &UPTBSettingsWidget::OnSyncOffsetTextCommitted);
		EditSyncOffset->OnTextChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnSyncOffsetTextChanged);
	}
	if (ButtonResetSyncOffset) ButtonResetSyncOffset->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnResetSyncOffsetClicked);

	if (ComboBoxWindowMode) ComboBoxWindowMode->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnWindowModeSelectionChanged);
	if (ComboBoxResolution) ComboBoxResolution->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnResolutionSelectionChanged);
	if (ComboBoxGraphics)   ComboBoxGraphics->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnGraphicsSelectionChanged);
	if (CheckBoxAntiAliasing) CheckBoxAntiAliasing->OnCheckStateChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnAntiAliasingCheckChanged);
	if (ComboBoxFrameRate)    ComboBoxFrameRate->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnFrameRateSelectionChanged);
	if (ComboBoxPollingRate)  ComboBoxPollingRate->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnPollingRateSelectionChanged);
	if (ComboBoxLanguage)   ComboBoxLanguage->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnLanguageSelectionChanged);
}

void UPTBSettingsWidget::OnCloseClicked() { CloseMenu(); }
void UPTBSettingsWidget::OnTabGraphicsClicked() { SwitchToTab(0); }
void UPTBSettingsWidget::OnTabSoundsClicked() { SwitchToTab(1); }
void UPTBSettingsWidget::OnTabKeysClicked() { SwitchToTab(2); }
void UPTBSettingsWidget::OnTabGameClicked() { SwitchToTab(3); }
void UPTBSettingsWidget::OnApplyClicked() { ApplySettings(); }
void UPTBSettingsWidget::OnResetClicked() { ResetToDefaults(); }
void UPTBSettingsWidget::OnRebindAClicked() { RequestRebind(EPTBActionType::ActionA); }
void UPTBSettingsWidget::OnRebindBClicked() { RequestRebind(EPTBActionType::ActionB); }
void UPTBSettingsWidget::OnRebindCClicked() { RequestRebind(EPTBActionType::ActionC); }
void UPTBSettingsWidget::OnRebindDClicked() { RequestRebind(EPTBActionType::ActionD); }
void UPTBSettingsWidget::OnRebindEClicked() { RequestRebind(EPTBActionType::ActionE); }

void UPTBSettingsWidget::OnMasterSliderChanged(float Value)
{
	PendingSettings.MasterVolume = Value;
	RefreshVolumeText(TextMasterValue, Value);
}

void UPTBSettingsWidget::OnBgmSliderChanged(float Value)
{
	PendingSettings.BGMVolume = Value;
	RefreshVolumeText(TextBgmValue, Value);
}

void UPTBSettingsWidget::OnSfxSliderChanged(float Value)
{
	PendingSettings.SFXVolume = Value;
	RefreshVolumeText(TextSfxValue, Value);
}

void UPTBSettingsWidget::OnGameplaySfxCheckChanged(bool bIsChecked)
{
	PendingSettings.bGameplaySFXEnabled = bIsChecked;
}

void UPTBSettingsWidget::OnSyncOffsetSliderChanged(float Value)
{
	// 1ms 단위로 맞춤 (슬라이더/입력칸/저장값이 항상 같은 숫자)
	PendingSettings.JudgementOffsetMs = FMath::Clamp(static_cast<float>(FMath::RoundToInt(Value)), SyncOffsetMinMs, SyncOffsetMaxMs);
	RefreshSyncOffsetWidgets(false); // 드래그 중인 슬라이더는 건드리지 않음
}

void UPTBSettingsWidget::OnSyncOffsetTextChanged(const FText& Text)
{
	if (bUpdatingSyncText || !EditSyncOffset) return;

	// 정수만 입력: 맨 앞의 +/- 하나와 숫자만 남김 (소수점·문자 등은 입력 즉시 지워짐)
	const FString Source = Text.ToString();
	FString Sign;
	FString Digits;
	for (int32 i = 0; i < Source.Len(); ++i)
	{
		const TCHAR Ch = Source[i];
		if (FChar::IsDigit(Ch))
		{
			Digits.AppendChar(Ch);
		}
		else if ((Ch == TEXT('-') || Ch == TEXT('+')) && Sign.IsEmpty() && Digits.IsEmpty())
		{
			Sign.AppendChar(Ch);
		}
	}

	// 앞자리 0 정리 (007 → 7)
	while (Digits.Len() > 1 && Digits[0] == TEXT('0'))
	{
		Digits.RightChopInline(1);
	}

	// 범위 밖(±200 초과)이면 입력 즉시 최대값으로 맞춤
	const int32 MaxAbs = FMath::RoundToInt(SyncOffsetMaxMs);
	if (!Digits.IsEmpty() && (Digits.Len() > 3 || FCString::Atoi(*Digits) > MaxAbs))
	{
		Digits = FString::FromInt(MaxAbs);
	}

	const FString Filtered = Sign + Digits;
	if (Filtered != Source)
	{
		TGuardValue<bool> Guard(bUpdatingSyncText, true);
		EditSyncOffset->SetText(FText::FromString(Filtered));
	}

	// 숫자가 있으면 입력하는 동안 슬라이더도 같이 움직임 (입력칸 내용은 그대로 둠)
	if (!Digits.IsEmpty())
	{
		const int32 Value = FCString::Atoi(*Digits) * (Sign == TEXT("-") ? -1 : 1);
		PendingSettings.JudgementOffsetMs = static_cast<float>(FMath::Clamp(Value, -MaxAbs, MaxAbs));
		if (SliderSyncOffset)
		{
			SliderSyncOffset->SetValue(PendingSettings.JudgementOffsetMs);
		}
	}
}

void UPTBSettingsWidget::OnSyncOffsetTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (bUpdatingSyncText) return;

	FString Str = Text.ToString().TrimStartAndEnd();
	Str.ReplaceInline(TEXT("ms"), TEXT(""), ESearchCase::IgnoreCase);
	Str.ReplaceInline(TEXT("+"), TEXT(""));
	Str.TrimStartAndEndInline();

	if (!Str.IsEmpty() && Str != TEXT("-") && Str.IsNumeric())
	{
		const float Parsed = FCString::Atof(*Str);
		PendingSettings.JudgementOffsetMs = FMath::Clamp(static_cast<float>(FMath::RoundToInt(Parsed)), SyncOffsetMinMs, SyncOffsetMaxMs);
	}
	// 잘못된 입력이면 이전 값으로 되돌림 / 범위 밖이면 맞춘 값으로 다시 표시
	RefreshSyncOffsetWidgets(true, true);
}

void UPTBSettingsWidget::RefreshSyncOffsetWidgets(bool bUpdateSlider, bool bForceEditText)
{
	const int32 Ms = FMath::RoundToInt(PendingSettings.JudgementOffsetMs);

	if (bUpdateSlider && SliderSyncOffset)
	{
		SliderSyncOffset->SetValue(static_cast<float>(Ms));
	}
	// 입력칸이 있으면 숫자는 입력칸에만 표시하고, TextSyncOffsetValue는 Designer 문구("ms") 그대로 둠
	if (TextSyncOffsetValue && !EditSyncOffset)
	{
		TextSyncOffsetValue->SetText(FText::FromString(
			Ms > 0 ? FString::Printf(TEXT("+%dms"), Ms) : FString::Printf(TEXT("%dms"), Ms)));
	}
	if (EditSyncOffset && (bForceEditText || !EditSyncOffset->HasKeyboardFocus()))
	{
		TGuardValue<bool> Guard(bUpdatingSyncText, true);
		EditSyncOffset->SetText(FText::FromString(
			Ms > 0 ? FString::Printf(TEXT("+%d"), Ms) : FString::Printf(TEXT("%d"), Ms)));
	}
}

void UPTBSettingsWidget::OnResetSyncOffsetClicked()
{
	// 판정 오프셋만 0으로, 즉시 저장 (다른 설정의 Apply 대기값은 건드리지 않음)
	SetSyncOffsetMs(0.f, true);
}

void UPTBSettingsWidget::SetSyncOffsetMs(float OffsetMs, bool bSaveNow)
{
	PendingSettings.JudgementOffsetMs = FMath::Clamp(static_cast<float>(FMath::RoundToInt(OffsetMs)), SyncOffsetMinMs, SyncOffsetMaxMs);
	RefreshSyncOffsetWidgets();

	if (bSaveNow)
	{
		if (UPTBGameInstance* GI = GetGameInstance<UPTBGameInstance>())
		{
			GI->SetJudgementOffsetMs(PendingSettings.JudgementOffsetMs);
			GI->SaveGame();
		}
	}
}

void UPTBSettingsWidget::OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	// ESelectInfo::Direct = 코드 내 SetSelectedIndex 호출 → PendingSettings 변경 불필요
	if (SelectionType == ESelectInfo::Direct || !ComboBoxWindowMode) return;
	PendingSettings.WindowMode = static_cast<EPTBWindowMode>(ComboBoxWindowMode->GetSelectedIndex());
}

void UPTBSettingsWidget::OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct || !ComboBoxResolution) return;
	PendingSettings.ResolutionPresetIndex = ComboBoxResolution->GetSelectedIndex();
}

void UPTBSettingsWidget::OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct || !ComboBoxGraphics) return;
	PendingSettings.GraphicsQuality = FMath::Clamp(ComboBoxGraphics->GetSelectedIndex(), 0, UPTBGameInstance::MaxGraphicsQuality);
}

void UPTBSettingsWidget::OnAntiAliasingCheckChanged(bool bIsChecked)
{
	PendingSettings.bAntiAliasingEnabled = bIsChecked;
}

void UPTBSettingsWidget::OnFrameRateSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct || !ComboBoxFrameRate) return;
	const int32 Index = ComboBoxFrameRate->GetSelectedIndex();
	if (!FrameRateOptions.IsValidIndex(Index))
	{
		return;
	}
	if (FrameRateOptions[Index] == FrameRateOptionVSync)
	{
		// 수직 동기화: 모니터 주사율에 맞춰지므로 별도 프레임 제한은 끔
		PendingSettings.bVSyncEnabled = true;
		PendingSettings.FrameRateLimit = 0;
	}
	else
	{
		PendingSettings.bVSyncEnabled = false;
		PendingSettings.FrameRateLimit = FMath::Max(0, FrameRateOptions[Index]);
	}
}

void UPTBSettingsWidget::OnPollingRateSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct || !ComboBoxPollingRate) return;
	const int32 Index = ComboBoxPollingRate->GetSelectedIndex();
	if (PollingRateOptions.IsValidIndex(Index))
	{
		PendingSettings.InputPollingRateHz = PollingRateOptions[Index];
	}
}

int32 UPTBSettingsWidget::FindClosestOptionIndex(const TArray<int32>& Options, int32 Value)
{
	int32 Best = 0;
	int32 BestDiff = MAX_int32;
	for (int32 i = 0; i < Options.Num(); ++i)
	{
		// 음수(수직 동기화 등 특수값)는 거리 계산에서 제외
		if (Options[i] < 0)
		{
			continue;
		}
		// 0(무제한/끔)은 정확히 같을 때만 선택
		if ((Options[i] == 0) != (Value == 0))
		{
			continue;
		}
		const int32 Diff = FMath::Abs(Options[i] - Value);
		if (Diff < BestDiff)
		{
			BestDiff = Diff;
			Best = i;
		}
	}
	return Best;
}

void UPTBSettingsWidget::OnLanguageSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct || !ComboBoxLanguage) return;
	const int32 Index = ComboBoxLanguage->GetSelectedIndex();
	if (LanguageCultures.IsValidIndex(Index))
	{
		PendingCulture = LanguageCultures[Index]; // Apply 버튼을 눌러야 실제 적용
	}
}

#undef LOCTEXT_NAMESPACE