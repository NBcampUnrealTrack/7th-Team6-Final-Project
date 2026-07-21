// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBSettingsWidget.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Image.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Core/PTBGameInstance.h"

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

	// ComboBox 옵션 구성 (값 설정은 RefreshAllWidgets에서 일괄 처리)
	if (ComboBoxWindowMode)
	{
		ComboBoxWindowMode->ClearOptions();
		ComboBoxWindowMode->AddOption(TEXT("전체화면"));
		ComboBoxWindowMode->AddOption(TEXT("창 전체화면"));
		ComboBoxWindowMode->AddOption(TEXT("창 모드"));
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
		ComboBoxGraphics->AddOption(TEXT("낮음"));
		ComboBoxGraphics->AddOption(TEXT("중간"));
		ComboBoxGraphics->AddOption(TEXT("높음"));
		ComboBoxGraphics->AddOption(TEXT("최고"));
	}
	if (SliderMaster)     { SliderMaster->SetMinValue(0.f); SliderMaster->SetMaxValue(1.f); }
	if (SliderBgm)        { SliderBgm->SetMinValue(0.f);   SliderBgm->SetMaxValue(1.f); }
	if (SliderSfx)        { SliderSfx->SetMinValue(0.f);   SliderSfx->SetMaxValue(1.f); }
	if (SliderSyncOffset) { SliderSyncOffset->SetMinValue(-200.f); SliderSyncOffset->SetMaxValue(200.f); }

	// 알림 텍스트 / 재바인딩 프롬프트 초기 숨김
	if (TextNotification) TextNotification->SetVisibility(ESlateVisibility::Hidden);
	if (TextRebindPrompt) TextRebindPrompt->SetVisibility(ESlateVisibility::Hidden);

	// 첫 탭(그래픽)으로 시작
	SwitchToTab(0);

	RefreshAllWidgets();
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
		OnSettingsApplied();
	}
}

void UPTBSettingsWidget::ResetToDefaults()
{
	PendingSettings = FPTBUserSettings{};
	RefreshAllWidgets();
	OnSettingsReset();
}

// ── 알림 텍스트 ───────────────────────────────────────────────────

void UPTBSettingsWidget::ShowNotification(const FString& Message)
{
	if (!TextNotification) return;

	TextNotification->SetText(FText::FromString(Message));
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
	ShowNotification(TEXT("설정이 저장되었습니다."));
}

