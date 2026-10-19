// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PTBSceneWidgetBase.h"
#include "PTBStoryViewerWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UWidget;

/**
 * 스토리 대사와 캐릭터 비주얼을 순차적으로 표시하는 스토리 뷰어 위젯
 *
 * WBP 구성 (이름이 같으면 자동 연결, 전부 선택사항):
 *  - TextSpeaker     : 화자 이름 (TextBlock)
 *  - SpeakerBox      : 화자 이름표 배경. 화자가 비면 숨김 (아무 위젯)
 *  - TextDialogue    : 대사 (TextBlock)
 *  - ImageBackground     : 배경 그림 (Image, 전체 화면 앵커). 대사에 Background가 있을 때만 교체
 *  - ImageCharacter      : 가운데 스탠딩 이미지 (Image)
 *  - ImageCharacterLeft  : 왼쪽 스탠딩 이미지 (Image)
 *  - ImageCharacterRight : 오른쪽 스탠딩 이미지 (Image)
 *    → 각 Image는 ScaleBox(Stretch: Scale to Fit)로 감싸서 크기/위치를 ScaleBox로 지정
 *  - ButtonNext      : 다음 대사
 *  - ButtonSkip      : 스토리 전체 스킵
 *
 * 스탠딩 이미지: 대사의 PortraitSlot 위치에 표시, 화자 위치는 밝게 나머지는 어둡게
 * 다음 대사: ButtonNext / Enter / Space / 화면 클릭 / 리듬 키
 * 배경은 WBP에서 맨 뒤에 전체 화면 Image로 직접 배치
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBStoryViewerWidget : public UPTBSceneWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnSceneSetup() override;
	virtual void NativeOnSceneAction(EPTBActionType Action) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

	/** 다음 대사. 마지막 대사였으면 스토리 종료 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Story")
	void AdvanceLine();

	/** 현재 대사 인덱스 */
	UFUNCTION(BlueprintPure, Category = "PTB|Story")
	int32 GetCurrentLineIndex() const { return CurrentLineIndex; }

	/** 전체 대사 수 */
	UFUNCTION(BlueprintPure, Category = "PTB|Story")
	int32 GetLineCount() const { return Lines.Num(); }

protected:
	/** 배경이 바뀐 직후 (BP 연출용: 페이드 인 애니메이션 재생 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Story")
	void OnBackgroundChanged(UTexture2D* NewBackground);

	/** 대사 표시 직후 (BP에서 연출 추가용: 페이드, 효과음 등). Text는 토큰 치환 완료본 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Story")
	void OnLineShown(int32 LineIndex, const FText& Speaker, const FText& Text);

	void ShowLine(int32 LineIndex);

	/** 위치별 Image 위젯 (없으면 nullptr) */
	UImage* GetPortraitImage(EPTBPortraitSlot InSlot) const;

	/** 화자 위치 강조, 나머지 어둡게. bNarration이면 전부 원래 밝기 */
	void RefreshPortraitHighlight(EPTBPortraitSlot ActiveSlot, bool bNarration);

	/** 화자가 아닌 스탠딩 이미지 색 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Story")
	FLinearColor InactivePortraitTint = FLinearColor(0.45f, 0.45f, 0.45f, 1.0f);

	UFUNCTION()
	void HandleNextClicked();

	UFUNCTION()
	void HandleSkipClicked();

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextSpeaker = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UWidget> SpeakerBox = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextDialogue = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageBackground = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageCharacter = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageCharacterLeft = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageCharacterRight = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSkip = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;

	UPROPERTY()
	TArray<FPTBDialogueLine> Lines;

	int32 CurrentLineIndex = INDEX_NONE;
};
