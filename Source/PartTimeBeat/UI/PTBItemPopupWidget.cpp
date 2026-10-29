#include "UI/PTBItemPopupWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Items/PTBItemSubsystem.h"
#include "UI/PTBItemEntryWidget.h"

// ══════════════════════════════════════════════════════════════════
// 공통 베이스
// ══════════════════════════════════════════════════════════════════

UPTBItemPopupWidget::UPTBItemPopupWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MoneyFormat = NSLOCTEXT("PTBItem", "PopupMoneyFormat", "{Money} G");
}

void UPTBItemPopupWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Action)
	{
		Button_Action->OnClicked.AddUniqueDynamic(this, &UPTBItemPopupWidget::HandleActionClicked);
	}

	if (UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		Items->OnItemStateChanged.AddUniqueDynamic(this, &UPTBItemPopupWidget::HandleItemStateChanged);
		Items->OnMoneyChanged.AddUniqueDynamic(this, &UPTBItemPopupWidget::HandleMoneyChanged);
	}
}

void UPTBItemPopupWidget::NativeDestruct()
{
	if (UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		Items->OnItemStateChanged.RemoveDynamic(this, &UPTBItemPopupWidget::HandleItemStateChanged);
		Items->OnMoneyChanged.RemoveDynamic(this, &UPTBItemPopupWidget::HandleMoneyChanged);
	}

	Super::NativeDestruct();
}

void UPTBItemPopupWidget::OpenPopup()
{
	Categories = BuildCategories();

	const bool bCurrentValid = Categories.ContainsByPredicate([this](const FPTBItemCategoryDef& Def)
	{
		return Def.CategoryId == CurrentCategory;
	});
	if (!bCurrentValid)
	{
		CurrentCategory = Categories.Num() > 0 ? Categories[0].CategoryId : NAME_None;
		SelectedItemId = NAME_None;
	}

	if (Text_Message)
	{
		Text_Message->SetText(FText::GetEmpty());
	}

	RefreshView();
	Super::OpenPopup();
}

UPTBItemSubsystem* UPTBItemPopupWidget::GetItemSubsystem() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UPTBItemSubsystem>() : nullptr;
}

void UPTBItemPopupWidget::SelectCategory(FName CategoryId)
{
	if (CategoryId == CurrentCategory)
	{
		return;
	}
	CurrentCategory = CategoryId;
	SelectedItemId = NAME_None;
	RefreshView();
}

void UPTBItemPopupWidget::SelectItem(FName ItemId)
{
	SelectedItemId = ItemId;
	RebuildItems();   // 선택 테두리 갱신
	UpdateDetail();
}

void UPTBItemPopupWidget::HandleEntryClicked(FName ItemId)
{
	SelectItem(ItemId);
}

void UPTBItemPopupWidget::ExecuteSelectedAction()
{
	FPTBItemView View;
	if (!GetSelectedItemView(View))
	{
		return;
	}

	FText Label;
	if (!GetActionState(View, Label))
	{
		return;
	}

	DoAction(View);
}

bool UPTBItemPopupWidget::GetSelectedItemView(FPTBItemView& OutView) const
{
	const FPTBItemView* Found = CurrentItems.FindByPredicate([this](const FPTBItemView& V)
	{
		return V.ItemId == SelectedItemId;
	});
	if (Found)
	{
		OutView = *Found;
		return true;
	}
	return false;
}

void UPTBItemPopupWidget::RefreshView()
{
	CurrentItems = BuildItems(CurrentCategory);

	// 선택 유지. 목록에 없으면 장착 중인 아이템 → 첫 아이템 순으로 선택
	FPTBItemView Dummy;
	if (!GetSelectedItemView(Dummy))
	{
		const FPTBItemView* Equipped = CurrentItems.FindByPredicate([](const FPTBItemView& V) { return V.bEquipped; });
		SelectedItemId = Equipped ? Equipped->ItemId
			: (CurrentItems.Num() > 0 ? CurrentItems[0].ItemId : NAME_None);
	}

	if (Text_Title)
	{
		Text_Title->SetText(GetTitleText());
	}

	RebuildTabs();
	RebuildItems();
	UpdateDetail();
	UpdateMoney();

	OnViewRefreshed();
}

