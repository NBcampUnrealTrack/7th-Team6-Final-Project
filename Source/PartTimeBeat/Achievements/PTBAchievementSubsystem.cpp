#include "Achievements/PTBAchievementSubsystem.h"

#include "Achievements/PTBAchievementBackend.h"
#include "Debug/PTBTeamLog.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Progression/PTBProgressLibrary.h"
#include "Progression/PTBProgressSettings.h"

namespace PTBAchievementInternal
{
	bool MatchesFilter(const FPTBAchievementRow& Row, FName MiniGameId, EPTBDifficulty Difficulty)
	{
		if (!Row.MiniGameId.IsNone() && Row.MiniGameId != MiniGameId)
		{
			return false;
		}
		switch (Row.DifficultyMatch)
		{
		case EPTBAchievementDifficultyMatch::Exactly: return Difficulty == Row.Difficulty;
		case EPTBAchievementDifficultyMatch::AtLeast: return static_cast<uint8>(Difficulty) >= static_cast<uint8>(Row.Difficulty);
		default:                                      return true;
		}
	}
}

void UPTBAchievementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<UPTBProfileSubsystem>();

	LoadTable();
	CreateBackend();

	if (UPTBProfileSubsystem* Profiles = GetProfiles())
	{
		ProfileChangedHandle = Profiles->OnActiveProfileChanged.AddUObject(this, &UPTBAchievementSubsystem::HandleActiveProfileChanged);
	}
}

void UPTBAchievementSubsystem::Deinitialize()
{
	if (UPTBProfileSubsystem* Profiles = GetProfiles())
	{
		Profiles->OnActiveProfileChanged.Remove(ProfileChangedHandle);
	}
	Super::Deinitialize();
}

UPTBProfileSubsystem* UPTBAchievementSubsystem::GetProfiles() const
{
	const UGameInstance* GI = GetGameInstance();
	return GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
}

void UPTBAchievementSubsystem::LoadTable()
{
	AchievementsById.Reset();
	SortedIds.Reset();

	const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>();
	AchievementTable = Settings ? Settings->AchievementTable.LoadSynchronous() : nullptr;
	if (!AchievementTable)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Achievement] 업적 DataTable 없음 (Project Settings → PTB 진행·업적 설정 → Achievement Table)"));
		return;
	}

	if (AchievementTable->GetRowStruct() != FPTBAchievementRow::StaticStruct())
	{
		PTB_ERROR(LogPTBProfile, TEXT("[Achievement] %s 의 Row Structure가 PTBAchievementRow가 아님"), *GetNameSafe(AchievementTable));
		AchievementTable = nullptr;
		return;
	}

	AchievementTable->ForeachRow<FPTBAchievementRow>(TEXT("PTBAchievement"), [this](const FName& RowName, const FPTBAchievementRow& Row)
	{
		AchievementsById.Add(RowName, Row);
		SortedIds.Add(RowName);
	});

	SortedIds.Sort([this](const FName& A, const FName& B)
	{
		const FPTBAchievementRow& RowA = AchievementsById[A];
		const FPTBAchievementRow& RowB = AchievementsById[B];
		return RowA.SortOrder != RowB.SortOrder ? RowA.SortOrder < RowB.SortOrder : A.LexicalLess(B);
	});

	PTB_RECORD(LogPTBProfile, TEXT("[Achievement] 업적 %d개 로드 (%s)"), AchievementsById.Num(), *GetNameSafe(AchievementTable));
}

void UPTBAchievementSubsystem::CreateBackend()
{
	TSubclassOf<UPTBAchievementBackend> BackendClass = UPTBLogAchievementBackend::StaticClass();
	if (const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>())
	{
		if (UClass* Custom = Settings->AchievementBackendClass.LoadSynchronous())
		{
			BackendClass = Custom;
		}
	}

	Backend = NewObject<UPTBAchievementBackend>(this, BackendClass);
	if (Backend)
	{
		Backend->InitializeBackend();
	}
}

void UPTBAchievementSubsystem::HandleActiveProfileChanged(const FPTBProfileData& NewProfile)
{
	// 업적이 나중에 추가됐거나 조건이 바뀐 경우를 위해 다시 판정 + 플랫폼 동기화
	EvaluateAchievements();
	if (const UPTBProfileSubsystem* Profiles = GetProfiles())
	{
		if (const FPTBProfileData* Active = Profiles->FindActiveProfileConst())
		{
			SyncUnlockedToPlatform(*Active);
		}
	}
}

