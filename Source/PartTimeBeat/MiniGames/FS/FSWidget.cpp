// Fill out your copyright notice in the Description page of Project Settings.


#include "FSWidget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Blueprint/WidgetTree.h"
#include "UI/PTBKeyTextLibrary.h"
#include "PartTimeBeat/MiniGames/FS/PTBFSMiniGame.h"
#include "Audio/PTBWwiseRhythmSyncComponent.h"

// ───────────────────────── 초기화 ─────────────────────────

void UFSWidget::InitializeWidget(APTBFSMiniGame* InGame)
{
	if (IsValid(FSMiniGame) && FSMiniGame != InGame)
	{
		FSMiniGame->OnRhythmKeyStateChanged.RemoveDynamic(this, &UFSWidget::HandleRhythmKeyStateChanged);
	}

	FSMiniGame = InGame;
	if (IsValid(FSMiniGame))
	{
		FSMiniGame->OnRhythmKeyStateChanged.AddUniqueDynamic(this, &UFSWidget::HandleRhythmKeyStateChanged);
	}

	RefreshKeyGuideLabels();
	ResetKeyGuidePressed();
}

void UFSWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// KeyGuide는 키 셔플로 계속 움직이므로 Designer 원래 배치를 먼저 저장 (= 왼/가운데/오른쪽 칸)
	UWidget* Guides[3] = { KeyGuideA, KeyGuideB, KeyGuideC };
	for (int32 i = 0; i < 3; ++i)
	{
		bHasKeyGuideDesignLayout[i] = false;
		if (const UCanvasPanelSlot* GuideSlot = Guides[i] ? Cast<UCanvasPanelSlot>(Guides[i]->Slot) : nullptr)
		{
			KeyGuideDesignLayout[i] = GuideSlot->GetLayout();
			bHasKeyGuideDesignLayout[i] = true;
		}
	}

	// 화살표 3개는 템플릿 → 숨김 (실제 노트는 복제본)
	if (ArrowLeft)  { ArrowLeft->SetVisibility(ESlateVisibility::Hidden); }
	if (ArrowUp)    { ArrowUp->SetVisibility(ESlateVisibility::Hidden); }
	if (ArrowRight) { ArrowRight->SetVisibility(ESlateVisibility::Hidden); }

	RefreshKeyGuideLabels();
	ResetKeyGuidePressed();
}

void UFSWidget::NativeDestruct()
{
	if (IsValid(FSMiniGame))
	{
		FSMiniGame->OnRhythmKeyStateChanged.RemoveDynamic(this, &UFSWidget::HandleRhythmKeyStateChanged);
	}
	Super::NativeDestruct();
}

void UFSWidget::RefreshKeyGuideLabels()
{
	if (KeyGuideTextA) { KeyGuideTextA->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionA)); }
	if (KeyGuideTextB) { KeyGuideTextB->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionB)); }
	if (KeyGuideTextC) { KeyGuideTextC->SetText(UPTBKeyTextLibrary::GetActionKeyLabel(this, EPTBActionType::ActionC)); }
}

// ───────────────────────── 좌표 변환 ─────────────────────────

bool UFSWidget::ComputeRectFromLayout(const FAnchorData& Layout, const FVector2D& CanvasSize, FVector2D& OutTopLeft, FVector2D& OutSize)
{
	const FAnchors& A = Layout.Anchors;
	const FMargin& O = Layout.Offsets;

	// X축
	if (FMath::IsNearlyEqual(A.Minimum.X, A.Maximum.X))
	{
		OutSize.X = O.Right; // 점 앵커: Right = 너비
		OutTopLeft.X = A.Minimum.X * CanvasSize.X + O.Left - Layout.Alignment.X * OutSize.X;
	}
	else
	{
		OutTopLeft.X = A.Minimum.X * CanvasSize.X + O.Left;
		OutSize.X = A.Maximum.X * CanvasSize.X - O.Right - OutTopLeft.X;
	}

	// Y축
	if (FMath::IsNearlyEqual(A.Minimum.Y, A.Maximum.Y))
	{
		OutSize.Y = O.Bottom; // 점 앵커: Bottom = 높이
		OutTopLeft.Y = A.Minimum.Y * CanvasSize.Y + O.Top - Layout.Alignment.Y * OutSize.Y;
	}
	else
	{
		OutTopLeft.Y = A.Minimum.Y * CanvasSize.Y + O.Top;
		OutSize.Y = A.Maximum.Y * CanvasSize.Y - O.Bottom - OutTopLeft.Y;
	}
	return true;
}

