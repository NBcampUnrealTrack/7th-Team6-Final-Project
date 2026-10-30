#include "Judgement/PTBJudgementDisplayWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "Judgement/PTBJudgeSkin.h"

void UPTBJudgementDisplayWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// WBP 없이 C++ 클래스로 생성된 경우 Image를 직접 만들어 루트로 사용
	if (!Image_Judgement && WidgetTree && !WidgetTree->RootWidget)
	{
		Image_Judgement = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("Image_Judgement"));
		WidgetTree->RootWidget = Image_Judgement;
	}

	SetVisibility(ESlateVisibility::Collapsed);
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
}

void UPTBJudgementDisplayWidget::SetScreenAnchor(const FVector2D& ScreenRatio)
{
	bUseWorldAnchor = false;
	ScreenAnchor = ScreenRatio;
	UpdatePosition();
}

void UPTBJudgementDisplayWidget::SetWorldAnchor(const FVector& WorldLocation)
{
	bUseWorldAnchor = true;
	WorldAnchor = WorldLocation;
	UpdatePosition();
}

void UPTBJudgementDisplayWidget::PlayJudgement(EPTBJudgementType JudgementType, const UPTBJudgeSkin* Skin, float ScaleMultiplier)
{
	UTexture2D* Texture = Skin ? Skin->GetTexture(JudgementType) : nullptr;
	if (!Texture || !Image_Judgement)
	{
		// 이 판정에 이미지가 없으면 표시하지 않음 (예: Miss 이미지를 비워 둔 스킨)
		bPlaying = false;
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Image_Judgement->SetBrushFromTexture(Texture, false);
	Image_Judgement->SetDesiredSizeOverride(Skin->ImageSize);
	Image_Judgement->SetColorAndOpacity(Skin->Tint);

	Duration      = FMath::Max(0.05f, Skin->DisplayDuration);
	PopStartScale = Skin->PopStartScale;
	PopInTime     = Skin->PopInTime;
	FadeOutTime   = FMath::Min(Skin->FadeOutTime, Duration);
	RiseDistance  = Skin->RiseDistance;
	BaseScale     = FMath::Max(0.05f, ScaleMultiplier);

	Elapsed = 0.f;
	bPlaying = true;
	SetVisibility(ESlateVisibility::HitTestInvisible);

	UpdatePosition();
	ApplyAnimation();
	OnJudgementDisplayed(JudgementType);
}

void UPTBJudgementDisplayWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bPlaying)
	{
		return;
	}

	Elapsed += InDeltaTime;
	if (Elapsed >= Duration)
	{
		bPlaying = false;
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	UpdatePosition();
	ApplyAnimation();
}

void UPTBJudgementDisplayWidget::UpdatePosition()
{
	FVector2D Position = FVector2D::ZeroVector;

	if (bUseWorldAnchor)
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC || !UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(PC, WorldAnchor, Position, false))
		{
			return; // 화면 뒤쪽 등 변환 실패 시 위치 유지
		}
	}
	else
	{
		const float ViewportScale = FMath::Max(0.01f, UWidgetLayoutLibrary::GetViewportScale(this));
		Position = ScreenAnchor * (UWidgetLayoutLibrary::GetViewportSize(this) / ViewportScale);
	}

	// 위로 떠오르는 연출
	if (bPlaying && !FMath::IsNearlyZero(RiseDistance))
	{
		Position.Y -= RiseDistance * FMath::Clamp(Elapsed / Duration, 0.f, 1.f);
	}

	SetPositionInViewport(Position, false);
}

void UPTBJudgementDisplayWidget::ApplyAnimation()
{
	// 팝: PopStartScale → 1 (ease out)
	float Scale = 1.f;
	if (PopInTime > 0.f && Elapsed < PopInTime)
	{
		const float Alpha = FMath::Clamp(Elapsed / PopInTime, 0.f, 1.f);
		Scale = FMath::InterpEaseOut(PopStartScale, 1.f, Alpha, 2.f);
	}
	SetRenderScale(FVector2D(Scale * BaseScale));

	// 페이드아웃: 마지막 FadeOutTime 동안
	float Opacity = 1.f;
	const float FadeStart = Duration - FadeOutTime;
	if (FadeOutTime > 0.f && Elapsed > FadeStart)
	{
		Opacity = 1.f - FMath::Clamp((Elapsed - FadeStart) / FadeOutTime, 0.f, 1.f);
	}
	SetRenderOpacity(Opacity);
}
