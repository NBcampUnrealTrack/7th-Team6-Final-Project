#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBModalMenuWidget.generated.h"

/**
 * 메뉴 닫힐 때 복원할 입력 모드
 * GameOnly  — 게임플레이 중(일시정지 메뉴)에서 열린 경우
 * UIOnly    — UI 화면(타이틀 옵션)에서 열린 경우
 */
UENUM(BlueprintType)
enum class EPTBMenuRestoreMode : uint8
{
	GameOnly  UMETA(DisplayName = "Game Only"),
	UIOnly    UMETA(DisplayName = "UI Only"),
};

/**
 * 모달 메뉴 베이스 위젯
 *
 * OpenMenu() 호출 시 뷰포트 위에 올라오며 자신에게만 마우스/키 입력 전달
 * CloseMenu() 호출 시 RestoreMode에 따라 입력 모드를 복원하고 뷰포트에서 제거
 * 파생 클래스(옵션 메뉴, 일시정지 메뉴 등)에서 필요한 버튼 로직을 구현
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBModalMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 뷰포트에 추가하고 자신에게 입력 포커스를 설정 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ModalMenu")
	virtual void OpenMenu();

	/** 입력 모드를 복원하고 뷰포트에서 제거 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ModalMenu")
	virtual void CloseMenu();

	/** 닫힐 때 복원할 입력 모드 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ModalMenu")
	EPTBMenuRestoreMode RestoreMode = EPTBMenuRestoreMode::GameOnly;

	/** true 이면 ESC 키로 CloseMenu() 호출 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ModalMenu")
	bool bCloseOnEscape = true;

	/** AddToViewport 에 넘길 ZOrder */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ModalMenu")
	int32 MenuZOrder = 100;

protected:
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	/** 메뉴가 열렸을 때 BP에서 진입 애니메이션 등 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ModalMenu")
	void OnMenuOpened();

	/** 메뉴가 닫히기 직전 BP에서 정리 처리 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ModalMenu")
	void OnMenuClosed();
};