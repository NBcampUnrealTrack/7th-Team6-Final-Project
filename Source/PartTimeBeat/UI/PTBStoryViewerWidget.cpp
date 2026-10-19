// Fill out your copyright notice in the Description page of Project Settings.

#include "PTBStoryViewerWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

void UPTBStoryViewerWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonNext)
	{
		ButtonNext->OnClicked.AddUniqueDynamic(this, &UPTBStoryViewerWidget::HandleNextClicked);
	}
	if (ButtonSkip)
	{
		ButtonSkip->OnClicked.AddUniqueDynamic(this, &UPTBStoryViewerWidget::HandleSkipClicked);
	}

	InitializeView();
}

void UPTBStoryViewerWidget::InitializeView()
{
	if (ImageCharacter)
	{
		// 첫 대사에 Portrait가 있을 때만 보이도록
		ImageCharacter->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPTBStoryViewerWidget::NativeOnSceneSetup()
{
	Lines.Reset();
	CurrentLineIndex = INDEX_NONE;

	if (!GetDialogueLines(Lines) || Lines.IsEmpty())
	{
		FinishScene(false);
		return;
	}

	ShowLine(0);
}

void UPTBStoryViewerWidget::NativeOnSceneAction(EPTBActionType Action)
{
	// 리듬 키로도 대사 넘기기
	AdvanceLine();
}

FReply UPTBStoryViewerWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (!InKeyEvent.IsRepeat()
		&& (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom))
	{
		AdvanceLine();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UPTBStoryViewerWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 버튼 이외 영역 클릭 시 다음 대사 (버튼은 자기 클릭을 먼저 소비함)
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		AdvanceLine();
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UPTBStoryViewerWidget::AdvanceLine()
{
	if (bSceneFinished)
	{
		return;
	}

	const int32 NextIndex = CurrentLineIndex + 1;
	if (!Lines.IsValidIndex(NextIndex))
	{
		FinishScene(false);
		return;
	}

	ShowLine(NextIndex);
}

void UPTBStoryViewerWidget::ShowLine(int32 LineIndex)
{
	if (!Lines.IsValidIndex(LineIndex))
	{
		return;
	}

	CurrentLineIndex = LineIndex;
	const FPTBDialogueLine& Line = Lines[LineIndex];

	const FText SpeakerText = FormatSceneText(Line.Speaker);
	const FText DialogueText = FormatSceneText(Line.Text);
	const bool bHasSpeaker = !SpeakerText.IsEmpty();

	if (TextSpeaker)
	{
		TextSpeaker->SetText(SpeakerText);
		TextSpeaker->SetVisibility(bHasSpeaker ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (SpeakerBox)
	{
		SpeakerBox->SetVisibility(bHasSpeaker ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (TextDialogue)
	{
		TextDialogue->SetText(DialogueText);
	}

	// Portrait가 비어 있으면 이전 이미지 유지
	if (ImageCharacter && !Line.Portrait.IsNull())
	{
		if (UTexture2D* PortraitTexture = Line.Portrait.LoadSynchronous())
		{
			ImageCharacter->SetBrushFromTexture(PortraitTexture, true);
			ImageCharacter->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	OnLineShown(LineIndex, SpeakerText, DialogueText);
}

void UPTBStoryViewerWidget::HandleNextClicked()
{
	AdvanceLine();
}

void UPTBStoryViewerWidget::HandleSkipClicked()
{
	SkipScene();
}
