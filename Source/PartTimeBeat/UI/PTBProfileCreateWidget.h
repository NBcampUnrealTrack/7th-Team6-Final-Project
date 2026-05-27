// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PTBProfileCreateWidget.generated.h"

class UEditableTextBox;

/**
 * 프로필 생성 화면 베이스 위젯.
 *
 * 모델 선택 (← →), 확인 (Enter) 키 입력 처리
 * 닉네임 유효성 검사 후 OnConfirmSuccess 를 BP에 전달
 * 실제 프로필 저장은 BP 에서 GameInstance::CreateProfile → SaveGame 으로 처리
 */
UCLASS()
class PARTTIMEBEAT_API UPTBProfileCreateWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 이전 모델로 포커스 이동 (← 키) */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	void SelectPrevModel();

	/** 다음 모델로 포커스 이동 (→ 키) */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	void SelectNextModel();
	
	UFUNCTION(BlueprintPure, Category = "PTB|UI|ProfileCreate")
	int32 GetSelectedModelIndex() const { return SelectedModelIndex; }
	
	UFUNCTION(BlueprintPure, Category = "PTB|UI|ProfileCreate")
	FString GetNickname() const;

	/**
	 * 닉네임 유효성 검사 후 확인 처리
	 * 성공 시 OnConfirmSuccess 호출, 실패 시 OnConfirmFailed 호출
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	bool TryConfirm();

	/** 닉네임 최소/최대 길이 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	int32 MinNicknameLength = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	int32 MaxNicknameLength = 12;

	// 모델 개수 (기본 2: 남/여)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	int32 ModelCount = 2;

protected:
	virtual void NativeConstruct() override;

	/** 텍스트 필드가 포커스를 가지지 않을 때 키 입력 처리
	 *  ← → : 모델 선택  /  Enter : TryConfirm
	 */
	virtual FReply NativeOnKeyDown(
		const FGeometry& InGeometry,
		const FKeyEvent& InKeyEvent) override;

	/** 모델 선택이 바뀔 때 BP에서 시각 업데이트 (밝기, 테두리 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnModelSelectionChanged(int32 NewIndex);

	/** TryConfirm 성공 시 호출 — BP에서 프로필 저장 후 레벨 전환 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnConfirmSuccess(const FString& Nickname, int32 ModelIndex);

	/** TryConfirm 실패 시 호출 — BP에서 경고 메시지 표시 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnConfirmFailed(const FString& Reason);
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Nickname;

private:
	int32 SelectedModelIndex = 0;
};