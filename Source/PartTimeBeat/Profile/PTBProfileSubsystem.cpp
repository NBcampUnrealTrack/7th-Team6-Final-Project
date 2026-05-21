#include "PTBProfileSubsystem.h"
#include "Core/PTBSaveGame.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"



void UPTBProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PTB_RECORD(LogPTBProfile, TEXT("Initialize"));

    LoadProfilesFromSave();
    ClearActiveProfile();
}

void UPTBProfileSubsystem::Deinitialize()
{
    PTB_RECORD(LogPTBProfile, TEXT("Deinitialize"));

    RequestSave();

    Super::Deinitialize();
}


FGuid UPTBProfileSubsystem::CreateProfile(
    const FString& Nickname,
    EPTBGender Gender,
    FDateTime Birthday)
{
    if (AllProfiles.Num() >= MaxProfiles)
    {
        PTB_ERROR(LogPTBProfile, TEXT("CreateProfile failed: slot full (%d/%d)"), AllProfiles.Num(), MaxProfiles);

        return FGuid();
    }

    const EPTBNicknameValidationResult ValidationResult = ValidateNickname(Nickname);
    if (ValidationResult != EPTBNicknameValidationResult::Valid)
    {
        PTB_ERROR(LogPTBProfile, TEXT("CreateProfile failed: nickname invalid"));

        return FGuid();
    }

    FPTBProfileData NewProfile;
    NewProfile.Nickname = NormalizeNickname(Nickname);
    NewProfile.Gender = Gender;
    NewProfile.Birthday = Birthday;

    const FGuid NewId = NewProfile.ProfileId;
    AllProfiles.Add(NewProfile);

    PTB_RECORD(LogPTBProfile, TEXT("CreateProfile success"));

    OnProfileListChanged.Broadcast();
    RequestSave();

    return NewId;
}

FPTBProfileData UPTBProfileSubsystem::GetProfile(const FGuid& ProfileId, bool& bOutFound) const
{
    for (const FPTBProfileData& Profile : AllProfiles)
    {
        if (Profile.ProfileId == ProfileId)
        {
            bOutFound = true;
            return Profile;
        }
    }

    bOutFound = false;
    return FPTBProfileData::MakeInvalid();
}

TArray<FPTBProfileData> UPTBProfileSubsystem::GetAllProfiles() const
{
    return AllProfiles;
}

int32 UPTBProfileSubsystem::GetProfileCount() const
{
    return AllProfiles.Num();
}

bool UPTBProfileSubsystem::DeleteProfile(const FGuid& ProfileId)
{
    const int32 RemovedCount = AllProfiles.RemoveAll(
        [&ProfileId](const FPTBProfileData& Profile)
        {
            return Profile.ProfileId == ProfileId;
        });

    if (RemovedCount == 0)
    {
        PTB_ERROR(LogPTBProfile, TEXT("DeleteProfile failed: not found"));

        return false;
    }

    if (ActiveProfileId == ProfileId)
    {
        ActiveProfileId = FGuid();
    }

    PTB_RECORD(LogPTBProfile, TEXT("DeleteProfile success"));

    OnProfileListChanged.Broadcast();
    RequestSave();

    return true;
}

bool UPTBProfileSubsystem::SetActiveProfile(const FGuid& ProfileId)
{
    bool bFound = false;
    const FPTBProfileData Profile = GetProfile(ProfileId, bFound);

    if (!bFound)
    {
        PTB_ERROR(LogPTBProfile, TEXT("SetActiveProfile failed: not found"));

        return false;
    }

    ActiveProfileId = ProfileId;
    PTB_RECORD(LogPTBProfile, TEXT("SetActiveProfile"));

    if (FPTBProfileData* Mutable = FindActiveProfileMutable())
    {
        Mutable->LastPlayedAt = FDateTime::Now();
    }

    RequestSave();

    bool bUpdatedFound = false;
    const FPTBProfileData UpdatedProfile = GetProfile(ActiveProfileId, bUpdatedFound);
    OnActiveProfileChanged.Broadcast(bUpdatedFound ? UpdatedProfile : Profile);

    return true;
}

FPTBProfileData UPTBProfileSubsystem::GetActiveProfile(bool& bOutHasActive) const
{
    if (!ActiveProfileId.IsValid())
    {
        bOutHasActive = false;
        return FPTBProfileData::MakeInvalid();
    }

    return GetProfile(ActiveProfileId, bOutHasActive);
}

bool UPTBProfileSubsystem::HasActiveProfile() const
{
    return ActiveProfileId.IsValid();
}

EPTBNicknameValidationResult UPTBProfileSubsystem::ValidateNickname(const FString& Nickname) const
{
    const FString Normalized = NormalizeNickname(Nickname);

    if (Normalized.IsEmpty())
    {
        return EPTBNicknameValidationResult::Empty;
    }
    if (Normalized.Len() < NicknameMinLength)
    {
        return EPTBNicknameValidationResult::TooShort;
    }
    if (Normalized.Len() > NicknameMaxLength)
    {
        return EPTBNicknameValidationResult::TooLong;
    }

    for (const FPTBProfileData& Profile : AllProfiles)
    {
        if (Profile.Nickname.Equals(Normalized, ESearchCase::IgnoreCase))
        {
            return EPTBNicknameValidationResult::AlreadyTaken;
        }
    }

    // 금칙어 필터링은 추후 추가
    // if (IsForbiddenWord(Normalized)) return EPTBNicknameValidationResult::ContainsForbiddenWord;

    return EPTBNicknameValidationResult::Valid;
}