int32 UPTBAchievementSubsystem::ComputeProgress(const FPTBProfileData& Profile, FName AchievementId, const FPTBAchievementRow& Row) const
{
	using namespace PTBAchievementInternal;

	switch (Row.Condition)
	{
	case EPTBAchievementCondition::ClearMiniGames:
	{
		TSet<FName> MiniGames;
		for (const TPair<FName, int32>& Pair : Profile.ClearCountByDifficulty)
		{
			FName MiniGameId;
			EPTBDifficulty Difficulty;
			if (Pair.Value > 0 && UPTBProgressLibrary::ParseDifficultyKey(Pair.Key, MiniGameId, Difficulty) && MatchesFilter(Row, MiniGameId, Difficulty))
			{
				MiniGames.Add(MiniGameId);
			}
		}
		return MiniGames.Num();
	}
	case EPTBAchievementCondition::GradeMiniGames:
	{
		TSet<FName> MiniGames;
		for (const TPair<FName, EPTBGradeType>& Pair : Profile.BestGradeByDifficulty)
		{
			FName MiniGameId;
			EPTBDifficulty Difficulty;
			if (UPTBProgressLibrary::IsGradeAtLeast(Pair.Value, Row.MinGrade)
				&& UPTBProgressLibrary::ParseDifficultyKey(Pair.Key, MiniGameId, Difficulty) && MatchesFilter(Row, MiniGameId, Difficulty))
			{
				MiniGames.Add(MiniGameId);
			}
		}
		return MiniGames.Num();
	}
	case EPTBAchievementCondition::PlayCount:
	{
		int32 Total = 0;
		for (const TPair<FName, int32>& Pair : Profile.PlayCountByDifficulty)
		{
			FName MiniGameId;
			EPTBDifficulty Difficulty;
			if (UPTBProgressLibrary::ParseDifficultyKey(Pair.Key, MiniGameId, Difficulty) && MatchesFilter(Row, MiniGameId, Difficulty))
			{
				Total += Pair.Value;
			}
		}
		return Total;
	}
	case EPTBAchievementCondition::TotalEarnedMoney:
		return Profile.TotalEarnedMoney;
	default:
		return Profile.UnlockedAchievements.Contains(AchievementId) ? 1 : 0;
	}
}

bool UPTBAchievementSubsystem::UnlockInProfile(FPTBProfileData& Profile, FName AchievementId, const FPTBAchievementRow& Row)
{
	if (Profile.UnlockedAchievements.Contains(AchievementId))
	{
		return false;
	}

	Profile.UnlockedAchievements.Add(AchievementId, FDateTime::Now());
	PTB_RECORD(LogPTBProfile, TEXT("[Achievement] 달성: %s (%s)"), *AchievementId.ToString(), *Row.DisplayName.ToString());

	if (Backend)
	{
		Backend->ReportUnlocked(AchievementId, Row);
	}
	return true;
}

TArray<FName> UPTBAchievementSubsystem::EvaluateAchievements()
{
	TArray<FName> NewlyUnlocked;

	UPTBProfileSubsystem* Profiles = GetProfiles();
	FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileMutable() : nullptr;
	if (!Profile)
	{
		return NewlyUnlocked;
	}

	for (const FName& Id : SortedIds)
	{
		const FPTBAchievementRow& Row = AchievementsById[Id];
		if (Row.bRetired || Row.Condition == EPTBAchievementCondition::Manual || Profile->UnlockedAchievements.Contains(Id))
		{
			continue;
		}

		const int32 Progress = ComputeProgress(*Profile, Id, Row);
		if (Progress >= FMath::Max(1, Row.RequiredCount))
		{
			if (UnlockInProfile(*Profile, Id, Row))
			{
				NewlyUnlocked.Add(Id);
			}
		}
		else if (Backend && Progress > 0)
		{
			Backend->ReportProgress(Id, Row, Progress, FMath::Max(1, Row.RequiredCount));
		}
	}

	if (NewlyUnlocked.Num() > 0)
	{
		Profiles->RequestSave();
		for (const FName& Id : NewlyUnlocked)
		{
			OnAchievementUnlocked.Broadcast(Id, MakeView(Profile, Id, AchievementsById[Id]));
		}
	}

	return NewlyUnlocked;
}

