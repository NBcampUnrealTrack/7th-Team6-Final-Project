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
 *  - ImageCharacter  : 스탠딩 이미지 (Image)
 *  - ButtonNext      : 다음 대사
 *  - ButtonSkip      : 스토리 전체 스킵
 *
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
	/** 대사 표시 직후 (BP에서 연출 추가용: 페이드, 효과음 등). Text는 토큰 치환 완료본 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Story")
	void OnLineShown(int32 LineIndex, const FText& Speaker, const FText& Text);

	void ShowLine(int32 LineIndex);

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
	TObjectPtr<UImage> ImageCharacter = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSkip = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;

	UPROPERTY()
	TArray<FPTBDialogueLine> Lines;

	int32 CurrentLineIndex = INDEX_NONE;
};
