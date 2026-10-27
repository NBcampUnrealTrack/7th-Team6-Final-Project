#include "Items/PTBItemSubsystem.h"

#include "Engine/DataTable.h"
#include "Items/PTBItemSettings.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Core/PTBStructEnums.h"
#include "Debug/PTBTeamLog.h"

void UPTBItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 프로필 서브시스템이 먼저 초기화(세이브 로드)되어 있어야 함
	Collection.InitializeDependency(UPTBProfileSubsystem::StaticClass());

	LoadCatalog();
	ValidateCatalog();

	if (UPTBProfileSubsystem* Profiles = GetProfileSubsystem())
	{
		ActiveProfileChangedHandle = Profiles->OnActiveProfileChanged.AddUObject(this, &UPTBItemSubsystem::HandleActiveProfileChanged);
	}

	PTB_RECORD(LogPTBProfile, TEXT("[Item] Initialize - items=%d categories_with_default=%d"),
		ItemsById.Num(), DefaultItemByCategory.Num());
}

void UPTBItemSubsystem::Deinitialize()
{
	if (UPTBProfileSubsystem* Profiles = GetProfileSubsystem())
	{
		Profiles->OnActiveProfileChanged.Remove(ActiveProfileChangedHandle);
	}
	ActiveProfileChangedHandle.Reset();

	ItemsById.Empty();
	DefaultItemByCategory.Empty();
	ItemTable = nullptr;

	Super::Deinitialize();
}

// ── 카탈로그 ─────────────────────────────────────────────────────

void UPTBItemSubsystem::LoadCatalog()
{
	ItemsById.Empty();
	DefaultItemByCategory.Empty();

	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (!Settings || Settings->ItemTable.IsNull())
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Item] ItemTable이 설정되지 않음 (Project Settings → Game → PTB 아이템 설정)"));
		return;
	}

	ItemTable = Settings->ItemTable.LoadSynchronous();
	if (!ItemTable)
	{
		PTB_ERROR(LogPTBProfile, TEXT("[Item] ItemTable 로드 실패: %s (패키징 빌드라면 Additional Asset Directories to Cook 확인)"),
			*Settings->ItemTable.ToString());
		return;
	}

	const UScriptStruct* RowStruct = ItemTable->GetRowStruct();
	if (!RowStruct || !RowStruct->IsChildOf(FPTBItemRow::StaticStruct()))
	{
		PTB_ERROR(LogPTBProfile, TEXT("[Item] ItemTable의 Row Structure가 PTBItemRow가 아님: %s"), *ItemTable->GetName());
		ItemTable = nullptr;
		return;
	}

	for (const TPair<FName, uint8*>& Pair : ItemTable->GetRowMap())
	{
		const FPTBItemRow* Row = reinterpret_cast<const FPTBItemRow*>(Pair.Value);
		if (!Row)
		{
			continue;
		}

		// 행 복사본 보관 (에디터에서 PIE 중 DataTable을 수정해도 포인터가 깨지지 않도록)
		ItemsById.Add(Pair.Key, *Row);

		if (Row->bIsDefault && !DefaultItemByCategory.Contains(Row->Category))
		{
			DefaultItemByCategory.Add(Row->Category, Pair.Key);
		}
	}
}

void UPTBItemSubsystem::ValidateCatalog() const
{
	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (!Settings)
	{
		return;
	}

	TMap<FName, int32> DefaultCountByCategory;
	for (const TPair<FName, FPTBItemRow>& Pair : ItemsById)
	{
		const FPTBItemRow& Row = Pair.Value;

		if (!Settings->FindCategory(Row.Category))
		{
			PTB_WARNING(LogPTBProfile, TEXT("[Item] %s: Category '%s'가 PTB 아이템 설정의 Categories에 없음"),
				*Pair.Key.ToString(), *Row.Category.ToString());
		}

		if (Row.bIsDefault)
		{
			++DefaultCountByCategory.FindOrAdd(Row.Category, 0);
		}
	}

	for (const FPTBItemCategoryDef& Def : Settings->Categories)
	{
		const int32 Count = DefaultCountByCategory.FindRef(Def.CategoryId);
		if (Count == 0)
		{
			PTB_WARNING(LogPTBProfile, TEXT("[Item] 카테고리 '%s'에 기본 아이템(bIsDefault)이 없음"), *Def.CategoryId.ToString());
		}
		else if (Count > 1)
		{
			PTB_WARNING(LogPTBProfile, TEXT("[Item] 카테고리 '%s'에 기본 아이템이 %d개 (첫 번째 행만 사용)"), *Def.CategoryId.ToString(), Count);
		}
	}
}

