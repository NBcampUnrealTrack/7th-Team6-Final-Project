#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBJudgementDisplayWidget.generated.h"

class UImage;
class UPTBJudgeSkin;

/**
 * 판정 이미지 한 개를 화면에 띄우는 위젯 (공통 판정 표시).
 * PTBJudgementDisplayComponent가 생성·위치 지정하므로 직접 쓸 일은 거의 없음.
 *
 * WBP 없이도 동작 (C++에서 Image를 자동 생성).
 * WBP로 꾸미고 싶으면 이 클래스를 부모로 만들고 Image 이름을 Image_Judgement로 → PTB 아이템 설정의 JudgementDisplayWidgetClass에 지정
 */
UCLASS(Blueprintable)
class PARTTIMEBEAT_API UPTBJudgementDisplayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 판정 표시 시작 (이미 표시 중이면 처음부터 다시) */
	void PlayJudgement(EPTBJudgementType JudgementType, const UPTBJudgeSkin* Skin, float ScaleMultiplier);

	/** 화면 비율 위치 고정 (0~1) */
	void SetScreenAnchor(const FVector2D& ScreenRatio);

	/** 월드 위치 따라가기 (표시되는 동안 매 프레임 화면 좌표로 변환) */
	void SetWorldAnchor(const FVector& WorldLocation);

	/** 뷰포트 위치 고정 (Set Position in Viewport, Remove DPI Scale=false 단위) */
	void SetViewportAnchor(const FVector2D& ViewportPosition);

	bool IsPlaying() const { return bPlaying; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** 표시 시작 직후 (BP: 추가 연출, 파티클 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Judgement")
	void OnJudgementDisplayed(EPTBJudgementType JudgementType);

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> Image_Judgement = nullptr;

private:
	void UpdatePosition();
	void ApplyAnimation();

	bool bPlaying = false;
	float Elapsed = 0.f;
	float BaseScale = 1.f;

	// 현재 스킨의 연출 값 (스킨 에셋 참조 대신 복사)
	float Duration = 0.6f;
	float PopStartScale = 1.4f;
	float PopInTime = 0.08f;
	float FadeOutTime = 0.2f;
	float RiseDistance = 0.f;

	enum class EAnchorMode : uint8 { ScreenRatio, World, Viewport };
	EAnchorMode AnchorMode = EAnchorMode::ScreenRatio;
	FVector WorldAnchor = FVector::ZeroVector;
	FVector2D ScreenAnchor = FVector2D(0.5f, 0.35f);
	FVector2D ViewportAnchor = FVector2D::ZeroVector;
};
