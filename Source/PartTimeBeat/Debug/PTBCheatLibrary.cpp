#include "Debug/PTBCheatLibrary.h"

#include "Achievements/PTBAchievementSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Items/PTBItemSubsystem.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Progression/PTBProgressLibrary.h"
#include "Progression/PTBProgressSettings.h"

#if PTB_WITH_CHEATS
namespace PTBCheatInternal
{
	UGameInstance* GetGameInstance(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameInstance() : nullptr;
	}

	template <typename T>
	T* GetSubsystem(const UObject* WorldContextObject)
	{
		UGameInstance* GI = GetGameInstance(WorldContextObject);
		return GI ? GI->GetSubsystem<T>() : nullptr;
	}

	void EvaluateAchievements(const UObject* WorldContextObject)
	{
		if (UPTBAchievementSubsystem* Achievements = GetSubsystem<UPTBAchievementSubsystem>(WorldContextObject))
		{
			Achievements->EvaluateAchievements();
		}
	}

	/** 패널 상태 (개발 빌드 전용이라 정적 보관) */
	TWeakObjectPtr<UUserWidget> CheatPanel;
	bool bPrevShowMouseCursor = false;
}
#endif

bool UPTBCheatLibrary::IsCheatAvailable()
{
	return PTB_WITH_CHEATS;
}

FPTBProfileData* UPTBCheatLibrary::GetActiveProfileMutable(const UObject* WorldContextObject, UPTBProfileSubsystem*& OutProfiles)
{
#if PTB_WITH_CHEATS
	OutProfiles = PTBCheatInternal::GetSubsystem<UPTBProfileSubsystem>(WorldContextObject);
	FPTBProfileData* Profile = OutProfiles ? OutProfiles->FindActiveProfileMutable() : nullptr;
	if (!Profile)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Cheat] 활성 프로필 없음 (프로필을 선택한 뒤 사용)"));
	}
	return Profile;
#else
	OutProfiles = nullptr;
	return nullptr;
#endif
}

void UPTBCheatLibrary::NotifyMoneyChanged(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	if (UPTBItemSubsystem* Items = PTBCheatInternal::GetSubsystem<UPTBItemSubsystem>(WorldContextObject))
	{
		Items->OnMoneyChanged.Broadcast(Items->GetCurrentMoney());
	}
#endif
}

// ── 알바비 ───────────────────────────────────────────────────────

void UPTBCheatLibrary::CheatSetMoney(const UObject* WorldContextObject, int32 CurrentMoney)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		// 소지금 = 누적 수입 - 사용 금액 → 사용 금액으로 맞춤 (음수 허용)
		Profile->SpentMoney = Profile->TotalEarnedMoney - CurrentMoney;
		Profiles->RequestSave();
		NotifyMoneyChanged(WorldContextObject);
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] SetMoney: %d"), CurrentMoney);
	}
#endif
}

void UPTBCheatLibrary::CheatAddMoney(const UObject* WorldContextObject, int32 Amount)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		Profile->SpentMoney -= Amount;
		Profiles->RequestSave();
		NotifyMoneyChanged(WorldContextObject);
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] AddMoney: %+d → %d"), Amount, Profile->GetCurrentMoney());
	}
#endif
}

void UPTBCheatLibrary::CheatSetTotalEarnedMoney(const UObject* WorldContextObject, int32 TotalEarned)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		const int32 Current = Profile->GetCurrentMoney();
		Profile->TotalEarnedMoney = TotalEarned;
		Profile->SpentMoney = TotalEarned - Current;   // 소지금 유지
		Profiles->RequestSave();
		NotifyMoneyChanged(WorldContextObject);
		PTBCheatInternal::EvaluateAchievements(WorldContextObject);
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] SetTotalEarnedMoney: %d"), TotalEarned);
	}
#endif
}

// ── 아이템 ───────────────────────────────────────────────────────

void UPTBCheatLibrary::CheatGrantItem(const UObject* WorldContextObject, FName ItemId)
{
#if PTB_WITH_CHEATS
	if (UPTBItemSubsystem* Items = PTBCheatInternal::GetSubsystem<UPTBItemSubsystem>(WorldContextObject))
	{
		const bool bGranted = Items->GrantItem(ItemId);
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] GrantItem: %s → %s"), *ItemId.ToString(), bGranted ? TEXT("지급") : TEXT("실패(없는 ID / 이미 보유 / 프로필 없음)"));
	}
#endif
}

