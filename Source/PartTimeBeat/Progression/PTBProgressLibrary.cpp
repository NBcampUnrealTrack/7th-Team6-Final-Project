#include "Progression/PTBProgressLibrary.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Profile/PTBProfileSubsystem.h"
#include "Progression/PTBProgressSettings.h"

#define LOCTEXT_NAMESPACE "PTBProgress"

namespace PTBProgressInternal
{
	FText GradeText(EPTBGradeType Grade)
	{
		switch (Grade)
		{
		case EPTBGradeType::PerfectFullCombo: return LOCTEXT("Grade_PFC", "올 퍼펙트");
		case EPTBGradeType::FullCombo:        return LOCTEXT("Grade_FC", "풀콤보");
		case EPTBGradeType::S:                return LOCTEXT("Grade_S", "S");
		case EPTBGradeType::A:                return LOCTEXT("Grade_A", "A");
		case EPTBGradeType::B:                return LOCTEXT("Grade_B", "B");
		case EPTBGradeType::C:                return LOCTEXT("Grade_C", "C");
		case EPTBGradeType::Clear:            return LOCTEXT("Grade_Clear", "클리어");
		default:                              return LOCTEXT("Grade_Fail", "실패");
		}
	}

	/** Insane 해금 판정에 쓰는 하위 난이도 (Easy 이상 = Easy + Standard) */
	const EPTBDifficulty CountedDifficulties[] = { EPTBDifficulty::Easy, EPTBDifficulty::Standard, EPTBDifficulty::Insane };
}

FName UPTBProgressLibrary::MakeDifficultyKey(FName MiniGameId, EPTBDifficulty Difficulty)
{
	// UPTBProfileSubsystem::MakeDifficultyScoreKey와 같은 형식
	const UEnum* DiffEnum = StaticEnum<EPTBDifficulty>();
	const FString DiffName = DiffEnum
		? DiffEnum->GetNameStringByValue(static_cast<int64>(Difficulty))
		: FString::FromInt(static_cast<int32>(Difficulty));
	return FName(*FString::Printf(TEXT("%s_%s"), *MiniGameId.ToString(), *DiffName));
}

bool UPTBProgressLibrary::ParseDifficultyKey(FName Key, FName& OutMiniGameId, EPTBDifficulty& OutDifficulty)
{
	const FString KeyString = Key.ToString();
	int32 SplitIndex = INDEX_NONE;
	if (!KeyString.FindLastChar(TEXT('_'), SplitIndex) || SplitIndex <= 0)
	{
		return false;
	}

	const UEnum* DiffEnum = StaticEnum<EPTBDifficulty>();
	const int64 Value = DiffEnum ? DiffEnum->GetValueByNameString(KeyString.Mid(SplitIndex + 1)) : INDEX_NONE;
	if (Value == INDEX_NONE)
	{
		return false;
	}

	OutMiniGameId = FName(*KeyString.Left(SplitIndex));
	OutDifficulty = static_cast<EPTBDifficulty>(Value);
	return true;
}

const FPTBProfileData* UPTBProgressLibrary::GetActiveProfile(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	const UGameInstance* GI = World ? World->GetGameInstance() : nullptr;
	const UPTBProfileSubsystem* Profiles = GI ? GI->GetSubsystem<UPTBProfileSubsystem>() : nullptr;
	return Profiles ? Profiles->FindActiveProfileConst() : nullptr;
}

int32 UPTBProgressLibrary::GetPlayCountInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty)
{
	const int32* Found = Profile.PlayCountByDifficulty.Find(MakeDifficultyKey(MiniGameId, Difficulty));
	return Found ? *Found : 0;
}

int32 UPTBProgressLibrary::GetClearCountInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty)
{
	const int32* Found = Profile.ClearCountByDifficulty.Find(MakeDifficultyKey(MiniGameId, Difficulty));
	return Found ? *Found : 0;
}

bool UPTBProgressLibrary::GetBestGradeInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty, EPTBGradeType& OutGrade)
{
	if (const EPTBGradeType* Found = Profile.BestGradeByDifficulty.Find(MakeDifficultyKey(MiniGameId, Difficulty)))
	{
		OutGrade = *Found;
		return true;
	}
	OutGrade = EPTBGradeType::Fail;
	return false;
}

bool UPTBProgressLibrary::IsGradeAtLeast(EPTBGradeType Grade, EPTBGradeType Threshold)
{
	// 값이 작을수록 높은 등급
	return static_cast<uint8>(Grade) <= static_cast<uint8>(Threshold);
}