TArray<FPTBItemCategoryDef> UPTBItemSubsystem::GetVisibleCategories() const
{
	TArray<FPTBItemCategoryDef> Result;
	if (const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>())
	{
		for (const FPTBItemCategoryDef& Def : Settings->Categories)
		{
			if (Def.bVisible)
			{
				Result.Add(Def);
			}
		}
	}
	return Result;
}

FPTBItemRow UPTBItemSubsystem::GetItemData(FName ItemId, bool& bOutFound) const
{
	const FPTBItemRow* Row = FindItem(ItemId);
	bOutFound = Row != nullptr;
	return Row ? *Row : FPTBItemRow();
}

FName UPTBItemSubsystem::GetDefaultItemId(FName Category) const
{
	return DefaultItemByCategory.FindRef(Category);
}

const FPTBItemRow* UPTBItemSubsystem::FindItem(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return nullptr;
	}
	return ItemsById.Find(ItemId);
}

// ── 목록 ─────────────────────────────────────────────────────────

TArray<FPTBItemView> UPTBItemSubsystem::GetOwnedItemsInCategory(FName Category) const
{
	return BuildViews(Category, /*bOwnedOnly*/ true, /*bShopOnly*/ false);
}

TArray<FPTBItemView> UPTBItemSubsystem::GetShopItemsInCategory(FName Category) const
{
	return BuildViews(Category, /*bOwnedOnly*/ false, /*bShopOnly*/ true);
}

TArray<FPTBItemView> UPTBItemSubsystem::BuildViews(FName Category, bool bOwnedOnly, bool bShopOnly) const
{
	TArray<FPTBItemView> Result;

	const FPTBProfileData* Profile = GetActiveProfileConst();
	const FName EquippedId = Profile ? ResolveEquipped(*Profile, Category) : GetDefaultItemId(Category);

	for (const TPair<FName, FPTBItemRow>& Pair : ItemsById)
	{
		const FPTBItemRow& Row = Pair.Value;
		if (Row.Category != Category)
		{
			continue;
		}

		// 기본 아이템은 상점에 진열하지 않음
		if (bShopOnly && (!Row.bSoldInShop || Row.bIsDefault))
		{
			continue;
		}

		const bool bOwned = Row.bIsDefault || (Profile && IsOwnedInProfile(*Profile, Pair.Key, Row));
		if (bOwnedOnly && !bOwned)
		{
			continue;
		}

		FPTBItemView& View = Result.AddDefaulted_GetRef();
		View.ItemId = Pair.Key;
		View.Data = Row;
		View.bOwned = bOwned;
		View.bEquipped = (Pair.Key == EquippedId);
	}

	// 기본 아이템 → SortOrder → ItemId 순
	Result.Sort([](const FPTBItemView& A, const FPTBItemView& B)
	{
		if (A.Data.bIsDefault != B.Data.bIsDefault)
		{
			return A.Data.bIsDefault;
		}
		if (A.Data.SortOrder != B.Data.SortOrder)
		{
			return A.Data.SortOrder < B.Data.SortOrder;
		}
		return A.ItemId.LexicalLess(B.ItemId);
	});

	return Result;
}

// ── 상태 조회 ────────────────────────────────────────────────────

