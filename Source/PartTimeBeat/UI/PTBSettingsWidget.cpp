// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBSettingsWidget.h"

#include "Components/Button.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
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
	if (SliderSyncOffset)
	{
		SliderSyncOffset->SetMinValue(-200.f);
		SliderSyncOffset->SetMaxValue(200.f);
	}

	RefreshAllWidgets();
}

// ── 설정 적용 / 초기화 ────────────────────────────────────────────

void UPTBSettingsWidget::ApplySettings()
{
	if (UPTBGameInstance* GI = GetGameInstance<UPTBGameInstance>())
	{
		GI->ApplyUserSettings(PendingSettings);
	}
	OnSettingsApplied();
}

void UPTBSettingsWidget::ResetToDefaults()
{
	PendingSettings = FPTBUserSettings{};
	RefreshAllWidgets();
	OnSettingsApplied();
}

// ── 키 재바인딩 ───────────────────────────────────────────────────

void UPTBSettingsWidget::RequestRebind(EPTBActionType Action)
{
	if (Action == EPTBActionType::None) return;
	PendingRebindAction = Action;
	SetKeyboardFocus();
	OnRebindStarted(Action);
}

void UPTBSettingsWidget::CancelRebind()
{
	PendingRebindAction = EPTBActionType::None;
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
		CancelRebind();
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
	case EPTBActionType::ActionA: SetText(TextKeyA); break;
	case EPTBActionType::ActionB: SetText(TextKeyB); break;
	case EPTBActionType::ActionC: SetText(TextKeyC); break;
	case EPTBActionType::ActionD: SetText(TextKeyD); break;
	case EPTBActionType::ActionE: SetText(TextKeyE); break;
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
	if (ButtonApply)   ButtonApply->OnClicked.AddDynamic(this,   &UPTBSettingsWidget::OnApplyClicked);
	if (ButtonReset)   ButtonReset->OnClicked.AddDynamic(this,   &UPTBSettingsWidget::OnResetClicked);
	if (ButtonRebindA) ButtonRebindA->OnClicked.AddDynamic(this, &UPTBSettingsWidget::OnRebindAClicked);
	if (ButtonRebindB) ButtonRebindB->OnClicked.AddDynamic(this, &UPTBSettingsWidget::OnRebindBClicked);
	if (ButtonRebindC) ButtonRebindC->OnClicked.AddDynamic(this, &UPTBSettingsWidget::OnRebindCClicked);
	if (ButtonRebindD) ButtonRebindD->OnClicked.AddDynamic(this, &UPTBSettingsWidget::OnRebindDClicked);
	if (ButtonRebindE) ButtonRebindE->OnClicked.AddDynamic(this, &UPTBSettingsWidget::OnRebindEClicked);

	if (SliderMaster)     SliderMaster->OnValueChanged.AddDynamic(this,     &UPTBSettingsWidget::OnMasterSliderChanged);
	if (SliderBgm)        SliderBgm->OnValueChanged.AddDynamic(this,        &UPTBSettingsWidget::OnBgmSliderChanged);
	if (SliderSfx)        SliderSfx->OnValueChanged.AddDynamic(this,        &UPTBSettingsWidget::OnSfxSliderChanged);
	if (SliderSyncOffset) SliderSyncOffset->OnValueChanged.AddDynamic(this, &UPTBSettingsWidget::OnSyncOffsetSliderChanged);

	if (ComboBoxWindowMode) ComboBoxWindowMode->OnSelectionChanged.AddDynamic(this, &UPTBSettingsWidget::OnWindowModeSelectionChanged);
	if (ComboBoxResolution) ComboBoxResolution->OnSelectionChanged.AddDynamic(this, &UPTBSettingsWidget::OnResolutionSelectionChanged);
	if (ComboBoxGraphics)   ComboBoxGraphics->OnSelectionChanged.AddDynamic(this,   &UPTBSettingsWidget::OnGraphicsSelectionChanged);
}

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
