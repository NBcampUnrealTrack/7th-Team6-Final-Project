#include "Items/PTBItemSettings.h"

const FPTBItemCategoryDef* UPTBItemSettings::FindCategory(FName CategoryId) const
{
	if (CategoryId.IsNone())
	{
		return nullptr;
	}
	return Categories.FindByPredicate([CategoryId](const FPTBItemCategoryDef& Def)
	{
		return Def.CategoryId == CategoryId;
	});
}