bool UPTBItemSubsystem::IsOwnedInProfile(const FPTBProfileData& Profile, FName ItemId, const FPTBItemRow& Row) const
{
	return Row.bIsDefault || Profile.OwnedItemIds.Contains(ItemId);
}

FName UPTBItemSubsystem::ResolveEquipped(const FPTBProfileData& Profile, FName Category) const
{
	if (const FName* Stored = Profile.EquippedItemByCategory.Find(Category))
	{
		// 저장된 장착 아이템이 아직 유효한지 확인 (DataTable에서 삭제·카테고리 변경 등 대비)
		const FPTBItemRow* Row = FindItem(*Stored);
		if (Row && Row->Category == Category && IsOwnedInProfile(Profile, *Stored, *Row))
		{
			return *Stored;
		}
	}
	return GetDefaultItemId(Category);
}

bool UPTBItemSubsystem::IsItemOwned(FName ItemId) const
{
	const FPTBItemRow* Row = FindItem(ItemId);
	if (!Row)
	{
		return false;
	}
	if (Row->bIsDefault)
	{
		return true;
	}
	const FPTBProfileData* Profile = GetActiveProfileConst();
	return Profile && IsOwnedInProfile(*Profile, ItemId, *Row);
}

bool UPTBItemSubsystem::IsItemEquipped(FName ItemId) const
{
	const FPTBItemRow* Row = FindItem(ItemId);
	return Row && GetEquippedItemId(Row->Category) == ItemId;
}

FName UPTBItemSubsystem::GetEquippedItemId(FName Category) const
{
	const FPTBProfileData* Profile = GetActiveProfileConst();
	return Profile ? ResolveEquipped(*Profile, Category) : GetDefaultItemId(Category);
}

FPTBItemRow UPTBItemSubsystem::GetEquippedItemData(FName Category, bool& bOutFound) const
{
	return GetItemData(GetEquippedItemId(Category), bOutFound);
}

int32 UPTBItemSubsystem::GetCurrentMoney() const
{
	const FPTBProfileData* Profile = GetActiveProfileConst();
	return Profile ? Profile->GetCurrentMoney() : 0;
}

// ── 행동 ─────────────────────────────────────────────────────────

EPTBPurchaseResult UPTBItemSubsystem::CanPurchase(FName ItemId) const
{
	const FPTBItemRow* Row = FindItem(ItemId);
	if (!Row)
	{
		return EPTBPurchaseResult::InvalidItem;
	}

	const FPTBProfileData* Profile = GetActiveProfileConst();
	if (!Profile)
	{
		return EPTBPurchaseResult::NoActiveProfile;
	}

	if (IsOwnedInProfile(*Profile, ItemId, *Row))
	{
		return EPTBPurchaseResult::AlreadyOwned;
	}

	if (!Row->bSoldInShop)
	{
		return EPTBPurchaseResult::NotForSale;
	}

	if (Profile->GetCurrentMoney() < Row->Price)
	{
		return EPTBPurchaseResult::NotEnoughMoney;
	}

	return EPTBPurchaseResult::Success;
}

EPTBPurchaseResult UPTBItemSubsystem::TryPurchase(FName ItemId, bool bEquipAfterPurchase)
{
	const EPTBPurchaseResult Check = CanPurchase(ItemId);
	if (Check != EPTBPurchaseResult::Success)
	{
		PTB_RECORD(LogPTBProfile, TEXT("[Item] TryPurchase rejected: item=%s result=%d"),
			*ItemId.ToString(), static_cast<int32>(Check));
		return Check;
	}

	UPTBProfileSubsystem* Profiles = GetProfileSubsystem();
	FPTBProfileData* Profile = GetActiveProfileMutable();
	const FPTBItemRow* Row = FindItem(ItemId);
	if (!Profiles || !Profile || !Row)
	{
		return EPTBPurchaseResult::NoActiveProfile;
	}

	Profile->SpentMoney += Row->Price;
	Profile->OwnedItemIds.Add(ItemId);

	if (bEquipAfterPurchase)
	{
		Profile->EquippedItemByCategory.Add(Row->Category, ItemId);
	}

	const int32 MoneyAfter = Profile->GetCurrentMoney();
	const FName Category = Row->Category;

	PTB_RECORD(LogPTBProfile, TEXT("[Item] Purchase: item=%s price=%d money_after=%d equip=%d"),
		*ItemId.ToString(), Row->Price, MoneyAfter, bEquipAfterPurchase ? 1 : 0);

	// 구매는 즉시 저장 (돈이 빠졌는데 아이템이 저장 안 되는 상황 방지)
	Profiles->RequestSave();

	OnMoneyChanged.Broadcast(MoneyAfter);
	OnItemStateChanged.Broadcast(Category);

	return EPTBPurchaseResult::Success;
}