bool UFSWidget::GetWidgetCenter(const UWidget* Widget, const FVector2D& CanvasSize, FVector2D& OutCenter, FVector2D* OutSize)
{
	const UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
	if (!CanvasSlot)
	{
		return false;
	}
	FVector2D TopLeft, Size;
	ComputeRectFromLayout(CanvasSlot->GetLayout(), CanvasSize, TopLeft, Size);
	OutCenter = TopLeft + Size * 0.5f;
	if (OutSize)
	{
		*OutSize = Size;
	}
	return true;
}

bool UFSWidget::SetWidgetCenter(UWidget* Widget, const FVector2D& CanvasSize, const FVector2D& CanvasCenter)
{
	UCanvasPanelSlot* CanvasSlot = Widget ? Cast<UCanvasPanelSlot>(Widget->Slot) : nullptr;
	if (!CanvasSlot)
	{
		return false;
	}
	const FAnchorData Layout = CanvasSlot->GetLayout();
	if (!FMath::IsNearlyEqual(Layout.Anchors.Minimum.X, Layout.Anchors.Maximum.X)
		|| !FMath::IsNearlyEqual(Layout.Anchors.Minimum.Y, Layout.Anchors.Maximum.Y))
	{
		return false; // 늘이기 앵커는 위치 이동 대상 아님
	}
	const FVector2D Size(Layout.Offsets.Right, Layout.Offsets.Bottom);
	// Position = 정렬점 기준. 중심 = 정렬점 + (0.5 - Alignment) * Size
	const FVector2D AlignPoint = CanvasCenter - (FVector2D(0.5f, 0.5f) - Layout.Alignment) * Size;
	CanvasSlot->SetPosition(AlignPoint - Layout.Anchors.Minimum * CanvasSize);
	return true;
}

bool UFSWidget::GetLaneCenter(int32 Lane, const FVector2D& CanvasSize, FVector2D& OutCenter) const
{
	if (Lane < 0 || Lane > 2 || !bHasKeyGuideDesignLayout[Lane])
	{
		return false;
	}
	FVector2D TopLeft, Size;
	ComputeRectFromLayout(KeyGuideDesignLayout[Lane], CanvasSize, TopLeft, Size);
	OutCenter = TopLeft + Size * 0.5f;
	return true;
}

// ───────────────────────── 키 안내 ─────────────────────────

void UFSWidget::UpdateKeyGuide(float VisualTimeMs, const FVector2D& CanvasSize)
{
	FVector2D LaneCenters[3];
	for (int32 Lane = 0; Lane < 3; ++Lane)
	{
		if (!GetLaneCenter(Lane, CanvasSize, LaneCenters[Lane]))
		{
			return; // KeyGuide 3개가 모두 Canvas 바로 아래에 있어야 동작
		}
	}

	UWidget* Guides[3] = { KeyGuideA, KeyGuideB, KeyGuideC };
	bool bSwapping = false;
	for (int32 Key = 0; Key < 3; ++Key)
	{
		if (!Guides[Key])
		{
			continue;
		}

		int32 FromLane = Key;
		int32 ToLane = Key;
		float Alpha = 1.0f;
		if (FSMiniGame)
		{
			FSMiniGame->GetKeyGuideMotion(Key, VisualTimeMs, FromLane, ToLane, Alpha);
		}
		FromLane = FMath::Clamp(FromLane, 0, 2);
		ToLane = FMath::Clamp(ToLane, 0, 2);

		FVector2D Center = LaneCenters[ToLane];
		if (FromLane != ToLane && Alpha < 1.0f)
		{
			bSwapping = true;
			// 빠르게 출발해서 부드럽게 멈추는 곡선
			const float Eased = FMath::InterpEaseOut(0.0f, 1.0f, Alpha, 2.0f);
			Center = FMath::Lerp(LaneCenters[FromLane], LaneCenters[ToLane], Eased);
			const float Direction = ToLane > FromLane ? -1.0f : 1.0f;
			Center.Y += Direction * KeyGuideSwapArcHeight * FMath::Sin(PI * Eased);
		}
		SetWidgetCenter(Guides[Key], CanvasSize, Center);
	}

	if (bSwapping && !bKeyGuideWasSwapping)
	{
		OnKeyGuideSwapStarted();
	}
	bKeyGuideWasSwapping = bSwapping;
}

