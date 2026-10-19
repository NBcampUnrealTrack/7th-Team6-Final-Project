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
		&& (Key == EKeys::Enter || Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Bottom)
		&& HandleConfirmInput())
	{
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

bool UPTBStoryViewerWidget::NativeOnConfirm()
{
	AdvanceLine();
	return true;
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

	// 배경: 지정된 줄에서만 교체, 비우면 이전 배경(또는 WBP 기본 배경) 유지
	if (ImageBackground && !Line.Background.IsNull())
	{
		if (UTexture2D* BackgroundTexture = Line.Background.LoadSynchronous())
		{
			// 배경은 전체 화면 앵커로 늘려 쓰므로 원본 크기 맞춤 안 함
			ImageBackground->SetBrushFromTexture(BackgroundTexture, false);
			ImageBackground->SetColorAndOpacity(FLinearColor::White);
			OnBackgroundChanged(BackgroundTexture);
		}
	}

	const bool bHasListener = !Line.ListenerPortrait.IsNull() && Line.ListenerSlot != Line.PortraitSlot;

	// 퇴장/장면 전환/혼잣말: 화자·청자 위치를 뺀 나머지 스탠딩 이미지 제거
	if (Line.bClearOtherPortraits)
	{
		for (const EPTBPortraitSlot EachSlot : { EPTBPortraitSlot::Center, EPTBPortraitSlot::Left, EPTBPortraitSlot::Right })
		{
			const bool bKeep = EachSlot == Line.PortraitSlot || (bHasListener && EachSlot == Line.ListenerSlot);
			UImage* OtherImage = GetPortraitImage(EachSlot);
			if (!bKeep && OtherImage)
			{
				OtherImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
	}

	// 듣는 사람 배치 (어둡게 표시는 아래 RefreshPortraitHighlight에서)
	if (bHasListener)
	{
		UImage* ListenerImage = GetPortraitImage(Line.ListenerSlot);
		UTexture2D* ListenerTexture = Line.ListenerPortrait.LoadSynchronous();
		if (ListenerImage && ListenerTexture)
		{
			ListenerImage->SetBrushFromTexture(ListenerTexture, true);
			ListenerImage->SetVisibility(ESlateVisibility::HitTestInvisible);
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
