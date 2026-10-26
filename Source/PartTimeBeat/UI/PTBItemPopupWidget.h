#pragma once

#include "CoreMinimal.h"
#include "UI/PTBHubPopupWidget.h"
#include "Items/PTBItemTypes.h"
#include "PTBItemPopupWidget.generated.h"

class UImage;
class UPanelWidget;
class UTextBlock;
class UPTBItemEntryWidget;
class UPTBItemSubsystem;
class UPTBItemTabWidget;

/**
 * 사물함/상점 공통 베이스.
 * 카테고리 탭 + 아이템 목록 + 선택 아이템 상세 + 동작 버튼(장착/구매) 구성.
 * 목록·탭·텍스트는 아래 이름의 위젯이 있으면 C++이 자동으로 채움 (전부 선택사항, BP 로직 없이 레이아웃만 만들면 동작)
 *
 * WBP 구성
 *  - Panel_Tabs           : 탭이 들어갈 패널 (Horizontal Box 권장). TabWidgetClass로 자동 생성
 *  - Panel_Items          : 아이템 칸이 들어갈 패널 (Wrap Box 권장. Uniform Grid는 칸이 겹치므로 사용 X). EntryWidgetClass로 자동 생성
 *  - Text_Title           : 팝업 제목 (상점은 상점 이름)
 *  - Text_Money           : 소지금 (MoneyFormat)
 *  - Image_SelectedIcon   : 선택 아이템 아이콘
 *  - Text_ItemName        : 선택 아이템 이름
 *  - Text_ItemDescription : 선택 아이템 설명
 *  - Button_Action        : 장착/구매 버튼
 *  - Text_Action          : 버튼 글자 (상황에 따라 "장착", "구매 (300 G)", "골드 부족" 등으로 바뀜)
 *  - Text_Message         : 결과 안내 ("구매 완료!" 등)
 *  - Button_Close         : 닫기 (부모 클래스)
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBItemPopupWidget : public UPTBHubPopupWidget
{
	GENERATED_BODY()

public:
	UPTBItemPopupWidget(const FObjectInitializer& ObjectInitializer);

	virtual void OpenPopup() override;

	/** 탭 선택 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	void SelectCategory(FName CategoryId);

	/** 아이템 선택 (상세 표시 갱신) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	void SelectItem(FName ItemId);

	/** 선택 아이템에 동작 실행 (사물함: 장착, 상점: 구매 또는 장착) */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	void ExecuteSelectedAction();

	/** 탭·목록·상세·소지금 전부 다시 그림 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item")
	void RefreshView();

	UFUNCTION(BlueprintPure, Category = "PTB|Item")
	bool GetSelectedItemView(FPTBItemView& OutView) const;

	/** 아이템 칸 클릭 시 (Entry 위젯에서 호출) */
	virtual void HandleEntryClicked(FName ItemId);

	virtual bool IsShopMode() const { return false; }

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	TArray<FPTBItemCategoryDef> Categories;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FName CurrentCategory = NAME_None;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	TArray<FPTBItemView> CurrentItems;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FName SelectedItemId = NAME_None;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// ── 파생 클래스에서 구현 ──
	virtual TArray<FPTBItemCategoryDef> BuildCategories() const;
	virtual TArray<FPTBItemView> BuildItems(FName CategoryId) const;
	/** 동작 버튼 글자와 활성 여부 */
	virtual bool GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const;
	virtual void DoAction(const FPTBItemView& ItemView);
	virtual FText GetTitleText() const { return FText::GetEmpty(); }

	/** 안내 문구 표시 (Text_Message + BP 이벤트) */
	void ShowMessage(const FText& Message, bool bIsError);

	UPTBItemSubsystem* GetItemSubsystem() const;

	/** 갱신 직후 (BP: 추가 연출) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Item")
	void OnViewRefreshed();

	/** 선택 아이템 변경 (BP: 캐릭터 미리보기 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Item")
	void OnSelectionChanged(const FPTBItemView& SelectedView, bool bHasSelection);

	/** 안내 문구 (BP: 토스트, 효과음 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Item")
	void OnMessage(const FText& Message, bool bIsError);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	TSubclassOf<UPTBItemEntryWidget> EntryWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	TSubclassOf<UPTBItemTabWidget> TabWidgetClass;

	/** 소지금 표시 형식. {Money} = 천 단위 쉼표 숫자 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FText MoneyFormat;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UPanelWidget> Panel_Tabs = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UPanelWidget> Panel_Items = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Title = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Money = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> Image_SelectedIcon = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_ItemName = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_ItemDescription = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> Button_Action = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Action = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Message = nullptr;

private:
	UFUNCTION()
	void HandleActionClicked();

	UFUNCTION()
	void HandleItemStateChanged(FName Category);

	UFUNCTION()
	void HandleMoneyChanged(int32 CurrentMoney);

	void RebuildTabs();
	void RebuildItems();
	void UpdateDetail();
	void UpdateMoney();
};

/**
 * 사물함 (I 키 / ESC 메뉴 / 허브 HUD 버튼).
 * 보유한 아이템만 표시. 칸을 클릭하면 바로 장착 (bEquipOnClick)
 * 기본 닫기 키: ESC, I
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBLockerWidget : public UPTBItemPopupWidget
{
	GENERATED_BODY()

public:
	UPTBLockerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void HandleEntryClicked(FName ItemId) override;

protected:
	virtual TArray<FPTBItemCategoryDef> BuildCategories() const override;
	virtual TArray<FPTBItemView> BuildItems(FName CategoryId) const override;
	virtual bool GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const override;
	virtual void DoAction(const FPTBItemView& ItemView) override;
	virtual FText GetTitleText() const override { return TitleText; }

	/** 칸 클릭 즉시 장착 (false면 선택만 하고 장착 버튼으로 장착) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	bool bEquipOnClick = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText TitleText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelEquip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelEquipped;
};

/**
 * 상점 (허브의 상점 오브젝트에서 Enter).
 * 상점 오브젝트가 InitShop으로 상점 이름과 판매 카테고리를 넘겨줌 → 헤어샵/판정 스킨샵/탈것 가게를 같은 WBP로 재사용 가능
 * 이미 산 아이템은 "장착" 버튼으로 바로 장착 가능
 */
