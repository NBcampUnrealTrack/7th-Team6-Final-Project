#pragma once

#include "CoreMinimal.h"
#include "MiniGames/Common/UI/PTBMiniGameLoadingWidget.h"
#include "PTBBBIntroWidget.generated.h"

class UTexture2D;

/**
 * BB 미니게임 전용 인트로 화면.
 *
 * IntroTextures를 순서대로 보여주며 Enter로 다음 페이지로 넘어가고,
 * 마지막 페이지에서 Enter를 누르면 부모 클래스의 시작 처리로 이어져 게임이 시작된다.
 *
 * WBP에서 해야 할 작업:
 *  - HandleReadyToStart 이벤트(부모 클래스 제공)에서 ResetIntroPages() 호출
 *    (화면이 다시 보일 때 항상 첫 페이지부터 시작하기 위함)
 *  - OnIntroPageChanged 이벤트에서 Image 위젯의 브러시를 PageTexture로 교체
 *  - IntroTextures 배열에 Intro1~4 텍스처를 순서대로 할당
 */
UCLASS()
class PARTTIMEBEAT_API UPTBBBIntroWidget : public UPTBMiniGameLoadingWidget
{
	GENERATED_BODY()

public:
	/** 순서대로 표시할 인트로 이미지 (Intro1~Intro4) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|BB|Intro")
	TArray<TObjectPtr<UTexture2D>> IntroTextures;

	/** 첫 페이지로 되돌리고 표시. WBP의 HandleReadyToStart 이벤트에서 호출할 것. */
	UFUNCTION(BlueprintCallable, Category = "PTB|BB|Intro")
	void ResetIntroPages();

	/** 현재 페이지의 텍스처가 바뀔 때 호출. WBP에서 Image 브러시 갱신. */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|BB|Intro")
	void OnIntroPageChanged(UTexture2D* PageTexture, int32 PageIndex);

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
	void ShowCurrentPage();

	int32 CurrentPageIndex = 0;
};
