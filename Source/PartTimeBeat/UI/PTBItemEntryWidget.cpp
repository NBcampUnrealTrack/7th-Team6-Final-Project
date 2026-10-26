#include "UI/PTBItemEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "UI/PTBItemPopupWidget.h"

namespace PTBItemEntryInternal
{
	void SetVisibleIf(UWidget* Widget, bool bVisible)
	{
		if (Widget)
		{
			Widget->SetVisibility(bVisible ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	}

	void ApplyIcon(UImage* Image, const TSoftObjectPtr<UTexture2D>& Icon, const FLinearColor& FallbackColor)
	{
		if (!Image)
		{
			return;
		}

		if (UTexture2D* Texture = Icon.IsNull() ? nullptr : Icon.LoadSynchronous())
		{
			Image->SetBrushFromTexture(Texture, false);
			Image->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			// 아이콘이 없으면 단색 견본 (헤어 색 등)
			Image->SetBrushFromTexture(nullptr, false);
			Image->SetColorAndOpacity(FallbackColor);
		}
	}
}

// ── 아이템 칸 ─────────────────────────────────────────────────────

UPTBItemEntryWidget::UPTBItemEntryWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PriceFormat = NSLOCTEXT("PTBItem", "EntryPriceFormat", "{Price} G");
}

void UPTBItemEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Entry)
	{
		Button_Entry->OnClicked.AddUniqueDynamic(this, &UPTBItemEntryWidget::HandleClicked);
	}
	ApplyView();
}

void UPTBItemEntryWidget::SetupEntry(UPTBItemPopupWidget* InOwner, const FPTBItemView& InView, bool bInSelected, bool bInShopMode)
{
	Owner = InOwner;
	View = InView;
	bSelected = bInSelected;
	bShopMode = bInShopMode;
	ApplyView();
}

void UPTBItemEntryWidget::ApplyView()
{
	using namespace PTBItemEntryInternal;

	ApplyIcon(Image_Icon, View.Data.Icon, View.Data.Color);

	if (Text_Name)
	{
		Text_Name->SetText(View.Data.DisplayName);
	}

	const bool bShowPrice = bShopMode && !View.bOwned;
	if (Text_Price)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Price"), FText::AsNumber(View.Data.Price));
		Text_Price->SetText(FText::Format(PriceFormat, Args));
	}
	SetVisibleIf(Text_Price, bShowPrice);

	SetVisibleIf(Widget_Equipped, View.bEquipped);
	SetVisibleIf(Widget_Owned, bShopMode && View.bOwned);
	SetVisibleIf(Widget_Selected, bSelected);

	OnEntryUpdated(View, bSelected, bShopMode);
}

void UPTBItemEntryWidget::HandleClicked()
{
	if (UPTBItemPopupWidget* Popup = Owner.Get())
	{
		Popup->HandleEntryClicked(View.ItemId);
	}
}

// ── 카테고리 탭 ───────────────────────────────────────────────────

void UPTBItemTabWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_Tab)
	{
		Button_Tab->OnClicked.AddUniqueDynamic(this, &UPTBItemTabWidget::HandleClicked);
	}
	ApplyView();
}

void UPTBItemTabWidget::SetupTab(UPTBItemPopupWidget* InOwner, const FPTBItemCategoryDef& InCategory, bool bInSelected)
{
	Owner = InOwner;
	Category = InCategory;
	bSelected = bInSelected;
	ApplyView();
}

void UPTBItemTabWidget::ApplyView()
{
	using namespace PTBItemEntryInternal;

	if (Text_Tab)
	{
		Text_Tab->SetText(Category.DisplayName.IsEmpty() ? FText::FromName(Category.CategoryId) : Category.DisplayName);
	}

	if (Image_Icon)
	{
		const bool bHasIcon = !Category.Icon.IsNull();
		if (bHasIcon)
		{
			ApplyIcon(Image_Icon, Category.Icon, FLinearColor::White);
		}
		SetVisibleIf(Image_Icon, bHasIcon);
	}

	SetVisibleIf(Widget_Selected, bSelected);

	OnTabUpdated(Category, bSelected);
}

void UPTBItemTabWidget::HandleClicked()
{
	if (UPTBItemPopupWidget* Popup = Owner.Get())
	{
		Popup->SelectCategory(Category.CategoryId);
	}
}
