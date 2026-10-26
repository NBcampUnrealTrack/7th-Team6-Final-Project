#include "UI/PTBHubProfileWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Items/PTBItemSubsystem.h"
#include "Profile/PTBProfileSubsystem.h"
#include "UI/PTBKeyTextLibrary.h"

UPTBHubProfileWidget::UPTBHubProfileWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MoneyFormat = NSLOCTEXT("PTBHub", "MoneyFormat", "{Money} G");
}

void UPTBHubProfileWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonShop)
	{
		ButtonShop->OnClicked.AddUniqueDynamic(this, &UPTBHubProfileWidget::HandleShopClicked);
	}
	if (ButtonInventory)
	{
		ButtonInventory->OnClicked.AddUniqueDynamic(this, &UPTBHubProfileWidget::HandleInventoryClicked);
	}

	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBProfileSubsystem* Profiles = GI->GetSubsystem<UPTBProfileSubsystem>())
		{
			ProfileListChangedHandle = Profiles->OnProfileListChanged.AddUObject(this, &UPTBHubProfileWidget::HandleProfileListChanged);
		}

		// 구매·장착 시 소지금/초상화 갱신
		if (UPTBItemSubsystem* Items = GI->GetSubsystem<UPTBItemSubsystem>())
		{
			Items->OnItemStateChanged.AddUniqueDynamic(this, &UPTBHubProfileWidget::HandleItemStateChanged);
			Items->OnMoneyChanged.AddUniqueDynamic(this, &UPTBHubProfileWidget::HandleMoneyChanged);
		}
	}

	RefreshProfile();
}

void UPTBHubProfileWidget::NativeDestruct()
{
	if (UGameInstance* GI = GetGameInstance())
	{
		if (UPTBProfileSubsystem* Profiles = GI->GetSubsystem<UPTBProfileSubsystem>())
		{
			Profiles->OnProfileListChanged.Remove(ProfileListChangedHandle);
		}

		if (UPTBItemSubsystem* Items = GI->GetSubsystem<UPTBItemSubsystem>())
		{
			Items->OnItemStateChanged.RemoveDynamic(this, &UPTBHubProfileWidget::HandleItemStateChanged);
			Items->OnMoneyChanged.RemoveDynamic(this, &UPTBHubProfileWidget::HandleMoneyChanged);
		}
	}
	ProfileListChangedHandle.Reset();

	Super::NativeDestruct();
}

int32 UPTBHubProfileWidget::GetCurrentMoney() const
{
	const UGameInstance* GI = GetGameInstance();
	const UPTBProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	return Profiles ? Profiles->GetCurrentMoney() : 0;
}

void UPTBHubProfileWidget::RefreshProfile()
{
	const FText PlayerName = UPTBKeyTextLibrary::GetPlayerName(this);
	const int32 Money = GetCurrentMoney();

	if (TextPlayerName)
	{
		TextPlayerName->SetText(PlayerName);
	}

	if (TextMoney)
	{
		FFormatNamedArguments Args;
		Args.Add(TEXT("Money"), FText::AsNumber(Money));
		TextMoney->SetText(FText::Format(MoneyFormat, Args));
	}

	if (ImagePortrait)
	{
		EPTBGender Gender = EPTBGender::Unset;
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const UPTBProfileSubsystem* Profiles = GI->GetSubsystem<UPTBProfileSubsystem>())
			{
				bool bHasActive = false;
				const FPTBProfileData Profile = Profiles->GetActiveProfile(bHasActive);
				if (bHasActive)
				{
					Gender = Profile.Gender;
				}
			}
		}

		// 1순위: 장착한 초상화 아이템 (Asset에 텍스처가 지정된 경우)
		UTexture2D* Portrait = nullptr;
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const UPTBItemSubsystem* Items = GI->GetSubsystem<UPTBItemSubsystem>())
			{
				bool bFound = false;
				const FPTBItemRow PortraitItem = Items->GetEquippedItemData(PTBItemCategory::Portrait, bFound);
				if (bFound && !PortraitItem.Asset.IsNull())
				{
					Portrait = Cast<UTexture2D>(PortraitItem.Asset.LoadSynchronous());
				}
			}
		}

		// 2순위: 성별별 이미지 → 기본 이미지 (초상화 아이템의 Asset이 비어 있으면 기존 방식 유지)
		if (!Portrait)
		{
			const TObjectPtr<UTexture2D>* Found = PortraitByGender.Find(Gender);
			Portrait = Found && *Found ? Found->Get() : DefaultPortrait.Get();
		}
		if (Portrait)
		{
			ImagePortrait->SetBrushFromTexture(Portrait, false);
		}
	}

	OnProfileRefreshed(PlayerName, Money);
}

void UPTBHubProfileWidget::HandleShopClicked()
{
	OnShopClicked();
}

void UPTBHubProfileWidget::HandleInventoryClicked()
{
	OnInventoryClicked();
}

void UPTBHubProfileWidget::HandleProfileListChanged()
{
	RefreshProfile();
}

void UPTBHubProfileWidget::HandleItemStateChanged(FName Category)
{
	if (Category.IsNone() || Category == PTBItemCategory::Portrait)
	{
		RefreshProfile();
	}
}

void UPTBHubProfileWidget::HandleMoneyChanged(int32 CurrentMoney)
{
	RefreshProfile();
}
