#include "UI/PTBKeyTextLibrary.h"

#include "Core/PTBGameInstance.h"
#include "Engine/World.h"

namespace PTBKeyTextInternal
{
	FText GetDefaultLabel(EPTBActionType Action)
	{
		switch (Action)
		{
		case EPTBActionType::ActionA: return FText::FromString(TEXT("Z"));
		case EPTBActionType::ActionB: return FText::FromString(TEXT("X"));
		case EPTBActionType::ActionC: return FText::FromString(TEXT("C"));
		case EPTBActionType::ActionD: return FText::FromString(TEXT("V"));
		case EPTBActionType::ActionE: return FText::FromString(TEXT("B"));
		default:                      return FText::FromString(TEXT("?"));
		}
	}
}

FText UPTBKeyTextLibrary::GetActionKeyLabel(const UObject* WorldContextObject, EPTBActionType Action)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	if (const UPTBGameInstance* GI = World ? Cast<UPTBGameInstance>(World->GetGameInstance()) : nullptr)
	{
		const FKey BoundKey = GI->CachedSettings.RhythmKeys.ResolveAction(Action);
		if (BoundKey.IsValid() && BoundKey != EKeys::Invalid)
		{
			return BoundKey.GetDisplayName();
		}
	}

	return PTBKeyTextInternal::GetDefaultLabel(Action);
}

FText UPTBKeyTextLibrary::FormatKeyTokens(const UObject* WorldContextObject, const FText& SourceText)
{
	if (SourceText.IsEmpty())
	{
		return SourceText;
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("ActionA"), GetActionKeyLabel(WorldContextObject, EPTBActionType::ActionA));
	Args.Add(TEXT("ActionB"), GetActionKeyLabel(WorldContextObject, EPTBActionType::ActionB));
	Args.Add(TEXT("ActionC"), GetActionKeyLabel(WorldContextObject, EPTBActionType::ActionC));
	Args.Add(TEXT("ActionD"), GetActionKeyLabel(WorldContextObject, EPTBActionType::ActionD));
	Args.Add(TEXT("ActionE"), GetActionKeyLabel(WorldContextObject, EPTBActionType::ActionE));

	return FText::Format(SourceText, Args);
}