bool UPTBProfileSubsystem::IsFirstTimeProfile(const FGuid& ProfileId, FName MiniGameId) const
{
    bool bFound = false;
    const FPTBProfileData Profile = GetProfile(ProfileId, bFound);

    if (!bFound)
    {
        return true;
    }

    return !Profile.CompletedTutorialIds.Contains(MiniGameId);
}

void UPTBProfileSubsystem::ApplyRoundResultToActive(const FPTBProfileProgressUpdate& Update)
{
    FPTBProfileData* Active = FindActiveProfileMutable();
    if (!Active)
    {
        PTB_ERROR(LogPTBProfile, TEXT("ApplyRoundResultToActive failed: no active profile"));

        return;
    }

    int32& BestScore = Active->BestScoresByMiniGame.FindOrAdd(Update.MiniGameId, 0);
    if (Update.Score > BestScore)
    {
        BestScore = Update.Score;
    }

    int32& BestStars = Active->EarnedStarsByMiniGame.FindOrAdd(Update.MiniGameId, 0);
    if (Update.EarnedStars > BestStars)
    {
        BestStars = Update.EarnedStars;
    }

    Active->TotalEarnedMoney += Update.EarnedMoney;
    RequestSave();
    
    PTB_RECORD(LogPTBProfile, TEXT("ApplyRoundResult: game=%s score=%d stars=%d money=%d (total=%d)"), 
        *Update.MiniGameId.ToString(), 
        Update.Score, Update.EarnedStars,
        Update.EarnedMoney, Active->TotalEarnedMoney);
    
}

void UPTBProfileSubsystem::MarkTutorialComplete(FName MiniGameId)
{
    FPTBProfileData* Active = FindActiveProfileMutable();
    if (!Active)
    {
        PTB_ERROR(LogPTBProfile, TEXT("MarkTutorialComplete failed: no active profile"));

        return;
    }

    Active->CompletedTutorialIds.Add(MiniGameId);

    RequestSave();

    PTB_RECORD(LogPTBProfile, TEXT("MarkTutorialComplete: game=%s"), *MiniGameId.ToString());
}

void UPTBProfileSubsystem::MarkStoryViewed(FName StoryId)
{
    FPTBProfileData* Active = FindActiveProfileMutable();
    if (!Active)
    {
        return;
    }

    Active->CompletedStoryIds.Add(StoryId);

    RequestSave();
}

//void UPTBProfileSubsystem::UnlockCostume(FName CostumeId)
//{
//    FPTBProfileData* Active = FindActiveProfileMutable();
//    if (!Active)
//    {
//        return;
//    }
//
//    //Active->UnlockedCostumes.Add(CostumeId);
// 
//    //RequestSave();
//}

int32 UPTBProfileSubsystem::GetBestScore(FName MiniGameId) const
{
    bool bFound = false;
    const FPTBProfileData Active = GetActiveProfile(bFound);
    if (!bFound)
    {
        return 0;
    }

    if (const int32* Found = Active.BestScoresByMiniGame.Find(MiniGameId))
    {
        return *Found;
    }
    return 0;
}

int32 UPTBProfileSubsystem::GetEarnedStars(FName MiniGameId) const
{
    bool bFound = false;
    const FPTBProfileData Active = GetActiveProfile(bFound);
    if (!bFound)
    {
        return 0;
    }

    if (const int32* Found = Active.EarnedStarsByMiniGame.Find(MiniGameId))
    {
        return *Found;
    }
    return 0;
}

int32 UPTBProfileSubsystem::GetTotalEarnedMoney() const
{
    bool bFound = false;
    const FPTBProfileData Active = GetActiveProfile(bFound);
    return bFound ? Active.TotalEarnedMoney : 0;
}


void UPTBProfileSubsystem::RequestSave()
{
    PTB_RECORD(LogPTBProfile, TEXT("RequestSave profiles=%d"), AllProfiles.Num());

    UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
    if (!SaveGame)
    {
        PTB_ERROR(LogPTBProfile, TEXT("RequestSave: SaveGame null"));

        return;
    }

    SaveGame->Profiles = AllProfiles;
    //const bool bSuccess = SaveGame->SaveToDisk();

    //PTB_RECORD(LogPTBProfile, TEXT("RequestSave %s"), bSuccess ? TEXT("Success") : TEXT("FAIL"));
}

void UPTBProfileSubsystem::LoadProfilesFromSave()
{
    AllProfiles.Empty();

    UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
    if (!SaveGame)
    {
        PTB_ERROR(LogPTBProfile, TEXT("LoadProfilesFromSave: SaveGame null"));

        return;
    }
    AllProfiles = SaveGame->Profiles;

    PTB_RECORD(LogPTBProfile, TEXT("LoadProfilesFromSave - loaded %d profiles"), AllProfiles.Num());
}

void UPTBProfileSubsystem::ClearActiveProfile()
{
    ActiveProfileId = FGuid();
}

UPTBSaveGame* UPTBProfileSubsystem::GetOrCreateSaveGame() const
{
    // SaveGame - LoadOrCreate.
    //return UPTBSaveGame::LoadOrCreate();
    //임시 nullptr처리
    return nullptr;
}

FString UPTBProfileSubsystem::NormalizeNickname(const FString& InNickname) const
{
    FString Result = InNickname;
    Result.TrimStartAndEndInline();
    return Result;
}

FPTBProfileData* UPTBProfileSubsystem::FindActiveProfileMutable()
{
    return FindProfileMutable(ActiveProfileId);
}

FPTBProfileData* UPTBProfileSubsystem::FindProfileMutable(const FGuid& ProfileId)
{
    if (!ProfileId.IsValid())
    {
        return nullptr;
    }

    for (FPTBProfileData& Profile : AllProfiles)
    {
        if (Profile.ProfileId == ProfileId)
        {
            return &Profile;
        }
    }
    return nullptr;
}
