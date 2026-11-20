#include "UI/PTBFirstPlayGuideWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "UI/PTBKeyTextLibrary.h"

void UPTBFirstPlayGuideWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	BuildDefaultLayoutIfNeeded();
}

void UPTBFirstPlayGuideWidget::BuildDefaultLayoutIfNeeded()
{
	if (!WidgetTree || WidgetTree->RootWidget)
	{
		return; // WBP에서 만든 레이아웃이 있으면 그대로 사용
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	TXT_Hint = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TXT_Hint"));
	FSlateFontInfo Font = TXT_Hint->GetFont();
	Font.Size = DefaultFontSize;
	Font.OutlineSettings.OutlineSize = 2;
	Font.OutlineSettings.OutlineColor = FLinearColor(0.f, 0.f, 0.f, 0.85f);
	TXT_Hint->SetFont(Font);
	TXT_Hint->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.92f, 0.45f, 1.f)));
	TXT_Hint->SetJustification(ETextJustify::Center);

	if (UCanvasPanelSlot* TextSlot = Root->AddChildToCanvas(TXT_Hint))
	{
		TextSlot->SetAnchors(FAnchors(0.5f, 0.f, 0.5f, 0.f));
		TextSlot->SetAlignment(FVector2D(0.5f, 0.f));
		TextSlot->SetPosition(FVector2D(0.f, DefaultTopMargin));
		TextSlot->SetAutoSize(true);
	}
}

void UPTBFirstPlayGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PulseTime = 0.f;
	RefreshHintText();
}

void UPTBFirstPlayGuideWidget::RefreshHintText()
{
	if (TXT_Hint)
	{
		TXT_Hint->SetText(UPTBKeyTextLibrary::FormatKeyTokens(this, HintFormat));
	}
}

void UPTBFirstPlayGuideWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!TXT_Hint || PulseMinOpacity >= 1.f)
	{
		return;
	}

	PulseTime += InDeltaTime;
	// 0~1을 부드럽게 오가는 값 (cos 기반: 처음엔 가장 밝게 시작)
	const float Wave = 0.5f + 0.5f * FMath::Cos(PulseTime * 2.f * PI / PulsePeriodSec);
	TXT_Hint->SetRenderOpacity(FMath::Lerp(PulseMinOpacity, 1.f, Wave));
}