UCLASS(Abstract, Blueprintable)
class PARTTIMEBEAT_API UPTBShopWidget : public UPTBItemPopupWidget
{
	GENERATED_BODY()

public:
	UPTBShopWidget(const FObjectInitializer& ObjectInitializer);

	/** 상점 이름과 판매 카테고리 설정 (OpenPopup 전에 호출). Categories가 비어 있으면 전체 카테고리 */
	UFUNCTION(BlueprintCallable, Category = "PTB|Item|Shop")
	void InitShop(const FText& InShopName, const TArray<FName>& InSoldCategories);

	virtual bool IsShopMode() const override { return true; }

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item|Shop")
	FText ShopName;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item|Shop")
	TArray<FName> SoldCategories;

protected:
	virtual TArray<FPTBItemCategoryDef> BuildCategories() const override;
	virtual TArray<FPTBItemView> BuildItems(FName CategoryId) const override;
	virtual bool GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const override;
	virtual void DoAction(const FPTBItemView& ItemView) override;
	virtual FText GetTitleText() const override { return ShopName; }

	/** 구매 직후 바로 장착 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Shop")
	bool bEquipAfterPurchase = true;

	/** {Price} = 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelBuyFormat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelNotEnoughMoney;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelEquip;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelEquipped;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText LabelUnavailable;

	/** {Item} = 아이템 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText MessagePurchasedFormat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText MessageNotEnoughMoney;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item|Text")
	FText MessagePurchaseFailed;
};
