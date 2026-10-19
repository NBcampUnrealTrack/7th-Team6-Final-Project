// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBTutorialWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "MiniGames/Common/PTBBaseMiniGame.h"

void UPTBTutorialWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonNext)
	{
		ButtonNext->OnClicked.AddUniqueDynamic(this, &UPTBTutorialWidget::HandleNextClicked);
	}
	if (ButtonSkip)
	{
		ButtonSkip->OnClicked.AddUniqueDynamic(this, &UPTBTutorialWidget::HandleSkipClicked);
	}

	InitializeView();
}

void UPTBTutorialWidget::InitializeView()
{
	if (ImageDemo)
	{
		ImageDemo->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UPTBTutorialWidget::NativeOnSceneSetup()
{
	Steps.Reset();
	CurrentStepIndex = INDEX_NONE;
	CurrentActionCount = 0;

	if (!GetTutorialSteps(Steps) || Steps.IsEmpty())
	{
		FinishScene(false);
		return;
	}

	ShowStep(0);
}

void UPTBTutorialWidget::NativeOnSceneAction(EPTBActionType Action)
{
	if (!Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	// 연습 스텝: 설명 중에는 리듬 키로도 시작 가능 (플레이 중 입력은 미니게임 판정으로 감)
	// 결과 표시 중에는 연습 끝에 이어 누른 키로 바로 넘어가지 않도록 리듬 키 무시 (다음 버튼/Enter/Space만)
	if (PracticeState != EPTBTutorialPracticeState::None)
	{
		if (PracticeState == EPTBTutorialPracticeState::Ready)
		{
			HandleProceed();
		}
		return;
	}

	// 입력 요구가 없는 스텝은 아무 리듬 키로 넘기기
	if (!IsCurrentStepWaitingForAction())
	{
		AdvanceStep();
		return;
	}

	const FPTBTutorialStep& Step = Steps[CurrentStepIndex];
	if (Action != Step.RequiredAction)
	{
		OnWrongAction(CurrentStepIndex, Action);
		return;
	}

	++CurrentActionCount;
	const int32 RequiredCount = FMath::Max(1, Step.RequiredCount);
	OnRequiredActionAccepted(CurrentStepIndex, CurrentActionCount, RequiredCount);
	RefreshProgressText();

	if (CurrentActionCount >= RequiredCount)
	{
		AdvanceStep();
	}
}

FReply UPTBTutorialWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	const bool bProceedKey = Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom;
	if (!InKeyEvent.IsRepeat() && bProceedKey)
	{
		if (PracticeState == EPTBTutorialPracticeState::Playing)
		{
			// 플레이 중에는 무시 (리듬 키로 쓰는 경우는 아래로 흘려보냄)
			return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
		}
		if (PracticeState != EPTBTutorialPracticeState::None)
		{
			HandleProceed();
			return FReply::Handled();
		}
		if (!IsCurrentStepWaitingForAction())
		{
			AdvanceStep();
			return FReply::Handled();
		}
	}

	// 리듬 키는 처리하지 않고 흘려보냄 → PlayerController → 미니게임 → NativeOnSceneAction
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UPTBTutorialWidget::AdvanceStep()
{
	if (bSceneFinished)
	{
		return;
	}

	const int32 NextIndex = CurrentStepIndex + 1;
	if (!Steps.IsValidIndex(NextIndex))
	{
		FinishScene(false);
		return;
	}

	ShowStep(NextIndex);
}

void UPTBTutorialWidget::ShowStep(int32 StepIndex)
{
	if (!Steps.IsValidIndex(StepIndex))
	{
		return;
	}

	CurrentStepIndex = StepIndex;
	CurrentActionCount = 0;
	const FPTBTutorialStep& Step = Steps[StepIndex];
	PracticeState = Step.IsPracticeStep() ? EPTBTutorialPracticeState::Ready : EPTBTutorialPracticeState::None;

	if (PanelInstruction)
	{
		PanelInstruction->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	const FText InstructionText = FormatSceneText(Step.Instruction);
	if (TextInstruction)
	{
		TextInstruction->SetText(InstructionText);
	}

	if (ImageDemo)
	{
		UTexture2D* DemoTexture = Step.DemoImage.IsNull() ? nullptr : Step.DemoImage.LoadSynchronous();
		if (DemoTexture)
		{
			ImageDemo->SetBrushFromTexture(DemoTexture, true);
			ImageDemo->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			ImageDemo->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (ButtonNext)
	{
		ButtonNext->SetVisibility(IsCurrentStepWaitingForAction()
			? ESlateVisibility::Collapsed
			: ESlateVisibility::Visible);
	}

	RefreshProgressText();
	OnStepShown(StepIndex, InstructionText, Step.RequiredAction, FMath::Max(1, Step.RequiredCount));
}

void UPTBTutorialWidget::RefreshProgressText()
{
	if (!TextProgress || !Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	if (IsCurrentStepWaitingForAction())
	{
		// 입력 진행도: "1 / 3"
		const int32 RequiredCount = FMath::Max(1, Steps[CurrentStepIndex].RequiredCount);
		TextProgress->SetText(FText::Format(
			NSLOCTEXT("PTBTutorial", "ActionProgress", "{0} / {1}"),
			FText::AsNumber(CurrentActionCount),
			FText::AsNumber(RequiredCount)));
	}
	else
	{
		// 스텝 진행도: "2 / 5"
		TextProgress->SetText(FText::Format(
			NSLOCTEXT("PTBTutorial", "StepProgress", "{0} / {1}"),
			FText::AsNumber(CurrentStepIndex + 1),
			FText::AsNumber(Steps.Num())));
	}
}

bool UPTBTutorialWidget::IsCurrentStepWaitingForAction() const
{
	return Steps.IsValidIndex(CurrentStepIndex)
		&& Steps[CurrentStepIndex].RequiredAction != EPTBActionType::None
		&& CurrentActionCount < FMath::Max(1, Steps[CurrentStepIndex].RequiredCount);
}

void UPTBTutorialWidget::HandleNextClicked()
{
	if (PracticeState != EPTBTutorialPracticeState::None)
	{
		HandleProceed();
		return;
	}

	if (!IsCurrentStepWaitingForAction())
	{
		AdvanceStep();
	}
}

void UPTBTutorialWidget::HandleProceed()
{
	switch (PracticeState)
	{
	case EPTBTutorialPracticeState::Ready:
	case EPTBTutorialPracticeState::Failed:
		StartPractice();
		break;
	case EPTBTutorialPracticeState::Passed:
		AdvanceStep();
		break;
	default:
		break;
	}
}

void UPTBTutorialWidget::StartPractice()
{
	APTBBaseMiniGame* MiniGame = GetOwnerMiniGame();
	if (!MiniGame || !Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	if (!MiniGame->StartTutorialPractice(Steps[CurrentStepIndex]))
	{
		// 연습을 못 돌리는 경우(빈 패턴 등) 막히지 않도록 그냥 다음 스텝
		PracticeState = EPTBTutorialPracticeState::Passed;
		AdvanceStep();
		return;
	}

	PracticeState = EPTBTutorialPracticeState::Playing;
	if (PanelInstruction)
	{
		PanelInstruction->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (ButtonNext)
	{
		ButtonNext->SetVisibility(ESlateVisibility::Collapsed);
	}

	OnPracticeStarted(CurrentStepIndex);
}

void UPTBTutorialWidget::NativeOnPracticeFinished(bool bPassed, int32 HitCount, int32 RequiredCount, int32 TotalCount)
{
	if (!Steps.IsValidIndex(CurrentStepIndex))
	{
		return;
	}

	const FPTBTutorialStep& Step = Steps[CurrentStepIndex];
	PracticeState = bPassed ? EPTBTutorialPracticeState::Passed : EPTBTutorialPracticeState::Failed;

	FText ResultText;
	if (bPassed)
	{
		ResultText = Step.PracticeSuccessText.IsEmpty()
			? NSLOCTEXT("PTBTutorial", "PracticePassed", "잘했어요!")
			: FormatSceneText(Step.PracticeSuccessText);
	}
	else
	{
		ResultText = Step.PracticeRetryText.IsEmpty()
			? NSLOCTEXT("PTBTutorial", "PracticeRetry", "아쉬워요! 박자에 맞춰 다시 해볼까요?")
			: FormatSceneText(Step.PracticeRetryText);
	}

	if (TextInstruction)
	{
		TextInstruction->SetText(ResultText);
	}
	if (TextProgress)
	{
		TextProgress->SetText(FText::Format(
			NSLOCTEXT("PTBTutorial", "PracticeResult", "{0} / {1}"),
			FText::AsNumber(HitCount),
			FText::AsNumber(RequiredCount)));
	}
	if (PanelInstruction)
	{
		PanelInstruction->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
	if (ButtonNext)
	{
		ButtonNext->SetVisibility(ESlateVisibility::Visible);
	}
}

void UPTBTutorialWidget::HandleSkipClicked()
{
	SkipScene();
}
