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
	// Portrait가 지정된 위치만 보이도록 처음엔 전부 숨김
	for (UImage* PortraitImage : { ImageCharacter.Get(), ImageCharacterLeft.Get(), ImageCharacterRight.Get() })
	{
		if (PortraitImage)
		{
			PortraitImage->SetVisibility(ESlateVisibility::Hidden);
		}
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

	// 퇴장/장면 전환: 다른 위치 스탠딩 이미지 제거
	if (Line.bClearOtherPortraits)
	{
		for (const EPTBPortraitSlot EachSlot : { EPTBPortraitSlot::Center, EPTBPortraitSlot::Left, EPTBPortraitSlot::Right })
		{
			UImage* OtherImage = GetPortraitImage(EachSlot);
			if (EachSlot != Line.PortraitSlot && OtherImage)
			{
				OtherImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// Portrait가 비어 있으면 해당 위치의 이전 이미지 유지
	UImage* TargetImage = GetPortraitImage(Line.PortraitSlot);
	if (TargetImage && !Line.Portrait.IsNull())
	{
		if (UTexture2D* PortraitTexture = Line.Portrait.LoadSynchronous())
		{
			// 원본 크기로 설정해야 ScaleBox가 비율을 유지함
			TargetImage->SetBrushFromTexture(PortraitTexture, true);
			TargetImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}

	RefreshPortraitHighlight(Line.PortraitSlot, !bHasSpeaker);

	OnLineShown(LineIndex, SpeakerText, DialogueText);
}

UImage* UPTBStoryViewerWidget::GetPortraitImage(EPTBPortraitSlot InSlot) const
{
	switch (InSlot)
	{
	case EPTBPortraitSlot::Left:  return ImageCharacterLeft;
	case EPTBPortraitSlot::Right: return ImageCharacterRight;
	default:                      return ImageCharacter;
	}
}

void UPTBStoryViewerWidget::RefreshPortraitHighlight(EPTBPortraitSlot ActiveSlot, bool bNarration)
{
	for (const EPTBPortraitSlot EachSlot : { EPTBPortraitSlot::Center, EPTBPortraitSlot::Left, EPTBPortraitSlot::Right })
	{
		if (UImage* PortraitImage = GetPortraitImage(EachSlot))
		{
			const bool bActive = bNarration || EachSlot == ActiveSlot;
			PortraitImage->SetColorAndOpacity(bActive ? FLinearColor::White : InactivePortraitTint);
		}
	}
}

void UPTBStoryViewerWidget::HandleNextClicked()
{
	AdvanceLine();
}

void UPTBStoryViewerWidget::HandleSkipClicked()
{
	SkipScene();
}
