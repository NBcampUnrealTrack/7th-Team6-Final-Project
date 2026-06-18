// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PTBModalMenuWidget.h"
#include "Components/ComboBoxString.h"
#include "Core/PTBStructEnums.h"
#include "PTBSettingsWidget.generated.h"

class UButton;
class USlider;
class UTextBlock;

/**
 * 옵션 화면 위젯.
 * GameInstance의 CachedSettings를 읽어 초기화하고,
 * Apply 버튼 클릭 시 변경된 값을 ApplyUserSettings로 일괄 반영한다.
 *
 * WBP Designer 탭에서 구성해야 하는 위젯 이름 (BindWidgetOptional):
 *  [볼륨]  SliderMaster, SliderBgm, SliderSfx
 *          TextMasterValue, TextBgmValue, TextSfxValue
 *  [싱크]  SliderSyncOffset (-200~+200ms), TextSyncOffsetValue
 *  [화면]  ComboBoxWindowMode, ComboBoxResolution, ComboBoxGraphics
 *  [키]    TextKeyA~TextKeyE, ButtonRebindA~ButtonRebindE, TextRebindPrompt
 *  [제어]  ButtonApply, ButtonReset, ButtonCalibrate
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

	// ── BP 구현 이벤트 ───────────────────────────────────────────

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

	/** ApplySettings/ResetToDefaults 완료. BP에서 "저장됨" 피드백 표시. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Settings")
	void OnSettingsApplied();

protected:
	virtual void NativeConstruct() override;
	virtual bool NativeSupportsKeyboardFocus() const override { return true; }
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	// ── 볼륨 ─────────────────────────────────────────────────────

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

	// ── 싱크 조절 ────────────────────────────────────────────────

	/** 판정 오프셋 슬라이더 (-200ms ~ +200ms). InitializeView에서 범위 자동 설정. */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Sync")
	TObjectPtr<USlider> SliderSyncOffset;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Sync")
	TObjectPtr<UTextBlock> TextSyncOffsetValue;

	// ── 화면/그래픽 ──────────────────────────────────────────────

	/** 창 모드 선택. 옵션: "전체화면" / "창 전체화면" / "창 모드" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxWindowMode;

	/** 해상도 프리셋 선택. 옵션: "1280×720" ~ "3840×2160" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxResolution;

	/** 그래픽 품질 선택. 옵션: "낮음" / "중간" / "높음" / "최고" */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Display")
	TObjectPtr<UComboBoxString> ComboBoxGraphics;

	// ── 키 바인딩 ─────────────────────────────────────────────────

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextKeyA;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextKeyB;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextKeyC;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextKeyD;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|Settings|Keys")
	TObjectPtr<UTextBlock> TextKeyE;

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

	// ── 제어 버튼 ────────────────────────────────────────────────

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

	void RefreshAllWidgets();
	void RefreshKeyText(EPTBActionType Action);
	void RefreshVolumeText(UTextBlock* TextWidget, float Value);
	void BindWidgetCallbacks();

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
