// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTBModalMenuWidget.h"
#include "Components/ComboBoxString.h"
#include "Core/PTBStructEnums.h"
#include "PTBSettingsWidget.generated.h"

class UButton;
class UImage;
class USlider;
class UTextBlock;
class UWidgetSwitcher;

/**
 * 옵션 화면 위젯.
 * GameInstance의 CachedSettings를 읽어 초기화하고,
 * Apply 버튼 클릭 시 변경된 값을 ApplyUserSettings로 일괄 반영한다.
 *
 * WBP Designer 탭에서 구성해야 하는 위젯 이름 (BindWidgetOptional):
 *  [탭]    TabSwitcher (WidgetSwitcher, Index: 0=그래픽, 1=사운드, 2=키, 3=게임)
 *          ButtonTabGraphics, ButtonTabSounds, ButtonTabKeys, ButtonTabGame
 *  [닫기]  ButtonClose
 *  [볼륨]  SliderMaster, SliderBgm, SliderSfx
 *          TextMasterValue, TextBgmValue, TextSfxValue
 *  [싱크]  SliderSyncOffset (-200~+200ms), TextSyncOffsetValue
 *  [화면]  ComboBoxWindowMode, ComboBoxResolution, ComboBoxGraphics
 *  [키]    KeyText_A~KeyText_E, ButtonRebindA~ButtonRebindE, TextRebindPrompt
 *  [제어]  ButtonApply, ButtonReset, ButtonCalibrate
 *  [알림]  TextNotification (2초 후 자동 숨김, Hidden 상태로 시작)
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBSettingsWidget : public UPTBModalMenuWidget
{
	GENERATED_BODY()

public:
	UPTBSettingsWidget(const FObjectInitializer& ObjectInitializer);

	/** NativeConstruct에서 자동 호출. 현재 GameInstance 설정값을 위젯에 반영. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	virtual void InitializeView();

	/** 현재 위젯 상태를 GameInstance에 적용하고 저장. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void ApplySettings();

	/** 설정을 기본값으로 되돌림 (Apply 전까지는 PendingSettings만 초기화). */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void ResetToDefaults();

	/** 키 재바인딩 시작. ButtonRebindX 클릭 또는 BP에서 직접 호출. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void RequestRebind(EPTBActionType Action);

	/** 진행 중인 키 재바인딩 취소. Escape 입력 시 자동 호출. */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void CancelRebind();

	/** 탭 직접 전환. BP에서 호출 가능. Index: 0=그래픽, 1=사운드, 2=키, 3=게임 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Settings")
	void SwitchToTab(int32 TabIndex);

	// ── BP 이벤트 ────────────────────────────────────────────────────

	/** 키 재바인딩 대기 시작. BP에서 "키를 눌러주세요" 안내 UI 표시. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnRebindStarted(EPTBActionType Action);

	/** 키 재바인딩 완료. BP에서 키 이름 텍스트 갱신. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnRebindCompleted(EPTBActionType Action, const FKey& NewKey);

	/**
	 * 키 충돌로 자동 스왑 발생. BP에서 경고 애니메이션 표시.
	 * ConflictingAction = 새 키를 빼앗긴 액션 (해당 액션에는 이전 키가 자동 할당됨)
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnKeyConflictDetected(EPTBActionType ConflictingAction);

	/** 재바인딩 취소 (Escape). BP에서 "키를 눌러주세요" 안내 UI 숨김. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnRebindCancelled(EPTBActionType Action);

	/**
	 * 탭 전환 완료. BP에서 활성 탭 버튼 스타일 변경에 사용.
	 * TabIndex: 0=그래픽, 1=사운드, 2=키, 3=게임
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnTabChanged(int32 TabIndex);

	/**
	 * ApplySettings 완료.
	 * C++ 기본 구현: TextNotification에 "설정이 저장되었습니다." 표시 후 2초 후 숨김.
	 * BP 오버라이드 시 추가 애니메이션 등 처리 가능. Super 호출 권장.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Settings")
	void OnSettingsApplied();
	virtual void OnSettingsApplied_Implementation();

	/**
	 * ResetToDefaults 완료. GameInstance에는 아직 반영되지 않음.
	 * C++ 기본 구현: TextNotification에 "기본값으로 초기화되었습니다." 표시.
	 * BP 오버라이드 시 추가 처리 가능. Super 호출 권장.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "PTB|Settings")
	void OnSettingsReset();
	virtual void OnSettingsReset_Implementation();

protected:
	virtual void NativeConstruct() override;
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }
	// PreviewKeyDown: 재바인딩 중 ESC가 부모의 CloseMenu로 전달되지 않도록 차단
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ── 탭 전환 ──────────────────────────────────────────────────────

	/** Index: 0=그래픽, 1=사운드, 2=키 바인딩, 3=게임 설정 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UWidgetSwitcher> TabSwitcher;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UButton> ButtonTabGraphics;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UButton> ButtonTabSounds;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UButton> ButtonTabKeys;

	/** 게임 설정 탭 (판정 오프셋 등) */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UButton> ButtonTabGame;

	/** 선택된 탭에서만 표시되는 하단 인디케이터 라인 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UImage> ImgTabLine_Graphics;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UImage> ImgTabLine_Sounds;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UImage> ImgTabLine_Keys;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Tab")
	TObjectPtr<UImage> ImgTabLine_Game;

	// ── 닫기 버튼 ────────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings")
	TObjectPtr<UButton> ButtonClose;

	// ── 알림 텍스트 ──────────────────────────────────────────────────

	/** 설정 적용/초기화 결과 알림. Hidden 상태로 시작, 2초 후 자동 숨김. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings")
	TObjectPtr<UTextBlock> TextNotification;

	// ── 볼륨 ─────────────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<USlider> SliderMaster;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<USlider> SliderBgm;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<USlider> SliderSfx;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<UTextBlock> TextMasterValue;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<UTextBlock> TextBgmValue;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Audio")
	TObjectPtr<UTextBlock> TextSfxValue;

	// ── 싱크 조절 ────────────────────────────────────────────────────

	/** 판정 오프셋 슬라이더 (-200ms ~ +200ms). InitializeView에서 범위 자동 설정. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Sync")
	TObjectPtr<USlider> SliderSyncOffset;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Sync")
	TObjectPtr<UTextBlock> TextSyncOffsetValue;

	// ── 화면/그래픽 ──────────────────────────────────────────────────

	/** 창 모드 선택. 옵션: "전체화면" / "창 전체화면" / "창 모드" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxWindowMode;

	/** 해상도 프리셋 선택. 옵션: "1280×720" ~ "3840×2160" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxResolution;

	/** 그래픽 품질 선택. 옵션: "낮음" / "중간" / "높음" / "최고" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxGraphics;

	// ── 키 바인딩 ────────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> KeyText_A;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> KeyText_B;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> KeyText_C;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> KeyText_D;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> KeyText_E;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UButton> ButtonRebindA;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UButton> ButtonRebindB;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UButton> ButtonRebindC;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UButton> ButtonRebindD;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UButton> ButtonRebindE;

	/** 재바인딩 대기 중 표시할 안내 텍스트. BP에서 가시성 제어 권장. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextRebindPrompt;

	// ── 제어 버튼 ────────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings")
	TObjectPtr<UButton> ButtonApply;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings")
	TObjectPtr<UButton> ButtonReset;

	/** 추후 싱크 조절 미니게임 진입용 (현재 미구현) */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings")
	TObjectPtr<UButton> ButtonCalibrate;