void UPTBItemPopupWidget::RebuildTabs()
{
	if (!Panel_Tabs || !TabWidgetClass)
	{
		return;
	}

	// 탭 구성이 같으면 선택 표시만 갱신 (탭 클릭 처리 중 위젯 삭제 방지)
	if (Panel_Tabs->GetChildrenCount() == Categories.Num())
	{
		bool bSameTabs = true;
		for (int32 i = 0; i < Categories.Num(); ++i)
		{
			const UPTBItemTabWidget* Tab = Cast<UPTBItemTabWidget>(Panel_Tabs->GetChildAt(i));
			if (!Tab || Tab->Category.CategoryId != Categories[i].CategoryId)
			{
				bSameTabs = false;
				break;
			}
		}

		if (bSameTabs)
		{
			for (int32 i = 0; i < Categories.Num(); ++i)
			{
				UPTBItemTabWidget* Tab = Cast<UPTBItemTabWidget>(Panel_Tabs->GetChildAt(i));
				Tab->SetupTab(this, Categories[i], Categories[i].CategoryId == CurrentCategory);
			}
			return;
		}
	}

	Panel_Tabs->ClearChildren();
	for (const FPTBItemCategoryDef& Def : Categories)
	{
		if (UPTBItemTabWidget* Tab = CreateWidget<UPTBItemTabWidget>(this, TabWidgetClass))
		{
			Tab->SetupTab(this, Def, Def.CategoryId == CurrentCategory);
			Panel_Tabs->AddChild(Tab);
		}
	}
}

void UPTBItemPopupWidget::RebuildItems()
{
	if (!Panel_Items || !EntryWidgetClass)
	{
		return;
	}

	// 같은 아이템 목록이면 칸을 다시 만들지 않고 표시만 갱신 (스크롤 위치 유지, 클릭 처리 중 위젯 삭제 방지)
	if (Panel_Items->GetChildrenCount() == CurrentItems.Num())
	{
		bool bSameList = true;
		for (int32 i = 0; i < CurrentItems.Num(); ++i)
		{
			const UPTBItemEntryWidget* Entry = Cast<UPTBItemEntryWidget>(Panel_Items->GetChildAt(i));
			if (!Entry || Entry->View.ItemId != CurrentItems[i].ItemId)
			{
				bSameList = false;
				break;
			}
		}

		if (bSameList)
		{
			for (int32 i = 0; i < CurrentItems.Num(); ++i)
			{
				UPTBItemEntryWidget* Entry = Cast<UPTBItemEntryWidget>(Panel_Items->GetChildAt(i));
				Entry->SetupEntry(this, CurrentItems[i], CurrentItems[i].ItemId == SelectedItemId, IsShopMode());
			}
			return;
		}
	}

	Panel_Items->ClearChildren();
	for (const FPTBItemView& View : CurrentItems)
	{
		if (UPTBItemEntryWidget* Entry = CreateWidget<UPTBItemEntryWidget>(this, EntryWidgetClass))
		{
			Entry->SetupEntry(this, View, View.ItemId == SelectedItemId, IsShopMode());
			Panel_Items->AddChild(Entry);
		}
	}
}

