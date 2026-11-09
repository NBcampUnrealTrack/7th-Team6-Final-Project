#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Profile/PTBProfileSubsystem.h"
#include "PTBProfileInfoDialogWidget.generated.h"

class UButton;
class UPTBConfirmDialogWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProfileInfoSlotEvent, int32, SlotIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProfileInfoClosed);

/**
 * 프로필 조회 창 (WBP_ProfileInfoDialog의 부모). 기존 프로필 슬롯을 눌렀을 때 뜸.
 *
 * 표시: 캐릭터(성별), 이름, 소지금, 진행도 → SetupForSlot 후 OnSummaryUpdated(BP)에서 화면 갱신
 * 버튼(이름을 맞추면 자동 연결): Btn_StartGame, Btn_EditProfile, Btn_DeleteProfile, Btn_Close
 * 삭제: DeleteConfirmDialogClass(WBP_DeleteProfileDialog)를 띄우고 "예"를 누르면 삭제
 */
UCLASS()
class PARTTIMEBEAT_API UPTBProfileInfoDialogWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 표시할 슬롯 지정 (Create Widget 직후 호출). 프로필이 없으면 false */
	UFUNCTION(BlueprintCallable, Category = "Profile|InfoDialog")
	bool SetupForSlot(int32 InSlotIndex);

	/** 현재 표시 중인 프로필 요약 */
	UPROPERTY(BlueprintReadOnly, Category = "Profile|InfoDialog")
	FPTBProfileSummary Summary;

	/** "5 / 9" 형식 진행도 텍스트 */
	UFUNCTION(BlueprintPure, Category = "Profile|InfoDialog")
	FText GetProgressText() const;

	/** 삭제 확인 창 클래스 (WBP_DeleteProfileDialog, 부모는 PTBConfirmDialogWidget). 비우면 확인 없이 바로 삭제 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|InfoDialog")
	TSubclassOf<UPTBConfirmDialogWidget> DeleteConfirmDialogClass;

	/** 게임 시작 후 이동할 맵을 직접 지정할 때만 사용 (비우면 Flow의 미니게임 선택 맵) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|InfoDialog")
	FName StartLevelNameOverride = NAME_None;

	/** 프로필 수정 화면 맵. 수정 버튼을 누르면 슬롯을 예약하고 이 맵으로 이동 → 생성 화면이 수정 모드로 열림 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Profile|InfoDialog")
	FName ProfileEditLevelName = TEXT("L_ProfileCreate");

	/** 프로필 수정 버튼 (알림용. ProfileEditLevelName이 비어 있을 때만 직접 처리 필요) */
	UPROPERTY(BlueprintAssignable, Category = "Profile|InfoDialog")
	FOnProfileInfoSlotEvent OnEditRequested;

	/** 프로필 삭제 완료 */
	UPROPERTY(BlueprintAssignable, Category = "Profile|InfoDialog")
	FOnProfileInfoSlotEvent OnProfileDeleted;

	/** 창이 닫힘 */
	UPROPERTY(BlueprintAssignable, Category = "Profile|InfoDialog")
	FOnProfileInfoClosed OnInfoDialogClosed;

	/** 게임 시작 (다른 흐름이 필요하면 BP에서 override) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Profile|InfoDialog")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Profile|InfoDialog")
	void RequestEdit();

	/** 삭제 확인 창 띄우기 */
	UFUNCTION(BlueprintCallable, Category = "Profile|InfoDialog")
	void RequestDelete();

	/** 실제 삭제 (확인 창의 "예"에서 호출됨) */
	UFUNCTION(BlueprintCallable, Category = "Profile|InfoDialog")
	void ConfirmDelete();

	UFUNCTION(BlueprintCallable, Category = "Profile|InfoDialog")
	void CloseDialog();

protected:
	virtual void NativeConstruct() override;

	/** 요약이 갱신됐을 때 — BP에서 캐릭터 이미지(Summary.Gender)·이름·소지금·진행도 갱신 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Profile|InfoDialog")
	void OnSummaryUpdated(const FPTBProfileSummary& InSummary);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_StartGame;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_EditProfile;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_DeleteProfile;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Btn_Close;

private:
	UPTBProfileSubsystem* GetProfileSubsystem() const;

	int32 SlotIndex = INDEX_NONE;
	bool bSetupDone = false;
};
