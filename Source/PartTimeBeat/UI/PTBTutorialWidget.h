// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/PTBSceneWidgetBase.h"
#include "PTBTutorialWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

/**
 * 조작/규칙 안내를 단계적으로 보여주는 튜토리얼 위젯
 * 안내 문구와 데모 이미지를 동기화해 학습 흐름을 제공
 *
 * WBP 구성 (이름이 같으면 자동 연결, 전부 선택사항):
 *  - TextInstruction : 안내 문구 (TextBlock)
 *  - TextProgress    : 입력 진행도 "1 / 3" 또는 스텝 진행도 (TextBlock)
 *  - ImageDemo       : 시범 이미지. 스텝에 없으면 숨김 (Image)
 *  - ButtonNext      : 다음 스텝. 입력이 필요한 스텝에서는 숨김
 *  - ButtonSkip      : 튜토리얼 전체 스킵
 *
 * 스텝 진행:
 *  - RequiredAction = None : ButtonNext / Enter / Space / 아무 리듬 키
 *  - RequiredAction 지정    : 해당 리듬 키를 RequiredCount번 입력
 * 배경은 반투명 전체 화면 Image를 WBP 맨 뒤에 직접 배치 (뒤의 게임 화면이 보이도록)
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBTutorialWidget : public UPTBSceneWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeOnSceneSetup() override;
	virtual void NativeOnSceneAction(EPTBActionType Action) override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

public:
	UFUNCTION(BlueprintCallable, Category = "PTB|UI")
	virtual void InitializeView();

	/** 다음 스텝. 마지막 스텝이었으면 튜토리얼 종료 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Tutorial")
	void AdvanceStep();

	/** 현재 스텝 인덱스 */
	UFUNCTION(BlueprintPure, Category = "PTB|Tutorial")
	int32 GetCurrentStepIndex() const { return CurrentStepIndex; }

	/** 전체 스텝 수 */
	UFUNCTION(BlueprintPure, Category = "PTB|Tutorial")
	int32 GetStepCount() const { return Steps.Num(); }

protected:
	/** 스텝 표시 직후 (BP 연출용). Instruction은 토큰 치환 완료본 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Tutorial")
	void OnStepShown(int32 StepIndex, const FText& Instruction, EPTBActionType RequiredAction, int32 RequiredCount);

	/** 요구 입력이 맞게 들어왔을 때 (BP 연출용: 체크 표시, 효과음 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Tutorial")
	void OnRequiredActionAccepted(int32 StepIndex, int32 CurrentCount, int32 RequiredCount);

	/** 요구와 다른 키가 들어왔을 때 (BP 연출용: 흔들기 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Tutorial")
	void OnWrongAction(int32 StepIndex, EPTBActionType PressedAction);

	void ShowStep(int32 StepIndex);
	void RefreshProgressText();
	bool IsCurrentStepWaitingForAction() const;

	UFUNCTION()
	void HandleNextClicked();

	UFUNCTION()
	void HandleSkipClicked();

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextInstruction = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> TextProgress = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> ImageDemo = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonNext = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> ButtonSkip = nullptr;

	UPROPERTY()
	TArray<FPTBTutorialStep> Steps;

	int32 CurrentStepIndex = INDEX_NONE;

	/** 현재 스텝에서 맞게 입력한 횟수 */
	int32 CurrentActionCount = 0;
};