void UFSWidget::ResetKeyGuidePressed()
{
	UWidget* Guides[3] = { KeyGuideA, KeyGuideB, KeyGuideC };
	UWidget* Presses[3] = { KeyGuidePressA, KeyGuidePressB, KeyGuidePressC };
	for (int32 i = 0; i < 3; ++i)
	{
		if (Guides[i]) { Guides[i]->SetRenderScale(FVector2D(1.0f, 1.0f)); }
		if (Presses[i]) { Presses[i]->SetVisibility(ESlateVisibility::Hidden); }
	}
}

void UFSWidget::HandleRhythmKeyStateChanged(EPTBActionType Action, bool bPressed)
{
	// KeyGuideA = A키 (키 셔플로 칸이 바뀌어도 A키 안내는 항상 KeyGuideA)
	UWidget* Guide = nullptr;
	UWidget* Press = nullptr;
	switch (Action)
	{
	case EPTBActionType::ActionA: Guide = KeyGuideA; Press = KeyGuidePressA; break;
	case EPTBActionType::ActionB: Guide = KeyGuideB; Press = KeyGuidePressB; break;
	case EPTBActionType::ActionC: Guide = KeyGuideC; Press = KeyGuidePressC; break;
	default: return;
	}

	if (Guide)
	{
		const float Scale = bPressed ? KeyGuidePressedScale : 1.0f;
		Guide->SetRenderScale(FVector2D(Scale, Scale));
	}
	if (Press)
	{
		Press->SetVisibility(bPressed ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	OnKeyGuidePressedChanged(Action, bPressed);
}

// ───────────────────────── 노트 화살표 ─────────────────────────

UImage* UFSWidget::GetArrowTemplate(int32 Lane) const
{
	switch (Lane)
	{
	case 0: return ArrowLeft;
	case 1: return ArrowUp;
	case 2: return ArrowRight;
	default: return nullptr;
	}
}

UWidget* UFSWidget::GetArrowTarget(int32 Lane) const
{
	switch (Lane)
	{
	case 0: return Image_left;
	case 1: return Image_up;
	case 2: return Image_right;
	default: return nullptr;
	}
}

void UFSWidget::OnNoteEvent(EPTBActionType Action, bool bLongNote, float InTargetTimeMs, float InCueLeadTimeMs)
{
	// Action = 화면 칸 (키 셔플 반영된 VisualAction). A=왼쪽, B=가운데(위), C=오른쪽
	int32 Lane = INDEX_NONE;
	switch (Action)
	{
	case EPTBActionType::ActionA: Lane = 0; break;
	case EPTBActionType::ActionB: Lane = 1; break;
	case EPTBActionType::ActionC: Lane = 2; break;
	default: return;
	}

	FFSActiveArrow NewArrow;
	NewArrow.Lane = Lane;
	NewArrow.TargetTimeMs = InTargetTimeMs;
	NewArrow.CueLeadTimeMs = FMath::Max(1.0f, InCueLeadTimeMs);

	// 첫 Tick 전(캔버스 크기 모름)이면 대기열에 넣고 Tick에서 생성
	if (LastCanvasSize.IsNearlyZero())
	{
		PendingSpawnArrows.Add(NewArrow);
		return;
	}

	NewArrow.PoolIndex = AcquireArrowFromPool(Lane);
	if (NewArrow.PoolIndex != INDEX_NONE)
	{
		ActiveArrows.Add(NewArrow);
		OnArrowSpawned(ArrowPool[NewArrow.PoolIndex], Action);
	}
}

int32 UFSWidget::AcquireArrowFromPool(int32 Lane)
{
	UImage* Template = GetArrowTemplate(Lane);
	if (!Template || !WidgetTree)
	{
		return INDEX_NONE;
	}
	UCanvasPanel* Canvas = Cast<UCanvasPanel>(Template->GetParent());
	const UCanvasPanelSlot* TemplateSlot = Cast<UCanvasPanelSlot>(Template->Slot);
	if (!Canvas || !TemplateSlot)
	{
		return INDEX_NONE;
	}

	int32 Index = ArrowPoolInUse.Find(false);
	if (Index == INDEX_NONE)
	{
		UImage* NewImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass());
		if (!NewImage)
		{
			return INDEX_NONE;
		}
		UCanvasPanelSlot* NewSlot = Canvas->AddChildToCanvas(NewImage);
		if (!NewSlot)
		{
			return INDEX_NONE;
		}
		// 중심 기준 배치 (위치 계산을 단순하게)
		NewSlot->SetAnchors(FAnchors(0.0f, 0.0f));
		NewSlot->SetAlignment(FVector2D(0.5f, 0.5f));
		NewSlot->SetAutoSize(false);
		Index = ArrowPool.Add(NewImage);
		ArrowPoolInUse.Add(false);
	}

	UImage* Arrow = ArrowPool[Index];
	ArrowPoolInUse[Index] = true;

	// 템플릿 외형 복사 (이미지·색·회전·크기·그리는 순서)
	Arrow->SetBrush(Template->GetBrush());
	Arrow->SetColorAndOpacity(Template->GetColorAndOpacity());
	Arrow->SetRenderTransform(Template->GetRenderTransform());
	Arrow->SetRenderTransformPivot(Template->GetRenderTransformPivot());
	Arrow->SetRenderOpacity(Template->GetRenderOpacity());
	if (UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(Arrow->Slot))
	{
		FVector2D Center, Size;
		GetWidgetCenter(Template, LastCanvasSize, Center, &Size);
		ArrowSlot->SetSize(Size);
		ArrowSlot->SetZOrder(TemplateSlot->GetZOrder());
		ArrowSlot->SetPosition(Center);
	}
	Arrow->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Index;
}

void UFSWidget::ReleaseArrow(int32 PoolIndex)
{
	if (ArrowPool.IsValidIndex(PoolIndex))
	{
		if (ArrowPool[PoolIndex])
		{
			ArrowPool[PoolIndex]->SetVisibility(ESlateVisibility::Collapsed);
		}
		ArrowPoolInUse[PoolIndex] = false;
	}
}

void UFSWidget::UpdateArrows(float CurrentTimeMs, const FVector2D& CanvasSize)
{
	// 첫 Tick 전에 들어온 노트 생성
	if (PendingSpawnArrows.Num() > 0)
	{
		TArray<FFSActiveArrow> Pending = MoveTemp(PendingSpawnArrows);
		PendingSpawnArrows.Reset();
		for (FFSActiveArrow& Arrow : Pending)
		{
			Arrow.PoolIndex = AcquireArrowFromPool(Arrow.Lane);
			if (Arrow.PoolIndex != INDEX_NONE)
			{
				ActiveArrows.Add(Arrow);
			}
		}
	}

	for (int32 i = ActiveArrows.Num() - 1; i >= 0; --i)
	{
		const FFSActiveArrow& State = ActiveArrows[i];
		UImage* Arrow = ArrowPool.IsValidIndex(State.PoolIndex) ? ArrowPool[State.PoolIndex].Get() : nullptr;
		UImage* Template = GetArrowTemplate(State.Lane);
		if (!Arrow || !Template)
		{
			ReleaseArrow(State.PoolIndex);
			ActiveArrows.RemoveAtSwap(i);
			continue;
		}

		const float Alpha = FMath::Clamp((State.TargetTimeMs - CurrentTimeMs) / State.CueLeadTimeMs, 0.0f, 1.0f);
		if (Alpha <= 0.0f)
		{
			ReleaseArrow(State.PoolIndex);
			ActiveArrows.RemoveAtSwap(i);
			continue;
		}

		FVector2D StartCenter, EndCenter;
		GetWidgetCenter(Template, CanvasSize, StartCenter);
		if (!GetWidgetCenter(GetArrowTarget(State.Lane), CanvasSize, EndCenter))
		{
			// 판정 화살표가 없으면 키 안내 칸 위치로
			if (!GetLaneCenter(State.Lane, CanvasSize, EndCenter))
			{
				EndCenter = StartCenter;
			}
		}

		if (UCanvasPanelSlot* ArrowSlot = Cast<UCanvasPanelSlot>(Arrow->Slot))
		{
			ArrowSlot->SetPosition(FMath::Lerp(EndCenter, StartCenter, Alpha));
		}

		// BP에서 템플릿 이미지를 바꾸면(콤보 피버 등) 날아가는 중인 화살표에도 반영
		if (Arrow->GetBrush().GetResourceObject() != Template->GetBrush().GetResourceObject())
		{
			Arrow->SetBrush(Template->GetBrush());
		}
	}
}

// ───────────────────────── 물고기 거리 마커 ─────────────────────────

void UFSWidget::UpdateFishMarker()
{
	if (!bAutoPlaceFishMarker || !FSMiniGame || !DistanceGauge || !FishdisTance)
	{
		return;
	}

	// 실제 화면(절대) 좌표로 계산 → 게이지/마커가 어떤 컨테이너·앵커에 있어도 동작
	const FGeometry& GaugeGeo = DistanceGauge->GetCachedGeometry();
	const FGeometry& MarkerGeo = FishdisTance->GetCachedGeometry();
	const FVector2D GaugeSize = GaugeGeo.GetLocalSize();
	const FVector2D MarkerSize = MarkerGeo.GetLocalSize();
	if (GaugeSize.IsNearlyZero() || MarkerSize.IsNearlyZero())
	{
		return; // 아직 레이아웃 전
	}

	const float Pad = FMath::Clamp(FishMarkerEdgePadding, 0.0f, GaugeSize.Y * 0.5f);
	const FVector2D TopAbs = GaugeGeo.LocalToAbsolute(FVector2D(GaugeSize.X * 0.5f, Pad));
	const FVector2D BottomAbs = GaugeGeo.LocalToAbsolute(FVector2D(GaugeSize.X * 0.5f, GaugeSize.Y - Pad));

	// 마커의 레이아웃 위치(Render Transform 제외) 중심
	const FSlateLayoutTransform& MarkerLayout = MarkerGeo.GetAccumulatedLayoutTransform();
	const FVector2D MarkerCenterAbs = MarkerLayout.TransformPoint(MarkerSize * 0.5f);
	const float LayoutScale = FMath::Max(KINDA_SMALL_NUMBER, MarkerLayout.GetScale());

	// Distance 0 = 잡힘(게이지 위쪽), 1 = 멀어짐(게이지 아래쪽)
	const float Distance = FMath::Clamp(FSMiniGame->GetFishDistance(), 0.0f, 1.0f);
	const float TargetY = FMath::Lerp(TopAbs.Y, BottomAbs.Y, Distance);

	FWidgetTransform Transform = FishdisTance->GetRenderTransform();
	Transform.Translation.Y = (TargetY - MarkerCenterAbs.Y) / LayoutScale;
	FishdisTance->SetRenderTransform(Transform);
}

// ───────────────────────── Tick ─────────────────────────

void UFSWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const FVector2D CanvasSize = MyGeometry.GetLocalSize();
	if (CanvasSize.IsNearlyZero())
	{
		return;
	}
	LastCanvasSize = CanvasSize;

	UpdateFishMarker();

	if (!FSMiniGame) return;
	UPTBWwiseRhythmSyncComponent* Sync = FSMiniGame->GetUPTBWwiseRhythmSyncComponent();
	if (!Sync) return;
	const float CurrentTime = Sync->GetVisualChartTimeMs();

	UpdateKeyGuide(CurrentTime, CanvasSize);
	UpdateArrows(CurrentTime, CanvasSize);
}
