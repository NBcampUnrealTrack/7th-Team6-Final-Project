#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBHubPopupWidget.generated.h"

class UButton;

/**
 * 허브(L_GameMap)에서 게임을 멈추지 않고 띄우는 팝업 베이스 (사물함, 상점 등).
 *
 * - 열면 UIOnly 입력 모드 → 캐릭터 이동·시야 회전이 막히고 마우스로 UI 조작
 *   (게임은 멈추지 않으므로 장착 변경이 뒤의 캐릭터에 바로 보임)
 * - CloseKeys(기본 ESC)로 닫힘. 팝업이 열린 동안에는 키가 PlayerController까지 가지 않으므로
 *   ESC 일시정지 메뉴와 겹치지 않음
 * - 닫으면 열기 직전의 입력 모드로 복원 (커서가 보이던 상태 → GameAndUI, 아니면 GameOnly)
 *
 * WBP 구성 (이름이 같으면 자동 연결, 선택사항)
 *  - Button_Close : 닫기 버튼
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBHubPopupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPTBHubPopupWidget(const FObjectInitializer& ObjectInitializer);

	/** 뷰포트에 띄우고 입력을 UI로 전환 */
	UFUNCTION(BlueprintCallable, Category = "PTB|HubPopup")
	virtual void OpenPopup();

	/** 입력 모드 복원 후 뷰포트에서 제거 */
	UFUNCTION(BlueprintCallable, Category = "PTB|HubPopup")
	virtual void ClosePopup();

	UFUNCTION(BlueprintPure, Category = "PTB|HubPopup")
	bool IsPopupOpen() const { return bIsOpen; }

	/** 닫힐 때 (HubUI 서브시스템이 활성 팝업 정리에 사용) */
	FSimpleMulticastDelegate OnPopupClosedNative;

protected:
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** 열린 직후 (BP: 등장 애니메이션 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|HubPopup")
	void OnPopupOpened();

	/** 닫히기 직전 (BP: 정리) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|HubPopup")
	void OnPopupClosed();

	/** 이 키들을 누르면 닫힘 (기본: Escape) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|HubPopup")
	TArray<FKey> CloseKeys;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|HubPopup")
	int32 PopupZOrder = 50;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> Button_Close = nullptr;

private:
	UFUNCTION()
	void HandleCloseClicked();

	bool bIsOpen = false;
	bool bPrevCursorVisible = false;
};
