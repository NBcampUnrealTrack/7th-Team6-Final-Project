#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/PTBItemTypes.h"
#include "PTBItemEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UWidget;
class UPTBItemPopupWidget;

/**
 * 사물함/상점 목록의 아이템 한 칸.
 * 팝업이 목록을 갱신할 때 자동 생성 (팝업의 EntryWidgetClass에 이 클래스를 부모로 한 WBP 지정)
 *
 * WBP 구성 (이름이 같으면 자동 연결, 선택사항)
 *  - Button_Entry    : 칸 전체 클릭 영역 (Button)
 *  - Image_Icon      : 아이콘. Icon이 비어 있으면 아이템 Color로 칠함 (헤어 색 견본용)
 *  - Text_Name       : 아이템 이름
 *  - Text_Price      : 가격 (상점에서 미보유일 때만 표시)
 *  - Widget_Equipped : 장착 중 표시 (아무 위젯. 장착 중이면 Visible)
 *  - Widget_Owned    : 보유 표시 (상점에서 이미 산 아이템이면 Visible)
 *  - Widget_Selected : 선택 테두리 등 (선택된 칸이면 Visible)
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBItemEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 팝업에서 호출. 표시 갱신 + OnEntryUpdated 호출 */
	void SetupEntry(UPTBItemPopupWidget* InOwner, const FPTBItemView& InView, bool bInSelected, bool bInShopMode);

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FPTBItemView View;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	bool bSelected = false;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	bool bShopMode = false;

protected:
	virtual void NativeConstruct() override;

	/** 기본 표시 갱신 직후 (BP: 추가 연출, 색상 변경 등) */
	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Item")
	void OnEntryUpdated(const FPTBItemView& InView, bool bInSelected, bool bInShopMode);

	/** 가격 표시 형식. {Price} = 가격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "PTB|Item")
	FText PriceFormat;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> Button_Entry = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> Image_Icon = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Name = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Price = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UWidget> Widget_Equipped = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UWidget> Widget_Owned = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UWidget> Widget_Selected = nullptr;

public:
	UPTBItemEntryWidget(const FObjectInitializer& ObjectInitializer);

private:
	UFUNCTION()
	void HandleClicked();

	void ApplyView();

	TWeakObjectPtr<UPTBItemPopupWidget> Owner;
};

/**
 * 사물함/상점 상단 카테고리 탭 한 개.
 *
 * WBP 구성 (선택사항)
 *  - Button_Tab      : 탭 클릭 (Button)
 *  - Text_Tab        : 탭 이름
 *  - Image_Icon      : 탭 아이콘 (카테고리 정의의 Icon)
 *  - Widget_Selected : 선택된 탭 표시
 */
UCLASS(Abstract)
class PARTTIMEBEAT_API UPTBItemTabWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupTab(UPTBItemPopupWidget* InOwner, const FPTBItemCategoryDef& InCategory, bool bInSelected);

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	FPTBItemCategoryDef Category;

	UPROPERTY(BlueprintReadOnly, Category = "PTB|Item")
	bool bSelected = false;

protected:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "PTB|Item")
	void OnTabUpdated(const FPTBItemCategoryDef& InCategory, bool bInSelected);

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UButton> Button_Tab = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UTextBlock> Text_Tab = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UImage> Image_Icon = nullptr;

	UPROPERTY(meta = (BindWidgetOptional), BlueprintReadOnly, Category = "PTB|UI")
	TObjectPtr<UWidget> Widget_Selected = nullptr;

private:
	UFUNCTION()
	void HandleClicked();

	void ApplyView();

	TWeakObjectPtr<UPTBItemPopupWidget> Owner;
};