void UPTBCheatLibrary::CheatGrantAllItems(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	if (UPTBItemSubsystem* Items = PTBCheatInternal::GetSubsystem<UPTBItemSubsystem>(WorldContextObject))
	{
		int32 Count = 0;
		for (const FName& ItemId : Items->GetAllItemIds())
		{
			Count += Items->GrantItem(ItemId) ? 1 : 0;
		}
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] GrantAllItems: %d개 지급"), Count);
	}
#endif
}

void UPTBCheatLibrary::CheatResetItems(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		Profile->OwnedItemIds.Reset();
		Profile->EquippedItemByCategory.Reset();
		Profiles->RequestSave();
		if (UPTBItemSubsystem* Items = PTBCheatInternal::GetSubsystem<UPTBItemSubsystem>(WorldContextObject))
		{
			Items->OnItemStateChanged.Broadcast(NAME_None);   // None = 전체 갱신
		}
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] ResetItems"));
	}
#endif
}

// ── 난이도 해금 / 기록 ───────────────────────────────────────────

void UPTBCheatLibrary::CheatUnlockDifficulty(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles);
	if (!Profile)
	{
		return;
	}

	TArray<FName> Targets;
	if (MiniGameId.IsNone())
	{
		if (const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>())
		{
			Targets = Settings->ProgressMiniGameIds;
		}
	}
	else
	{
		Targets.Add(MiniGameId);
	}

	for (const FName& Target : Targets)
	{
		Profile->ForceUnlockedDifficulties.Add(UPTBProgressLibrary::MakeDifficultyKey(Target, Difficulty));
	}
	Profiles->RequestSave();
	PTB_RECORD(LogPTBProfile, TEXT("[Cheat] UnlockDifficulty: %s %s"),
		MiniGameId.IsNone() ? TEXT("(전체)") : *MiniGameId.ToString(), *UEnum::GetValueAsString(Difficulty));
#endif
}

void UPTBCheatLibrary::CheatResetForcedUnlocks(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		Profile->ForceUnlockedDifficulties.Reset();
		Profiles->RequestSave();
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] ResetForcedUnlocks"));
	}
#endif
}

void UPTBCheatLibrary::CheatAddRecord(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty, EPTBGradeType Grade, int32 Times)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles);
	if (!Profile || MiniGameId.IsNone())
	{
		return;
	}

	Times = FMath::Max(1, Times);
	const FName Key = UPTBProgressLibrary::MakeDifficultyKey(MiniGameId, Difficulty);
	Profile->PlayCountByDifficulty.FindOrAdd(Key, 0) += Times;
	if (Grade != EPTBGradeType::Fail)
	{
		Profile->ClearCountByDifficulty.FindOrAdd(Key, 0) += Times;
	}
	EPTBGradeType& Best = Profile->BestGradeByDifficulty.FindOrAdd(Key, EPTBGradeType::Fail);
	if (UPTBProgressLibrary::IsGradeAtLeast(Grade, Best))
	{
		Best = Grade;
	}

	Profiles->RequestSave();
	PTBCheatInternal::EvaluateAchievements(WorldContextObject);
	PTB_RECORD(LogPTBProfile, TEXT("[Cheat] AddRecord: %s Grade=%s ×%d"), *Key.ToString(), *UEnum::GetValueAsString(Grade), Times);
#endif
}

void UPTBCheatLibrary::CheatResetRecords(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	if (FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles))
	{
		Profile->PlayCountByDifficulty.Reset();
		Profile->ClearCountByDifficulty.Reset();
		Profile->BestGradeByDifficulty.Reset();
		Profiles->RequestSave();
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] ResetRecords"));
	}
#endif
}

// ── 업적 ─────────────────────────────────────────────────────────

void UPTBCheatLibrary::CheatUnlockAchievement(const UObject* WorldContextObject, FName AchievementId)
{
#if PTB_WITH_CHEATS
	if (UPTBAchievementSubsystem* Achievements = PTBCheatInternal::GetSubsystem<UPTBAchievementSubsystem>(WorldContextObject))
	{
		Achievements->UnlockAchievement(AchievementId);
	}
#endif
}

void UPTBCheatLibrary::CheatUnlockAllAchievements(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	if (UPTBAchievementSubsystem* Achievements = PTBCheatInternal::GetSubsystem<UPTBAchievementSubsystem>(WorldContextObject))
	{
		Achievements->UnlockAllAchievements();
	}
#endif
}

void UPTBCheatLibrary::CheatResetAchievements(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	if (UPTBAchievementSubsystem* Achievements = PTBCheatInternal::GetSubsystem<UPTBAchievementSubsystem>(WorldContextObject))
	{
		Achievements->ResetActiveProfileAchievements();
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] ResetAchievements"));
	}
