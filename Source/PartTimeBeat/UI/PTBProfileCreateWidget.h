// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/PTBStructEnums.h"
#include "PTBProfileCreateWidget.generated.h"

class UEditableTextBox;
class UButton;

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
	
	/** 해당 모델을 바로 선택 (이미지 클릭용). 범위 밖이면 무시 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	void SelectModel(int32 ModelIndex);

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

	/**
	 * 모델 인덱스 → 성별 (생성 BP에서 쓰는 매핑과 같아야 함). 기본: 0=남, 1=여.
	 * 프로필 수정 모드에서 기존 성별로 모델을 미리 선택하고, 저장할 때 성별로 바꾸는 데 사용.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	TArray<EPTBGender> ModelGenders = { EPTBGender::Male, EPTBGender::Female };

	// ── 프로필 수정 모드 ──

	/**
	 * 이 화면을 기존 프로필 수정 모드로 전환. 이름 입력칸·모델 선택을 기존 값으로 채움.
	 * Create Widget 직후(화면에 붙이기 전/후 상관없음) 호출. 프로필이 없으면 false.
	 */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	bool BeginEditProfile(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "PTB|UI|ProfileCreate")
	bool IsEditMode() const { return bEditMode; }

	UFUNCTION(BlueprintPure, Category = "PTB|UI|ProfileCreate")
	int32 GetEditingSlotIndex() const { return EditingSlotIndex; }

	/**
	 * 프로필 선택 화면이 있는 맵.
	 * 뒤로가기(생성/수정 모두, 저장 안 함)와 수정 저장 후 이 맵으로 돌아감. 비우면 이동하지 않고 이벤트만 호출.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	FName ProfileSelectLevelName = NAME_None;

	/**
	 * 생성 모드에서 확인 시 C++이 프로필 생성 → 활성화 → 바로 게임 시작(허브 이동)까지 처리.
	 * 끄면 예전처럼 OnConfirmSuccess만 호출하고 생성·이동은 BP가 처리.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|UI|ProfileCreate")
	bool bCreateAndStartGame = true;

	/** 뒤로가기: 생성/수정 내용을 저장하지 않고 프로필 선택 맵으로 (Btn_Back, Esc) */
	UFUNCTION(BlueprintCallable, Category = "PTB|UI|ProfileCreate")
	void GoBack();

	/** 모델 인덱스에 해당하는 성별 */
	UFUNCTION(BlueprintPure, Category = "PTB|UI|ProfileCreate")
	EPTBGender GetGenderForModelIndex(int32 ModelIndex) const;

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

	/** 수정 모드로 전환됐을 때 — BP에서 제목("프로필 수정")·버튼 문구 변경 */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnEditModeStarted(const FString& CurrentNickname, int32 CurrentModelIndex);

	/** 수정 모드에서 TryConfirm 성공 — 이미 저장까지 끝난 상태 (이후 ProfileSelectLevelName으로 이동) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnEditConfirmed(const FString& Nickname, int32 ModelIndex);

	/** 생성 모드에서 C++이 프로필을 만든 직후 (bCreateAndStartGame=true일 때, 게임 시작 이동 직전) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnProfileCreated(const FString& Nickname, int32 ModelIndex);

	/** 뒤로가기 시 (ProfileSelectLevelName이 비어 있으면 BP에서 직접 이동 처리) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|UI|ProfileCreate")
	void OnBackRequested();

	/** WBP에 이 이름의 버튼이 있으면 자동 연결 (기존 BP On Clicked 이벤트는 지울 것) */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Confirm;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Back;

	UFUNCTION()
	void HandleConfirmClicked();

	/** 캐릭터 이미지 버튼 (Img_Model0/1을 Button으로 감싸고 이 이름으로). 있으면 클릭 시 자동 선택 */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Model0;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Model1;

	UFUNCTION()
	void HandleModel0Clicked();

	UFUNCTION()
	void HandleModel1Clicked();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Nickname;

private:
	int32 SelectedModelIndex = 0;

	bool bEditMode = false;
	int32 EditingSlotIndex = INDEX_NONE;
	FGuid EditingProfileId;

	/** 닉네임 검사 결과 → 실패 메시지 */
	FString GetValidationFailMessage(EPTBNicknameValidationResult Result) const;
};