bool UPTBProgressLibrary::IsDifficultyUnlockedInProfile(const FPTBProfileData& Profile, FName MiniGameId, EPTBDifficulty Difficulty)
{
	if (Difficulty != EPTBDifficulty::Insane)
	{
		return true;
	}

	const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>();
	if (!Settings || !Settings->bLockInsaneByDefault)
	{
		return true;
	}

	if (Profile.ForceUnlockedDifficulties.Contains(MakeDifficultyKey(MiniGameId, Difficulty)))
	{
		return true;
	}

	int32 Plays = 0;
	int32 Clears = 0;
	for (const EPTBDifficulty Counted : PTBProgressInternal::CountedDifficulties)
	{
		Plays  += GetPlayCountInProfile(Profile, MiniGameId, Counted);
		Clears += GetClearCountInProfile(Profile, MiniGameId, Counted);
	}

	if (Settings->InsaneUnlockPlayCount > 0 && Plays >= Settings->InsaneUnlockPlayCount)
	{
		return true;
	}
	if (Settings->InsaneUnlockClearCount > 0 && Clears >= Settings->InsaneUnlockClearCount)
	{
		return true;
	}

	EPTBGradeType StandardGrade;
	return GetBestGradeInProfile(Profile, MiniGameId, EPTBDifficulty::Standard, StandardGrade)
		&& IsGradeAtLeast(StandardGrade, Settings->InsaneUnlockStandardGrade);
}

bool UPTBProgressLibrary::IsDifficultyUnlocked(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty)
{
	if (Difficulty != EPTBDifficulty::Insane)
	{
		return true;
	}
	const FPTBProfileData* Profile = GetActiveProfile(WorldContextObject);
	if (!Profile)
	{
		// 프로필 없이 레벨 직접 실행 등: 막지 않음
		return true;
	}
	return IsDifficultyUnlockedInProfile(*Profile, MiniGameId, Difficulty);
}

FText UPTBProgressLibrary::GetDifficultyUnlockConditionText(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty)
{
	if (IsDifficultyUnlocked(WorldContextObject, MiniGameId, Difficulty))
	{
		return FText::GetEmpty();
	}

	const UPTBProgressSettings* Settings = GetDefault<UPTBProgressSettings>();
	const FPTBProfileData* Profile = GetActiveProfile(WorldContextObject);
	if (!Settings || !Profile)
	{
		return FText::GetEmpty();
	}

	int32 Plays = 0;
	int32 Clears = 0;
	for (const EPTBDifficulty Counted : PTBProgressInternal::CountedDifficulties)
	{
		Plays  += GetPlayCountInProfile(*Profile, MiniGameId, Counted);
		Clears += GetClearCountInProfile(*Profile, MiniGameId, Counted);
	}

	TArray<FText> Lines;
	Lines.Add(LOCTEXT("InsaneUnlock_Title", "발광모드 해금 조건 (하나만 달성하면 해금)"));
	if (Settings->InsaneUnlockPlayCount > 0)
	{
		Lines.Add(FText::Format(LOCTEXT("InsaneUnlock_Play", "· 플레이 {0}/{1}회"),
			FText::AsNumber(FMath::Min(Plays, Settings->InsaneUnlockPlayCount)), FText::AsNumber(Settings->InsaneUnlockPlayCount)));
	}
	if (Settings->InsaneUnlockClearCount > 0)
	{
		Lines.Add(FText::Format(LOCTEXT("InsaneUnlock_Clear", "· 클리어 {0}/{1}회"),
			FText::AsNumber(FMath::Min(Clears, Settings->InsaneUnlockClearCount)), FText::AsNumber(Settings->InsaneUnlockClearCount)));
	}

	EPTBGradeType StandardGrade;
	const bool bHasStandard = GetBestGradeInProfile(*Profile, MiniGameId, EPTBDifficulty::Standard, StandardGrade);
	Lines.Add(FText::Format(LOCTEXT("InsaneUnlock_Grade", "· 통상모드 {0} 이상 (최고: {1})"),
		PTBProgressInternal::GradeText(Settings->InsaneUnlockStandardGrade),
		bHasStandard ? PTBProgressInternal::GradeText(StandardGrade) : LOCTEXT("Grade_None", "기록 없음")));

	return FText::Join(FText::FromString(TEXT("\n")), Lines);
}

int32 UPTBProgressLibrary::GetPlayCount(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty)
{
	const FPTBProfileData* Profile = GetActiveProfile(WorldContextObject);
	return Profile ? GetPlayCountInProfile(*Profile, MiniGameId, Difficulty) : 0;
}

int32 UPTBProgressLibrary::GetClearCount(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty)
{
	const FPTBProfileData* Profile = GetActiveProfile(WorldContextObject);
	return Profile ? GetClearCountInProfile(*Profile, MiniGameId, Difficulty) : 0;
}

EPTBGradeType UPTBProgressLibrary::GetBestGrade(const UObject* WorldContextObject, FName MiniGameId, EPTBDifficulty Difficulty, bool& bOutHasRecord)
{
	EPTBGradeType Grade = EPTBGradeType::Fail;
	const FPTBProfileData* Profile = GetActiveProfile(WorldContextObject);
	bOutHasRecord = Profile && GetBestGradeInProfile(*Profile, MiniGameId, Difficulty, Grade);
	return Grade;
}

#undef LOCTEXT_NAMESPACE
