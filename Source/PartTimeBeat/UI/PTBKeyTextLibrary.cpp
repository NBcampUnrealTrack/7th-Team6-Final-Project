#include "UI/PTBKeyTextLibrary.h"

#include "Core/PTBGameInstance.h"
#include "Engine/World.h"
#include "Items/PTBItemSettings.h"
#include "Profile/PTBProfileSubsystem.h"

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

// 허브 키는 현재 "PTB 아이템 설정"(프로젝트 설정)에서 읽음.
// 나중에 옵션에서 바꿀 수 있게 되면 이 세 함수와 PTBHubUISubsystem::HandleHubKeyPressed만
// 사용자 설정(CachedSettings)을 보도록 바꾸면 문구는 자동으로 따라감
FText UPTBKeyTextLibrary::GetInteractKeyLabel(const UObject* WorldContextObject)
{
	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (Settings && Settings->InteractKeys.Num() > 0 && Settings->InteractKeys[0].IsValid())
	{
		return Settings->InteractKeys[0].GetDisplayName();
	}
	return EKeys::Enter.GetDisplayName();
}

FText UPTBKeyTextLibrary::GetLockerKeyLabel(const UObject* WorldContextObject)
{
	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	if (Settings && Settings->LockerKey.IsValid())
	{
		return Settings->LockerKey.GetDisplayName();
	}
	return EKeys::I.GetDisplayName();
}

bool UPTBKeyTextLibrary::IsInteractKey(const UObject* WorldContextObject, const FKey& Key)
{
	const UPTBItemSettings* Settings = GetDefault<UPTBItemSettings>();
	return Settings ? Settings->InteractKeys.Contains(Key) : (Key == EKeys::Enter);
}

FText UPTBKeyTextLibrary::GetPlayerName(const UObject* WorldContextObject)
{
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	if (const UPTBProfileSubsystem* ProfileSubsystem = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr)
	{
		bool bHasActive = false;
		const FPTBProfileData Profile = ProfileSubsystem->GetActiveProfile(bHasActive);
		if (bHasActive && !Profile.Nickname.IsEmpty())
		{
			return FText::FromString(Profile.Nickname);
		}
	}

	// 프로필 없음(직접 PIE 등) 폴백 호칭
	return NSLOCTEXT("PTBText", "DefaultPlayerName", "알바생");
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
	Args.Add(TEXT("Interact"), GetInteractKeyLabel(WorldContextObject));
	Args.Add(TEXT("Locker"), GetLockerKeyLabel(WorldContextObject));
	Args.Add(TEXT("Player"), GetPlayerName(WorldContextObject));

	return FText::Format(SourceText, Args);
}