void UPTBSettingsWidget::OnSettingsReset_Implementation()
{
	ShowNotification(TEXT("기본값으로 초기화되었습니다."));
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
	if (PressedKey == EKeys::LeftControl  || PressedKey == EKeys::RightControl ||
		PressedKey == EKeys::LeftShift    || PressedKey == EKeys::RightShift   ||
		PressedKey == EKeys::LeftAlt      || PressedKey == EKeys::RightAlt)
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
	RefreshVolumeText(TextBgmValue,    PendingSettings.BGMVolume);
	RefreshVolumeText(TextSfxValue,    PendingSettings.SFXVolume);

	if (SliderSyncOffset) SliderSyncOffset->SetValue(PendingSettings.JudgementOffsetMs);
	if (TextSyncOffsetValue)
	{
		const FString Sign = PendingSettings.JudgementOffsetMs >= 0.f ? TEXT("+") : TEXT("");
		TextSyncOffsetValue->SetText(FText::FromString(
			FString::Printf(TEXT("%s%.0fms"), *Sign, PendingSettings.JudgementOffsetMs)));
	}

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
		const int32 GraphicsIndex = FMath::Clamp(PendingSettings.GraphicsQuality, 0, 3);
		PendingSettings.GraphicsQuality = GraphicsIndex;
		ComboBoxGraphics->SetSelectedIndex(GraphicsIndex);
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
	const FString KeyName = CurrentKey.GetDisplayName().ToString();

	auto SetText = [&](UTextBlock* Text) {
		if (Text) Text->SetText(FText::FromString(KeyName));
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
	if (ButtonClose)        ButtonClose->OnClicked.AddUniqueDynamic(this,        &UPTBSettingsWidget::OnCloseClicked);
	if (ButtonTabSounds)    ButtonTabSounds->OnClicked.AddUniqueDynamic(this,    &UPTBSettingsWidget::OnTabSoundsClicked);
	if (ButtonTabGraphics)  ButtonTabGraphics->OnClicked.AddUniqueDynamic(this,  &UPTBSettingsWidget::OnTabGraphicsClicked);
	if (ButtonTabKeys)      ButtonTabKeys->OnClicked.AddUniqueDynamic(this,      &UPTBSettingsWidget::OnTabKeysClicked);
	if (ButtonTabGame)      ButtonTabGame->OnClicked.AddUniqueDynamic(this,      &UPTBSettingsWidget::OnTabGameClicked);

	if (ButtonApply)   ButtonApply->OnClicked.AddUniqueDynamic(this,   &UPTBSettingsWidget::OnApplyClicked);
	if (ButtonReset)   ButtonReset->OnClicked.AddUniqueDynamic(this,   &UPTBSettingsWidget::OnResetClicked);
	if (ButtonRebindA) ButtonRebindA->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindAClicked);
	if (ButtonRebindB) ButtonRebindB->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindBClicked);
	if (ButtonRebindC) ButtonRebindC->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindCClicked);
	if (ButtonRebindD) ButtonRebindD->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindDClicked);
	if (ButtonRebindE) ButtonRebindE->OnClicked.AddUniqueDynamic(this, &UPTBSettingsWidget::OnRebindEClicked);

	if (SliderMaster)     SliderMaster->OnValueChanged.AddUniqueDynamic(this,     &UPTBSettingsWidget::OnMasterSliderChanged);
	if (SliderBgm)        SliderBgm->OnValueChanged.AddUniqueDynamic(this,        &UPTBSettingsWidget::OnBgmSliderChanged);
	if (SliderSfx)        SliderSfx->OnValueChanged.AddUniqueDynamic(this,        &UPTBSettingsWidget::OnSfxSliderChanged);
	if (SliderSyncOffset) SliderSyncOffset->OnValueChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnSyncOffsetSliderChanged);

	if (ComboBoxWindowMode) ComboBoxWindowMode->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnWindowModeSelectionChanged);
	if (ComboBoxResolution) ComboBoxResolution->OnSelectionChanged.AddUniqueDynamic(this, &UPTBSettingsWidget::OnResolutionSelectionChanged);
	if (ComboBoxGraphics)   ComboBoxGraphics->OnSelectionChanged.AddUniqueDynamic(this,   &UPTBSettingsWidget::OnGraphicsSelectionChanged);
}

void UPTBSettingsWidget::OnCloseClicked()        { CloseMenu(); }
void UPTBSettingsWidget::OnTabGraphicsClicked()  { SwitchToTab(0); }
void UPTBSettingsWidget::OnTabSoundsClicked()    { SwitchToTab(1); }
void UPTBSettingsWidget::OnTabKeysClicked()      { SwitchToTab(2); }
void UPTBSettingsWidget::OnTabGameClicked()      { SwitchToTab(3); }
void UPTBSettingsWidget::OnApplyClicked()        { ApplySettings(); }
void UPTBSettingsWidget::OnResetClicked()        { ResetToDefaults(); }
void UPTBSettingsWidget::OnRebindAClicked()      { RequestRebind(EPTBActionType::ActionA); }
void UPTBSettingsWidget::OnRebindBClicked()      { RequestRebind(EPTBActionType::ActionB); }
void UPTBSettingsWidget::OnRebindCClicked()      { RequestRebind(EPTBActionType::ActionC); }
void UPTBSettingsWidget::OnRebindDClicked()      { RequestRebind(EPTBActionType::ActionD); }
void UPTBSettingsWidget::OnRebindEClicked()      { RequestRebind(EPTBActionType::ActionE); }

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

void UPTBSettingsWidget::OnSyncOffsetSliderChanged(float Value)
{
	PendingSettings.JudgementOffsetMs = Value;
	if (TextSyncOffsetValue)
	{
		const FString Sign = Value >= 0.f ? TEXT("+") : TEXT("");
		TextSyncOffsetValue->SetText(FText::FromString(
			FString::Printf(TEXT("%s%.0fms"), *Sign, Value)));
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
	PendingSettings.GraphicsQuality = FMath::Clamp(ComboBoxGraphics->GetSelectedIndex(), 0, 3);
}
