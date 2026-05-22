// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBDialogHostWidget.generated.h"

/**
   * 다이얼로그(팝업) 스택을 관리하는 베이스 위젯
   * 팝업이 필요한 모든 화면 위젯이 이 클래스를 상속
   *
   * 사용 조건:
   *   Blueprint Designer에 이름이 "Overlay_Dialog"인 Overlay 위젯이 있어야 함
   */
  UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBDialogHostWidget : public UUserWidget
  {
  	GENERATED_BODY()

  public:
  	/** 팝업을 Overlay에 추가하고 스택에 쌓기 */
  	UFUNCTION(BlueprintCallable, Category = "Dialog")
  	void PushDialog(UUserWidget* Dialog);

  	/** 가장 위 팝업만 닫기 */
  	UFUNCTION(BlueprintCallable, Category = "Dialog")
  	void CloseTopDialog();

  	/** 열린 팝업 전부 닫기 */
  	UFUNCTION(BlueprintCallable, Category = "Dialog")
  	void CloseAllDialogs();

  	/** 현재 열린 팝업 수 */
  	UFUNCTION(BlueprintPure, Category = "Dialog")
  	int32 GetDialogCount() const { return DialogStack.Num(); }

  	/** 특정 팝업이 스택에 있는지 확인 */
  	UFUNCTION(BlueprintPure, Category = "Dialog")
  	bool IsDialogOpen(TSubclassOf<UUserWidget> DialogClass) const;

  protected:
  	virtual FReply NativeOnKeyDown(
		  const FGeometry& InGeometry,
		  const FKeyEvent& InKeyEvent) override;

  	/** Blueprint에서 팝업이 닫힐 때 추가 처리가 필요하면 오버라이드 */
  	UFUNCTION(BlueprintImplementableEvent, Category = "Dialog")
  	void OnDialogClosed(UUserWidget* ClosedDialog);
  	
  	UPROPERTY(meta = (BindWidget))
  	TObjectPtr<class UOverlay> Overlay_Dialog;

  private:
  	UPROPERTY()
  	TArray<UUserWidget*> DialogStack;
  };
