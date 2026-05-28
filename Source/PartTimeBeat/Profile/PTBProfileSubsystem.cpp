#include "PTBProfileSubsystem.h"
#include "Core/PTBSaveGame.h"
#include "Core/PTBGameInstance.h"
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
    int32 SlotIndex,
    FDateTime Birthday)
{
    if (AllProfiles.Num() >= MaxProfiles)
    {
        PTB_ERROR(LogPTBProfile, TEXT("CreateProfile failed: slot full (%d/%d)"), AllProfiles.Num(), MaxProfiles);

        return FGuid();
    }

    // SlotIndex 중복 검사 — 같은 슬롯에 프로필이 이미 있으면 거부
    bool bSlotTaken = false;
    GetProfileBySlot(SlotIndex, bSlotTaken);
    if (bSlotTaken)
    {
        PTB_ERROR(LogPTBProfile,
            TEXT("CreateProfile failed: SlotIndex %d is already occupied. 덮어쓰려면 먼저 DeleteProfile을 호출하세요."),
            SlotIndex);
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
    NewProfile.SlotIndex = SlotIndex;

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

FPTBProfileData UPTBProfileSubsystem::GetProfileBySlot(int32 SlotIndex, bool& bOutFound) const
{
    const FPTBProfileData* FirstFound = nullptr;
    int32 MatchCount = 0;

    for (const FPTBProfileData& Profile : AllProfiles)
    {
        if (Profile.SlotIndex == SlotIndex)
        {
            if (!FirstFound)
            {
                FirstFound = &Profile;
            }
            ++MatchCount;
        }
    }

    // 중복 슬롯 감지 — CreateProfile의 SlotIndex 검사가 정상 동작했다면 발생하지 않아야 함
    if (MatchCount > 1)
    {
        PTB_WARNING(LogPTBProfile,
            TEXT("GetProfileBySlot: SlotIndex %d에 중복 프로필 %d개 감지 — 첫 번째 반환. 데이터 정합성을 확인하세요."),
            SlotIndex, MatchCount);
    }

    if (FirstFound)
    {
        bOutFound = true;
        return *FirstFound;
    }

    bOutFound = false;
    return FPTBProfileData::MakeInvalid();
}

bool UPTBProfileSubsystem::DeleteProfile(const FGuid& ProfileId)
{
    // 삭제 전: 대상 프로필이 실제로 어떤 슬롯/닉네임인지 먼저 확인
    bool bPreCheck = false;
    const FPTBProfileData TargetProfile = GetProfile(ProfileId, bPreCheck);
    if (!bPreCheck)
    {
        PTB_ERROR(LogPTBProfile,
            TEXT("DeleteProfile failed: ProfileId=[%s] not found in AllProfiles"),
            *ProfileId.ToString());
        return false;
    }

    PTB_RECORD(LogPTBProfile,
        TEXT("DeleteProfile: 삭제 대상 — Nickname=%s SlotIndex=%d Id=%s"),
        *TargetProfile.Nickname, TargetProfile.SlotIndex, *ProfileId.ToString());

    const int32 RemovedCount = AllProfiles.RemoveAll(
        [&ProfileId](const FPTBProfileData& Profile)
        {
            return Profile.ProfileId == ProfileId;
        });

    // 위에서 bPreCheck==true였는데 RemovedCount==0이면 내부 정합성 오류
    if (RemovedCount == 0)
    {
        PTB_ERROR(LogPTBProfile,
            TEXT("DeleteProfile: 내부 오류 — PreCheck 통과 후 RemoveAll 0개 제거. 데이터 정합성을 확인하세요."));
        return false;
    }

    if (ActiveProfileId == ProfileId)
    {
        ActiveProfileId = FGuid();
    }

    PTB_RECORD(LogPTBProfile,
        TEXT("DeleteProfile success: Nickname=%s SlotIndex=%d"),
        *TargetProfile.Nickname, TargetProfile.SlotIndex);

    OnProfileListChanged.Broadcast();
    RequestSave();

    return true;
}

bool UPTBProfileSubsystem::DeleteProfileBySlot(int32 SlotIndex)
{
    bool bFound = false;
    const FPTBProfileData Profile = GetProfileBySlot(SlotIndex, bFound);

    if (!bFound)
    {
        PTB_ERROR(LogPTBProfile,
            TEXT("DeleteProfileBySlot failed: SlotIndex=%d에 프로필이 없습니다."), SlotIndex);
        return false;
    }

    PTB_RECORD(LogPTBProfile,
        TEXT("DeleteProfileBySlot: SlotIndex=%d → Nickname=%s Id=%s"),
        SlotIndex, *Profile.Nickname, *Profile.ProfileId.ToString());

    return DeleteProfile(Profile.ProfileId);
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

    // GameInstance의 CurrentSaveGame에 프로필 반영 후 GameInstance를 통해 저장
    if (UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance()))
    {
        if (GI->CurrentSaveGame)
        {
            GI->CurrentSaveGame->Profiles = AllProfiles;
            GI->SaveGame();
            return;
        }
    }

    // GameInstance 없거나 CurrentSaveGame 없을 때 직접 저장
    UPTBSaveGame* SaveGame = GetOrCreateSaveGame();
    if (!SaveGame)
    {
        PTB_ERROR(LogPTBProfile, TEXT("RequestSave: SaveGame null"));
        return;
    }
    SaveGame->Profiles = AllProfiles;
    const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, TEXT("PTBSave"), 0);
    PTB_RECORD(LogPTBProfile, TEXT("RequestSave %s"), bSuccess ? TEXT("Success") : TEXT("FAIL"));
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
    if (UPTBGameInstance* GI = Cast<UPTBGameInstance>(GetGameInstance()))
    {
        if (GI->CurrentSaveGame)
            return GI->CurrentSaveGame;
    }

    // GameInstance가 아직 준비 안 됐을 때 디스크에서 직접 로드
    const FString SlotName = TEXT("PTBSave");
    const int32 UserIndex = 0;

    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        USaveGame* Loaded = UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex);
        if (UPTBSaveGame* PTBSave = Cast<UPTBSaveGame>(Loaded))
            return PTBSave;
    }

    UPTBSaveGame* NewSave = Cast<UPTBSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPTBSaveGame::StaticClass()));
    if (NewSave)
        NewSave->InitializeDefaultSave();
    return NewSave;
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