bool UPTBItemSubsystem::TryEquip(FName ItemId)
{
	const FPTBItemRow* Row = FindItem(ItemId);
	UPTBProfileSubsystem* Profiles = GetProfileSubsystem();
	FPTBProfileData* Profile = GetActiveProfileMutable();
	if (!Row || !Profiles || !Profile)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Item] TryEquip failed: item=%s (invalid item or no active profile)"), *ItemId.ToString());
		return false;
	}

	if (!IsOwnedInProfile(*Profile, ItemId, *Row))
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Item] TryEquip failed: item=%s not owned"), *ItemId.ToString());
		return false;
	}

	if (ResolveEquipped(*Profile, Row->Category) == ItemId)
	{
		return true; // 이미 장착 중
	}

	if (Row->bIsDefault)
	{
		// 기본 아이템은 기록을 지워서 "기본값 따름" 상태로 둠 (나중에 기본 아이템이 바뀌어도 자연스럽게 따라감)
		Profile->EquippedItemByCategory.Remove(Row->Category);
	}
	else
	{
		Profile->EquippedItemByCategory.Add(Row->Category, ItemId);
	}

	PTB_RECORD(LogPTBProfile, TEXT("[Item] Equip: category=%s item=%s"), *Row->Category.ToString(), *ItemId.ToString());

	Profiles->RequestSave();
	OnItemStateChanged.Broadcast(Row->Category);
	return true;
}

bool UPTBItemSubsystem::GrantItem(FName ItemId)
{
	const FPTBItemRow* Row = FindItem(ItemId);
	UPTBProfileSubsystem* Profiles = GetProfileSubsystem();
	FPTBProfileData* Profile = GetActiveProfileMutable();
	if (!Row || !Profiles || !Profile)
	{
		return false;
	}

	if (IsOwnedInProfile(*Profile, ItemId, *Row))
	{
		return false;
	}

	Profile->OwnedItemIds.Add(ItemId);

	PTB_RECORD(LogPTBProfile, TEXT("[Item] Grant: item=%s"), *ItemId.ToString());

	Profiles->RequestSave();
	OnItemStateChanged.Broadcast(Row->Category);
	return true;
}

// ── 내부 ─────────────────────────────────────────────────────────

UPTBProfileSubsystem* UPTBItemSubsystem::GetProfileSubsystem() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
}

const FPTBProfileData* UPTBItemSubsystem::GetActiveProfileConst() const
{
	const UPTBProfileSubsystem* Profiles = GetProfileSubsystem();
	return Profiles ? Profiles->FindActiveProfileConst() : nullptr;
}

FPTBProfileData* UPTBItemSubsystem::GetActiveProfileMutable() const
{
	UPTBProfileSubsystem* Profiles = GetProfileSubsystem();
	return Profiles ? Profiles->FindActiveProfileMutable() : nullptr;
}

void UPTBItemSubsystem::HandleActiveProfileChanged(const FPTBProfileData& NewProfile)
{
	// 프로필이 바뀌면 보유·장착·소지금이 통째로 바뀜 → 전체 갱신 알림
	OnMoneyChanged.Broadcast(NewProfile.GetCurrentMoney());
	OnItemStateChanged.Broadcast(NAME_None);
}
