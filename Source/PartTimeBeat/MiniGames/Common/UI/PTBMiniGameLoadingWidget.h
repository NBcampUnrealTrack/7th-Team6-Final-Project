#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBMiniGameLoadingWidget.generated.h"

class APTBBaseMiniGame;

/**
 * 미니게임 시작 전 로딩 화면과 시작 대기 화면을 표시하기 위한 공용 Widget입니다
 *
 * 실제 문구, 애니메이션, 버튼 표시는 WBP에서 구현합니다
 */
UCLASS()
class PARTTIMEBEAT_API UPTBMiniGameLoadingWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 소유 미니게임 설정 */
	void InitializeLoadingWidget(APTBBaseMiniGame* InOwnerMiniGame);

	/** 로딩 시작 상태로 전환 */
	void SetLoadingState();

	/** 시작 대기 상태로 전환 */
	void SetReadyToStartState();

	/** 로딩 시작 상태 반영 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Loading")
	void HandleLoadingStarted();

	/** 시작 대기 상태 반영 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Loading")
	void HandleReadyToStart();

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	FReply RequestStartIfReady();

	/** 소유 미니게임 */
	UPROPERTY()
	TObjectPtr<APTBBaseMiniGame> OwnerMiniGame;

	/** 시작 입력 허용 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PTB|Loading")
	bool bCanRequestStart = false;
};