#endif
}

// ── 기타 ─────────────────────────────────────────────────────────

void UPTBCheatLibrary::CheatPrintStatus(const UObject* WorldContextObject)
{
#if PTB_WITH_CHEATS
	UPTBProfileSubsystem* Profiles = nullptr;
	const FPTBProfileData* Profile = GetActiveProfileMutable(WorldContextObject, Profiles);
	if (!Profile)
	{
		return;
	}

	PTB_RECORD(LogPTBProfile, TEXT("[Cheat] ===== %s | 소지금 %d (누적 %d, 사용 %d) | 아이템 %d개 | 업적 %d개 ====="),
		*Profile->Nickname, Profile->GetCurrentMoney(), Profile->TotalEarnedMoney, Profile->SpentMoney,
		Profile->OwnedItemIds.Num(), Profile->UnlockedAchievements.Num());

	if (const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>())
	{
		for (const FName& MiniGameId : Settings->ProgressMiniGameIds)
		{
			EPTBGradeType StdGrade;
			const bool bHasStd = UPTBProgressLibrary::GetBestGradeInProfile(*Profile, MiniGameId, EPTBDifficulty::Standard, StdGrade);
			PTB_RECORD(LogPTBProfile, TEXT("[Cheat] %-3s | Easy 플레이 %d 클리어 %d | Standard 플레이 %d 클리어 %d 최고 %s | Insane %s"),
				*MiniGameId.ToString(),
				UPTBProgressLibrary::GetPlayCountInProfile(*Profile, MiniGameId, EPTBDifficulty::Easy),
				UPTBProgressLibrary::GetClearCountInProfile(*Profile, MiniGameId, EPTBDifficulty::Easy),
				UPTBProgressLibrary::GetPlayCountInProfile(*Profile, MiniGameId, EPTBDifficulty::Standard),
				UPTBProgressLibrary::GetClearCountInProfile(*Profile, MiniGameId, EPTBDifficulty::Standard),
				bHasStd ? *UEnum::GetValueAsString(StdGrade) : TEXT("-"),
				UPTBProgressLibrary::IsDifficultyUnlockedInProfile(*Profile, MiniGameId, EPTBDifficulty::Insane) ? TEXT("해금") : TEXT("잠금"));
		}
	}
#endif
}

bool UPTBCheatLibrary::IsCheatPanelKey(const FKey& Key)
{
#if PTB_WITH_CHEATS
	const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>();
	return Settings && Settings->CheatPanelKey.IsValid() && Key == Settings->CheatPanelKey;
#else
	return false;
#endif
}

void UPTBCheatLibrary::ToggleCheatPanel(APlayerController* PlayerController)
{
#if PTB_WITH_CHEATS
	using namespace PTBCheatInternal;
	if (!PlayerController)
	{
		return;
	}

	// 닫기
	if (UUserWidget* Existing = CheatPanel.Get())
	{
		if (Existing->IsInViewport())
		{
			Existing->RemoveFromParent();
			CheatPanel.Reset();
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = bPrevShowMouseCursor;
			return;
		}
	}

	// 열기
	const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>();
	UClass* PanelClass = Settings ? Settings->CheatPanelWidgetClass.LoadSynchronous() : nullptr;
	if (!PanelClass)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Cheat] 치트 패널 WBP 미지정 (Project Settings → PTB 진행·업적 설정 → Cheat Panel Widget Class). 콘솔에서 PTB.Cheat.Help 사용"));
		return;
	}

	UUserWidget* Panel = CreateWidget<UUserWidget>(PlayerController, PanelClass);
	if (!Panel)
	{
		return;
	}
	Panel->AddToViewport(1000);
	CheatPanel = Panel;

	bPrevShowMouseCursor = PlayerController->bShowMouseCursor;
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
#endif
}

// ── 콘솔 명령어 (Shipping 제외) ──────────────────────────────────

#if PTB_WITH_CHEATS
namespace PTBCheatConsole
{
	int32 IntArg(const TArray<FString>& Args, int32 Index, int32 Default = 0)
	{
		return Args.IsValidIndex(Index) ? FCString::Atoi(*Args[Index]) : Default;
	}

	FName NameArg(const TArray<FString>& Args, int32 Index)
	{
		return Args.IsValidIndex(Index) && !Args[Index].Equals(TEXT("All"), ESearchCase::IgnoreCase) ? FName(*Args[Index]) : NAME_None;
	}

