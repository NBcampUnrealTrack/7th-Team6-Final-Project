#include "MiniGames/SR/SRWidget.h"

void USRWidget::UpdatePreviewImage(UTexture2D* NewTexture)
{
    if (Img_NextTopping && NewTexture)
    {
        // 블루프린트의 "Set Brush from Texture" 노드를 C++ 코드로 구현한 한 줄입니다.
        Img_NextTopping->SetBrushFromTexture(NewTexture);
    }
}
