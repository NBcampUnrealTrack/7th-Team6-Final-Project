#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBFirstPlayGuideWidget.generated.h"

class UTextBlock;

/**
 * 첫 플레이 안내 문구 (화면 상단, 천천히 밝아졌다 어두워짐).
 *
 * - WBP 없이 이 클래스를 그대로 써도 기본 레이아웃(상단 가운데 텍스트)을 코드로 만들어서 표시함.
 * - 모양을 꾸미고 싶으면 이 클래스를 부모로 WBP를 만들고 Text Block 이름을 TXT_Hint로 두면 됨.
 * - 대상이 화면 밖이면 화면 가장자리에 방향 화살표(OffscreenIndicator)를 띄움.
 *   WBP로 만들 때는 아무 위젯(그림 등)을 Canvas Panel 바로 아래에 두고 이름을 OffscreenIndicator로. 위쪽(▲)을 가리키는 모양으로 만들면 됨.
 * - 클릭을 막지 않음 (Not Hit-Testable).
 */
UCLASS()
class PARTTIMEBEAT_API UPTBFirstPlayGuideWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** {Interact} 자리에 현재 상호작용 키가 들어감 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	FText HintFormat = NSLOCTEXT("PTBFirstPlayGuide", "Hint", "보라색 기둥에 다가가 [{Interact}] 키로 상호작용하세요!");

	/** 기본 레이아웃(WBP 없이 쓸 때)의 글자 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	int32 DefaultFontSize = 30;

	/** 기본 레이아웃의 화면 위쪽 여백 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	float DefaultTopMargin = 90.f;

	/** 펄스: 가장 흐릴 때 투명도 (1이면 펄스 없음) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide", meta = (ClampMin = "0", ClampMax = "1"))
	float PulseMinOpacity = 0.45f;

	/** 펄스 한 번(밝음→흐림→밝음)에 걸리는 시간(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide", meta = (ClampMin = "0.1"))
	float PulsePeriodSec = 1.6f;

	/** 화면 밖 방향 표시 색 (기본 레이아웃) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	FLinearColor IndicatorColor = FLinearColor(1.f, 0.85f, 0.1f, 1.f);

	/** 화면 밖 방향 표시 크기 (기본 레이아웃 글자 크기) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	int32 IndicatorFontSize = 100;

	/** 화면 안 대상 위 표시(▼) 크기 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	int32 MarkerFontSize = 80;

	/** 화면 안 대상 위 표시가 위아래로 움직이는 폭(UI 단위) / 주기(초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	float MarkerBobAmplitude = 14.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide", meta = (ClampMin = "0.1"))
	float MarkerBobPeriodSec = 1.0f;

	/** 화면 가장자리에서 띄울 여백 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|FirstPlayGuide")
	float IndicatorEdgeMargin = 70.f;

	/**
	 * 화면 밖 방향 표시 갱신 (액터가 매 프레임 호출).
	 * @param ScreenDirection 화면 중앙 기준 대상 방향 (오른쪽 +X, 아래 +Y)
	 */
	void SetOffscreenIndicator(bool bShow, FVector2D ScreenDirection);

	/**
	 * 화면 안 대상 위 표시(▼) 갱신 (액터가 매 프레임 호출). 벽에 가려져도 항상 보임.
	 * @param ViewportPixelPos 화면 픽셀 좌표 (ProjectWorldLocationToScreen 결과)
	 */
	void SetOnscreenMarker(bool bShow, FVector2D ViewportPixelPos);

	/** 표시 크기 변경 (WBP 없이 기본 레이아웃 쓸 때 액터 설정값 적용용) */
	void SetIndicatorSizes(int32 InIndicatorFontSize, int32 InMarkerFontSize);

	/** 상단 문구만 보이기/숨기기 (화면 밖 방향 표시는 별도) */
	void SetHintTextVisible(bool bVisible);

	/** 문구 다시 채우기 (키 설정이 바뀌었을 때 등) */
	UFUNCTION(BlueprintCallable, Category = "PTB|FirstPlayGuide")
	void RefreshHintText();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|FirstPlayGuide")
	TObjectPtr<UTextBlock> TXT_Hint;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|FirstPlayGuide")
	TObjectPtr<UWidget> OffscreenIndicator;

	/** 화면 안에서 대상 위에 떠 있는 표시 (아래를 가리키는 모양). WBP에서는 Canvas Panel 바로 아래에 둘 것 */
	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|FirstPlayGuide")
	TObjectPtr<UWidget> OnscreenMarker;

private:
	/** WBP 레이아웃이 없으면 코드로 기본 레이아웃 생성 */
	void BuildDefaultLayoutIfNeeded();

	float PulseTime = 0.f;

	/** 삼각형 글자 위젯 만들기 (기본 레이아웃용) */
	UTextBlock* MakeTriangleText(class UCanvasPanel* Root, FName Name, const TCHAR* Glyph, int32 FontSize);
};