void UPTBItemPopupWidget::UpdateDetail()
{
	FPTBItemView View;
	const bool bHasSelection = GetSelectedItemView(View);

	if (Text_ItemName)
	{
		Text_ItemName->SetText(bHasSelection ? View.Data.DisplayName : FText::GetEmpty());
	}
	if (Text_ItemDescription)
	{
		Text_ItemDescription->SetText(bHasSelection ? View.Data.Description : FText::GetEmpty());
	}
	if (Image_SelectedIcon)
	{
		UTexture2D* Texture = (bHasSelection && !View.Data.Icon.IsNull()) ? View.Data.Icon.LoadSynchronous() : nullptr;
		Image_SelectedIcon->SetBrushFromTexture(Texture, false);
		Image_SelectedIcon->SetColorAndOpacity(Texture ? FLinearColor::White : (bHasSelection ? View.Data.Color : FLinearColor::Transparent));
	}

	FText Label;
	const bool bActionEnabled = bHasSelection && GetActionState(View, Label);
	if (Button_Action)
	{
		Button_Action->SetIsEnabled(bActionEnabled);
		Button_Action->SetVisibility(bHasSelection ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
	if (Text_Action)
	{
		Text_Action->SetText(Label);
	}

	OnSelectionChanged(View, bHasSelection);
}

void UPTBItemPopupWidget::UpdateMoney()
{
	if (!Text_Money)
	{
		return;
	}

	const UPTBItemSubsystem* Items = GetItemSubsystem();
	FFormatNamedArguments Args;
	Args.Add(TEXT("Money"), FText::AsNumber(Items ? Items->GetCurrentMoney() : 0));
	Text_Money->SetText(FText::Format(MoneyFormat, Args));
}

void UPTBItemPopupWidget::ShowMessage(const FText& Message, bool bIsError)
{
	if (Text_Message)
	{
		Text_Message->SetText(Message);
	}
	OnMessage(Message, bIsError);
}

void UPTBItemPopupWidget::HandleActionClicked()
{
	ExecuteSelectedAction();
}

void UPTBItemPopupWidget::HandleItemStateChanged(FName Category)
{
	if (IsPopupOpen())
	{
		RefreshView();
	}
}

void UPTBItemPopupWidget::HandleMoneyChanged(int32 CurrentMoney)
{
	if (IsPopupOpen())
	{
		UpdateMoney();
		UpdateDetail(); // 골드 부족 → 구매 가능으로 바뀔 수 있음
	}
}

TArray<FPTBItemCategoryDef> UPTBItemPopupWidget::BuildCategories() const
{
	const UPTBItemSubsystem* Items = GetItemSubsystem();
	return Items ? Items->GetVisibleCategories() : TArray<FPTBItemCategoryDef>();
}

TArray<FPTBItemView> UPTBItemPopupWidget::BuildItems(FName CategoryId) const
{
	return TArray<FPTBItemView>();
}

bool UPTBItemPopupWidget::GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const
{
	OutLabel = FText::GetEmpty();
	return false;
}

void UPTBItemPopupWidget::DoAction(const FPTBItemView& ItemView)
{
}

// ══════════════════════════════════════════════════════════════════
// 사물함
// ══════════════════════════════════════════════════════════════════

UPTBLockerWidget::UPTBLockerWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeys.AddUnique(EKeys::I);

	TitleText     = NSLOCTEXT("PTBItem", "LockerTitle", "인벤토리");
	LabelEquip    = NSLOCTEXT("PTBItem", "LockerEquip", "장착");
	LabelEquipped = NSLOCTEXT("PTBItem", "LockerEquipped", "장착 중");
}

TArray<FPTBItemCategoryDef> UPTBLockerWidget::BuildCategories() const
{
	return Super::BuildCategories();
}

TArray<FPTBItemView> UPTBLockerWidget::BuildItems(FName CategoryId) const
{
	const UPTBItemSubsystem* Items = GetItemSubsystem();
	return Items ? Items->GetOwnedItemsInCategory(CategoryId) : TArray<FPTBItemView>();
}

bool UPTBLockerWidget::GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const
{
	if (ItemView.bEquipped)
	{
		OutLabel = LabelEquipped;
		return false;
	}
	OutLabel = LabelEquip;
	return ItemView.bOwned;
}

void UPTBLockerWidget::DoAction(const FPTBItemView& ItemView)
{
	if (UPTBItemSubsystem* Items = GetItemSubsystem())
	{
		Items->TryEquip(ItemView.ItemId); // 성공 시 OnItemStateChanged → RefreshView
	}
}

void UPTBLockerWidget::HandleEntryClicked(FName ItemId)
{
	SelectItem(ItemId);
	if (bEquipOnClick)
	{
		ExecuteSelectedAction();
	}
}

// ══════════════════════════════════════════════════════════════════
// 상점
// ══════════════════════════════════════════════════════════════════

UPTBShopWidget::UPTBShopWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	LabelBuyFormat         = NSLOCTEXT("PTBItem", "ShopBuyFormat", "구매 ({Price} G)");
	LabelNotEnoughMoney    = NSLOCTEXT("PTBItem", "ShopNotEnoughMoney", "골드 부족");
	LabelEquip             = NSLOCTEXT("PTBItem", "ShopEquip", "장착");
	LabelEquipped          = NSLOCTEXT("PTBItem", "ShopEquipped", "장착 중");
	LabelUnavailable       = NSLOCTEXT("PTBItem", "ShopUnavailable", "구매 불가");
	MessagePurchasedFormat = NSLOCTEXT("PTBItem", "ShopPurchased", "{Item} 구매 완료!");
	MessageNotEnoughMoney  = NSLOCTEXT("PTBItem", "ShopMsgNotEnoughMoney", "골드가 부족해요.");
	MessagePurchaseFailed  = NSLOCTEXT("PTBItem", "ShopMsgFailed", "구매할 수 없는 아이템이에요.");
}