	template <typename EnumType>
	EnumType EnumArg(const TArray<FString>& Args, int32 Index, EnumType Default)
	{
		if (!Args.IsValidIndex(Index))
		{
			return Default;
		}
		const UEnum* Enum = StaticEnum<EnumType>();
		const int64 Value = Enum ? Enum->GetValueByNameString(Args[Index]) : INDEX_NONE;
		return Value == INDEX_NONE ? Default : static_cast<EnumType>(Value);
	}

#define PTB_CHEAT_COMMAND(Name, Help, ...) \
	static FAutoConsoleCommandWithWorldAndArgs GCheat_##Name(TEXT("PTB.Cheat." #Name), TEXT(Help), \
		FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World) __VA_ARGS__));

	PTB_CHEAT_COMMAND(Help, "치트 명령어 목록", {
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.SetMoney <금액> | AddMoney <금액> | SetTotalEarned <금액>"));
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.GrantItem <ItemId> | GrantAllItems | ResetItems"));
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.UnlockInsane [MiniGameId|All] | ResetForcedUnlocks"));
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.AddRecord <MiniGameId> <Easy|Standard|Insane> <PerfectFullCombo|FullCombo|S|A|B|C|Clear|Fail> [횟수] | ResetRecords"));
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.UnlockAchievement <Id> | UnlockAllAchievements | ResetAchievements"));
		PTB_RECORD(LogPTBProfile, TEXT("[Cheat] PTB.Cheat.Status"));
	})
	PTB_CHEAT_COMMAND(SetMoney, "소지금 설정: PTB.Cheat.SetMoney 10000", { UPTBCheatLibrary::CheatSetMoney(World, IntArg(Args, 0)); })
	PTB_CHEAT_COMMAND(AddMoney, "소지금 더하기: PTB.Cheat.AddMoney 5000", { UPTBCheatLibrary::CheatAddMoney(World, IntArg(Args, 0)); })
	PTB_CHEAT_COMMAND(SetTotalEarned, "누적 수입 설정(엔딩 테스트): PTB.Cheat.SetTotalEarned 100000", { UPTBCheatLibrary::CheatSetTotalEarnedMoney(World, IntArg(Args, 0)); })
	PTB_CHEAT_COMMAND(GrantItem, "아이템 지급: PTB.Cheat.GrantItem Hair_Pink", { UPTBCheatLibrary::CheatGrantItem(World, NameArg(Args, 0)); })
	PTB_CHEAT_COMMAND(GrantAllItems, "모든 아이템 지급", { UPTBCheatLibrary::CheatGrantAllItems(World); })
	PTB_CHEAT_COMMAND(ResetItems, "보유·장착 아이템 초기화", { UPTBCheatLibrary::CheatResetItems(World); })
	PTB_CHEAT_COMMAND(UnlockInsane, "Insane 해금: PTB.Cheat.UnlockInsane BB (생략·All = 전체)", { UPTBCheatLibrary::CheatUnlockDifficulty(World, NameArg(Args, 0), EPTBDifficulty::Insane); })
	PTB_CHEAT_COMMAND(ResetForcedUnlocks, "치트로 해금한 난이도 되돌리기", { UPTBCheatLibrary::CheatResetForcedUnlocks(World); })
	PTB_CHEAT_COMMAND(AddRecord, "기록 추가: PTB.Cheat.AddRecord BB Standard S 1", {
		UPTBCheatLibrary::CheatAddRecord(World, NameArg(Args, 0),
			EnumArg<EPTBDifficulty>(Args, 1, EPTBDifficulty::Standard),
			EnumArg<EPTBGradeType>(Args, 2, EPTBGradeType::S),
			IntArg(Args, 3, 1));
	})
	PTB_CHEAT_COMMAND(ResetRecords, "플레이·클리어 횟수, 최고 등급 초기화", { UPTBCheatLibrary::CheatResetRecords(World); })
	PTB_CHEAT_COMMAND(UnlockAchievement, "업적 달성: PTB.Cheat.UnlockAchievement ACH_FirstClear", { UPTBCheatLibrary::CheatUnlockAchievement(World, NameArg(Args, 0)); })
	PTB_CHEAT_COMMAND(UnlockAllAchievements, "모든 업적 달성", { UPTBCheatLibrary::CheatUnlockAllAchievements(World); })
	PTB_CHEAT_COMMAND(ResetAchievements, "업적 초기화", { UPTBCheatLibrary::CheatResetAchievements(World); })
	PTB_CHEAT_COMMAND(Status, "활성 프로필 상태 출력", { UPTBCheatLibrary::CheatPrintStatus(World); })

#undef PTB_CHEAT_COMMAND
}
#endif
