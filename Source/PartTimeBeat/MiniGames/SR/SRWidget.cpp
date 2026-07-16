#include "MiniGames/SR/SRWidget.h"

void USRWidget::UpdatePreviewImage(UTexture2D* NewTexture)
{
    if (Img_NextTopping && NewTexture)
    {
        // 블루프린트의 "Set Brush from Texture" 노드를 C++ 코드로 구현한 한 줄
        Img_NextTopping->SetBrushFromTexture(NewTexture);
    }
}

void USRWidget::UpdateSuccessCount(int32 NewCount)
{
    if (Txt_SuccessCount)
    {
        Txt_SuccessCount->SetText(FText::AsNumber(NewCount));
    }
}

void USRWidget::UpdateJudgementText(EPTBJudgementType JudgementType, float DeltaMs, bool bHasTimingInfo)
{
    if (!Txt_Judgement)
    {
        return;
    }

    FString Label;
    switch (JudgementType)
    {
    case EPTBJudgementType::HighPerfect:
        Label = TEXT("High Perfect");
        break;
    case EPTBJudgementType::Perfect:
        Label = TEXT("Perfect");
        break;
    case EPTBJudgementType::Good:
        Label = TEXT("Good");
        break;
    case EPTBJudgementType::Miss:
    default:
        Label = TEXT("Miss");
        break;
    }

    // ★ Good/Miss일 때만 몇 ms 빠르거나(음수) 늦었는지(양수) 같이 표시합니다.
    //   bHasTimingInfo는 EmptyInput(헛입력, 매칭된 노트 자체가 없어 ms가 의미 없음)일 때 false로 넘어옵니다.
    const bool bShouldShowTiming = bHasTimingInfo
        && (JudgementType == EPTBJudgementType::Good || JudgementType == EPTBJudgementType::Miss);

    FString DisplayText = Label;
    if (bShouldShowTiming)
    {
        DisplayText += FString::Printf(TEXT(" (%s%.0fms)"), DeltaMs >= 0.0f ? TEXT("+") : TEXT(""), DeltaMs);
    }

    Txt_Judgement->SetText(FText::FromString(DisplayText));
}