void UPTBShopWidget::InitShop(const FText& InShopName, const TArray<FName>& InSoldCategories)
{
	ShopName = InShopName;
	SoldCategories = InSoldCategories;
	CurrentCategory = NAME_None; // OpenPopup에서 첫 판매 카테고리로 맞춤
}

TArray<FPTBItemCategoryDef> UPTBShopWidget::BuildCategories() const
{
	TArray<FPTBItemCategoryDef> All = Super::BuildCategories();
	if (SoldCategories.Num() == 0)
	{
		return All;
	}

	// 상점 오브젝트에 지정한 순서대로
	TArray<FPTBItemCategoryDef> Result;
	for (const FName& Id : SoldCategories)
	{
		if (const FPTBItemCategoryDef* Found = All.FindByPredicate([Id](const FPTBItemCategoryDef& Def) { return Def.CategoryId == Id; }))
		{
			Result.Add(*Found);
		}
	}
	return Result;
}

TArray<FPTBItemView> UPTBShopWidget::BuildItems(FName CategoryId) const
{
	const UPTBItemSubsystem* Items = GetItemSubsystem();
	return Items ? Items->GetShopItemsInCategory(CategoryId) : TArray<FPTBItemView>();
}

bool UPTBShopWidget::GetActionState(const FPTBItemView& ItemView, FText& OutLabel) const
{
	if (ItemView.bOwned)
	{
		OutLabel = ItemView.bEquipped ? LabelEquipped : LabelEquip;
		return !ItemView.bEquipped;
	}

	const UPTBItemSubsystem* Items = GetItemSubsystem();
	const EPTBPurchaseResult Check = Items ? Items->CanPurchase(ItemView.ItemId) : EPTBPurchaseResult::NoActiveProfile;
	switch (Check)
	{
	case EPTBPurchaseResult::Success:
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Price"), FText::AsNumber(ItemView.Data.Price));
		OutLabel = FText::Format(LabelBuyFormat, Args);
		return true;
	}
	case EPTBPurchaseResult::NotEnoughMoney:
		OutLabel = LabelNotEnoughMoney;
		return false;
	default:
		OutLabel = LabelUnavailable;
		return false;
	}
}

void UPTBShopWidget::DoAction(const FPTBItemView& ItemView)
{
	UPTBItemSubsystem* Items = GetItemSubsystem();
	if (!Items)
	{
		return;
	}

	if (ItemView.bOwned)
	{
		Items->TryEquip(ItemView.ItemId);
		return;
	}

	const EPTBPurchaseResult Result = Items->TryPurchase(ItemView.ItemId, bEquipAfterPurchase);
	switch (Result)
	{
	case EPTBPurchaseResult::Success:
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Item"), ItemView.Data.DisplayName);
		ShowMessage(FText::Format(MessagePurchasedFormat, Args), false);
		break;
	}
	case EPTBPurchaseResult::NotEnoughMoney:
		ShowMessage(MessageNotEnoughMoney, true);
		break;
	default:
		ShowMessage(MessagePurchaseFailed, true);
		break;
	}
}
