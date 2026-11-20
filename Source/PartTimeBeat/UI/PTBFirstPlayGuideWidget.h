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

	/** 문구 다시 채우기 (키 설정이 바뀌었을 때 등) */
	UFUNCTION(BlueprintCallable, Category = "PTB|FirstPlayGuide")
	void RefreshHintText();

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|FirstPlayGuide")
	TObjectPtr<UTextBlock> TXT_Hint;

private:
	/** WBP 레이아웃이 없으면 코드로 기본 레이아웃 생성 */
	void BuildDefaultLayoutIfNeeded();

	float PulseTime = 0.f;
};
