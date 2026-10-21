// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "FSWidget.generated.h"

class UImage;
class UTextBlock;
class UWidget;
class APTBFSMiniGame;

struct FFSArrowState
{
	float TargetTimeMs = 0.0f;
	float CueLeadTimeMs = 1.0f;
	bool bIsActive = false;
};

UCLASS()
class PARTTIMEBEAT_API UFSWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// ── 키 안내 UI (전부 선택사항, 이름이 같으면 자동 연결) ──
	// KeyGuideA/B/C   : A/B/C 키 안내 묶음 (Canvas Panel 바로 아래 자식. 이 위젯이 칸 위치로 이동)
	// KeyGuideTextA/B/C : 키 이름 텍스트 (설정한 키 이름이 자동으로 들어감)
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideA = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideB = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UWidget* KeyGuideC = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextA = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextB = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly)
	UTextBlock* KeyGuideTextC = nullptr;

	/** 칸별 키 안내 위치 (0=왼쪽, 1=가운데, 2=오른쪽). Canvas 슬롯 Position 값 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|KeyGuide")
	TArray<FVector2D> KeyGuideLanePositions = { FVector2D(760.0, 660.0), FVector2D(960.0, 660.0), FVector2D(1160.0, 660.0) };

	/** 교체 중 곡선 이동 높이 (오른쪽으로 가는 키는 위로, 왼쪽으로 가는 키는 아래로 돌아서 겹치지 않음) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "FS|KeyGuide")
	float KeyGuideSwapArcHeight = 40.0f;

	/** 키 배치 교체 연출이 시작될 때 (BP: 효과음, 깜빡임 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "FS|KeyGuide")
	void OnKeyGuideSwapStarted();

	/** 설정된 키 이름으로 텍스트 갱신 */
	UFUNCTION(BlueprintCallable, Category = "FS|KeyGuide")
	void RefreshKeyGuideLabels();

	void UpdateKeyGuide(float VisualTimeMs);

	bool bKeyGuideWasSwapping = false;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowLeft;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowRight;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	UImage* ArrowUp;

	UPROPERTY()
	APTBFSMiniGame* FSMiniGame;
	
	FFSArrowState ArrowStateA;
	FFSArrowState ArrowStateB;
	FFSArrowState ArrowStateC;

public:
	void InitializeWidget(APTBFSMiniGame* InGame);

	UFUNCTION()
	void OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs);
};