bool UPTBAchievementSubsystem::UnlockAchievement(FName AchievementId)
{
	const FPTBAchievementRow* Row = AchievementsById.Find(AchievementId);
	UPTBProfileSubsystem* Profiles = GetProfiles();
	FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileMutable() : nullptr;
	if (!Row || !Profile)
	{
		PTB_WARNING(LogPTBProfile, TEXT("[Achievement] UnlockAchievement 실패: %s (없는 ID 또는 활성 프로필 없음)"), *AchievementId.ToString());
		return false;
	}

	if (!UnlockInProfile(*Profile, AchievementId, *Row))
	{
		return false;
	}

	Profiles->RequestSave();
	OnAchievementUnlocked.Broadcast(AchievementId, MakeView(Profile, AchievementId, *Row));
	return true;
}

void UPTBAchievementSubsystem::SyncUnlockedToPlatform(const FPTBProfileData& Profile)
{
	if (!Backend)
	{
		return;
	}
	for (const TPair<FName, FDateTime>& Pair : Profile.UnlockedAchievements)
	{
		if (const FPTBAchievementRow* Row = AchievementsById.Find(Pair.Key))
		{
			Backend->ReportUnlocked(Pair.Key, *Row);
		}
	}
}

bool UPTBAchievementSubsystem::IsAchievementUnlocked(FName AchievementId) const
{
	const UPTBProfileSubsystem* Profiles = GetProfiles();
	const FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileConst() : nullptr;
	return Profile && Profile->UnlockedAchievements.Contains(AchievementId);
}

FPTBAchievementView UPTBAchievementSubsystem::MakeView(const FPTBProfileData* Profile, FName AchievementId, const FPTBAchievementRow& Row) const
{
	FPTBAchievementView View;
	View.AchievementId = AchievementId;
	View.Data = Row;
	View.RequiredCount = FMath::Max(1, Row.RequiredCount);

	if (Profile)
	{
		if (const FDateTime* UnlockedAt = Profile->UnlockedAchievements.Find(AchievementId))
		{
			View.bUnlocked = true;
			View.UnlockedAt = *UnlockedAt;
		}
		View.CurrentCount = View.bUnlocked
			? View.RequiredCount
			: FMath::Min(ComputeProgress(*Profile, AchievementId, Row), View.RequiredCount);
	}

	View.bConcealed = Row.bHidden && !View.bUnlocked;
	return View;
}

TArray<FPTBAchievementView> UPTBAchievementSubsystem::GetAchievementViews() const
{
	TArray<FPTBAchievementView> Views;
	const UPTBProfileSubsystem* Profiles = GetProfiles();
	const FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileConst() : nullptr;

	for (const FName& Id : SortedIds)
	{
		const FPTBAchievementRow& Row = AchievementsById[Id];
		if (!Row.bRetired)
		{
			Views.Add(MakeView(Profile, Id, Row));
		}
	}
	return Views;
}

bool UPTBAchievementSubsystem::GetAchievementView(FName AchievementId, FPTBAchievementView& OutView) const
{
	const FPTBAchievementRow* Row = AchievementsById.Find(AchievementId);
	if (!Row)
	{
		return false;
	}
	const UPTBProfileSubsystem* Profiles = GetProfiles();
	OutView = MakeView(Profiles ? Profiles->FindActiveProfileConst() : nullptr, AchievementId, *Row);
	return true;
}

void UPTBAchievementSubsystem::GetAchievementCounts(int32& OutUnlocked, int32& OutTotal) const
{
	OutUnlocked = 0;
	OutTotal = 0;
	const UPTBProfileSubsystem* Profiles = GetProfiles();
	const FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileConst() : nullptr;

	for (const FName& Id : SortedIds)
	{
		if (AchievementsById[Id].bRetired)
		{
			continue;
		}
		++OutTotal;
		if (Profile && Profile->UnlockedAchievements.Contains(Id))
		{
			++OutUnlocked;
		}
	}
}

void UPTBAchievementSubsystem::ResetActiveProfileAchievements()
{
	UPTBProfileSubsystem* Profiles = GetProfiles();
	if (FPTBProfileData* Profile = Profiles ? Profiles->FindActiveProfileMutable() : nullptr)
	{
		Profile->UnlockedAchievements.Reset();
		Profiles->RequestSave();
	}
	if (Backend)
	{
		Backend->ResetAll();
	}
}

void UPTBAchievementSubsystem::UnlockAllAchievements()
{
	for (const FName& Id : SortedIds)
	{
		if (!AchievementsById[Id].bRetired)
		{
			UnlockAchievement(Id);
		}
	}
}