private:
	/** Apply 전까지만 유효한 임시 설정값 */
	FPTBUserSettings PendingSettings;

	/** 현재 재바인딩 대기 중인 액션. None이면 대기 없음. */
	EPTBActionType PendingRebindAction = EPTBActionType::None;

	/** TextNotification 자동 숨김 타이머 */
	FTimerHandle NotificationTimerHandle;

	void RefreshAllWidgets();
	void RefreshKeyText(EPTBActionType Action);
	void RefreshVolumeText(UTextBlock* TextWidget, float Value);
	void BindWidgetCallbacks();

	void ShowNotification(const FString& Message);
	void HideNotification();

	UFUNCTION() void OnCloseClicked();
	UFUNCTION() void OnTabSoundsClicked();
	UFUNCTION() void OnTabGraphicsClicked();
	UFUNCTION() void OnTabKeysClicked();
	UFUNCTION() void OnTabGameClicked();
	UFUNCTION() void OnApplyClicked();
	UFUNCTION() void OnResetClicked();
	UFUNCTION() void OnRebindAClicked();
	UFUNCTION() void OnRebindBClicked();
	UFUNCTION() void OnRebindCClicked();
	UFUNCTION() void OnRebindDClicked();
	UFUNCTION() void OnRebindEClicked();
	UFUNCTION() void OnMasterSliderChanged(float Value);
	UFUNCTION() void OnBgmSliderChanged(float Value);
	UFUNCTION() void OnSfxSliderChanged(float Value);
	UFUNCTION() void OnSyncOffsetSliderChanged(float Value);
	UFUNCTION() void OnWindowModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION() void OnResolutionSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION() void OnGraphicsSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
