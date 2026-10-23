#include "UI/PTBHubProfileWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
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
	}
	ProfileListChangedHandle.Reset();

	Super::NativeDestruct();
}

int32 UPTBHubProfileWidget::GetCurrentMoney() const
{
	const UGameInstance* GI = GetGameInstance();
	const UPTBProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	return Profiles ? Profiles->GetTotalEarnedMoney() : 0;
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

		const TObjectPtr<UTexture2D>* Found = PortraitByGender.Find(Gender);
		UTexture2D* Portrait = Found && *Found ? Found->Get() : DefaultPortrait.Get();
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
