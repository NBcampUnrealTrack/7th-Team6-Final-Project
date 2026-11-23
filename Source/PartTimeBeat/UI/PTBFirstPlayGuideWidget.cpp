#include "UI/PTBFirstPlayGuideWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "UI/PTBKeyTextLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"

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

	// 화면 밖 방향 표시: 위쪽을 가리키는 삼각형 (회전시켜서 방향 표시)
	OffscreenIndicator = MakeTriangleText(Root, TEXT("OffscreenIndicator"), TEXT("\u25B2"), IndicatorFontSize); // ▲
	// 화면 안 대상 위 표시: 아래를 가리키는 삼각형
	OnscreenMarker = MakeTriangleText(Root, TEXT("OnscreenMarker"), TEXT("\u25BC"), MarkerFontSize); // ▼
}

UTextBlock* UPTBFirstPlayGuideWidget::MakeTriangleText(UCanvasPanel* Root, FName Name, const TCHAR* Glyph, int32 FontSize)
{
	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	FSlateFontInfo Font = Text->GetFont();
	Font.Size = FontSize;
	Font.OutlineSettings.OutlineSize = 4;
	Font.OutlineSettings.OutlineColor = FLinearColor(0.f, 0.f, 0.f, 0.9f);
	Text->SetFont(Font);
	Text->SetColorAndOpacity(FSlateColor(IndicatorColor));
	Text->SetText(FText::FromString(Glyph));
	Text->SetJustification(ETextJustify::Center);
	if (UCanvasPanelSlot* TextSlot = Root->AddChildToCanvas(Text))
	{
		TextSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		TextSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		TextSlot->SetAutoSize(true);
	}
	Text->SetVisibility(ESlateVisibility::Collapsed); // 첫 갱신 전까지 숨김
	return Text;
}

void UPTBFirstPlayGuideWidget::SetIndicatorSizes(int32 InIndicatorFontSize, int32 InMarkerFontSize)
{
	IndicatorFontSize = InIndicatorFontSize;
	MarkerFontSize = InMarkerFontSize;

	auto ApplySize = [](UWidget* Widget, int32 Size)
	{
		if (UTextBlock* Text = Cast<UTextBlock>(Widget))
		{
			FSlateFontInfo Font = Text->GetFont();
			Font.Size = Size;
			Text->SetFont(Font);
		}
	};
	ApplySize(OffscreenIndicator, IndicatorFontSize);
	ApplySize(OnscreenMarker, MarkerFontSize);
}

void UPTBFirstPlayGuideWidget::SetOnscreenMarker(bool bShow, FVector2D ViewportPixelPos)
{
	if (!OnscreenMarker)
	{
		return;
	}
	if (!bShow)
	{
		OnscreenMarker->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	OnscreenMarker->SetVisibility(ESlateVisibility::HitTestInvisible);

	const float ViewportScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), KINDA_SMALL_NUMBER);
	const float Bob = MarkerBobAmplitude * FMath::Sin(PulseTime * 2.f * PI / MarkerBobPeriodSec);
	if (UCanvasPanelSlot* MarkerSlot = Cast<UCanvasPanelSlot>(OnscreenMarker->Slot))
	{
		MarkerSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		// 아래 끝(뾰족한 부분)이 대상 지점에 오도록
		MarkerSlot->SetAlignment(FVector2D(0.5f, 1.f));
		MarkerSlot->SetPosition(ViewportPixelPos / ViewportScale + FVector2D(0.f, Bob));
	}
}

void UPTBFirstPlayGuideWidget::SetOffscreenIndicator(bool bShow, FVector2D ScreenDirection)
{
	if (!OffscreenIndicator)
	{
		return;
	}

	if (!bShow || ScreenDirection.IsNearlyZero())
	{
		OffscreenIndicator->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	OffscreenIndicator->SetVisibility(ESlateVisibility::HitTestInvisible);

	// 화면(UI 단위) 크기
	const float ViewportScale = FMath::Max(UWidgetLayoutLibrary::GetViewportScale(this), KINDA_SMALL_NUMBER);
	const FVector2D ViewSize = UWidgetLayoutLibrary::GetViewportSize(this) / ViewportScale;
	const FVector2D Center = ViewSize * 0.5f;
	const FVector2D HalfExtent(FMath::Max(Center.X - IndicatorEdgeMargin, 1.f), FMath::Max(Center.Y - IndicatorEdgeMargin, 1.f));

	// 중앙에서 방향대로 뻗어서 화면 가장자리(여백 안쪽)에 닿는 지점
	const FVector2D Dir = ScreenDirection.GetSafeNormal();
	const float ScaleX = FMath::Abs(Dir.X) > KINDA_SMALL_NUMBER ? HalfExtent.X / FMath::Abs(Dir.X) : TNumericLimits<float>::Max();
	const float ScaleY = FMath::Abs(Dir.Y) > KINDA_SMALL_NUMBER ? HalfExtent.Y / FMath::Abs(Dir.Y) : TNumericLimits<float>::Max();
	const FVector2D EdgePos = Center + Dir * FMath::Min(ScaleX, ScaleY);

	if (UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(OffscreenIndicator->Slot))
	{
		ArrowSlot->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		ArrowSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		ArrowSlot->SetPosition(EdgePos);
	}

	// ▲(위쪽)이 기준 → 방향 각도 + 90도
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(Dir.Y, Dir.X)) + 90.f;
	OffscreenIndicator->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	OffscreenIndicator->SetRenderTransformAngle(AngleDeg);
}

void UPTBFirstPlayGuideWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::HitTestInvisible);
	PulseTime = 0.f;
	RefreshHintText();
}

void UPTBFirstPlayGuideWidget::SetHintTextVisible(bool bVisible)
{
	if (!TXT_Hint)
	{
		return;
	}
	const ESlateVisibility Wanted = bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed;
	if (TXT_Hint->GetVisibility() != Wanted)
	{
		TXT_Hint->SetVisibility(Wanted);
		if (bVisible)
		{
			RefreshHintText(); // 숨겨진 사이 키 설정이 바뀌었을 수 있음
		}
	}
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

	PulseTime += InDeltaTime;
	if (PulseMinOpacity >= 1.f)
	{
		return;
	}

	// 0~1을 부드럽게 오가는 값 (cos 기반: 처음엔 가장 밝게 시작)
	const float Wave = 0.5f + 0.5f * FMath::Cos(PulseTime * 2.f * PI / PulsePeriodSec);
	const float Opacity = FMath::Lerp(PulseMinOpacity, 1.f, Wave);
	if (TXT_Hint)
	{
		TXT_Hint->SetRenderOpacity(Opacity);
	}
	if (OffscreenIndicator)
	{
		OffscreenIndicator->SetRenderOpacity(FMath::Max(Opacity, 0.7f)); // 방향 표시는 너무 흐려지지 않게
	}
}
