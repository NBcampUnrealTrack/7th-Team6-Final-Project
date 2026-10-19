// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBTutorialWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

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
	if (!InKeyEvent.IsRepeat()
		&& (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
		&& !IsCurrentStepWaitingForAction())
	{
		AdvanceStep();
		return FReply::Handled();
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
	if (!IsCurrentStepWaitingForAction())
	{
		AdvanceStep();
	}
}

void UPTBTutorialWidget::HandleSkipClicked()
{
	SkipScene();
}
