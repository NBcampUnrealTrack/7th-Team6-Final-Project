#include "PTBProfileSubsystem.h"
#include "Core/PTBSaveGame.h"
#include "Core/PTBGameInstance.h"
#include "Debug/PTBTeamLog.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

void UPTBProfileSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    PTB_RECORD(LogPTBProfile, TEXT("Initialize"));

    LoadForbiddenWords();
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

    if (ContainsForbiddenWord(Normalized))
    {
        return EPTBNicknameValidationResult::ContainsForbiddenWord;
    }

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

// ── 금칙어 필터 ──────────────────────────────────────────────────────────────

void UPTBProfileSubsystem::LoadForbiddenWords()
{
    ForbiddenWords.Empty();

    // 패키징 후에도 접근 가능한 경로: Content/Data/ForbiddenWords.txt
    const FString FilePath = FPaths::ProjectContentDir() / TEXT("Data/ForbiddenWords.txt");

    FString RawContent;
    if (!FFileHelper::LoadFileToString(RawContent, *FilePath))
    {
        PTB_WARNING(LogPTBProfile,
            TEXT("LoadForbiddenWords: 금칙어 파일을 찾을 수 없습니다 — 필터링 비활성화. 경로: %s"),
            *FilePath);
        return;
    }

    TArray<FString> Lines;
    RawContent.ParseIntoArrayLines(Lines, /*bCullEmpty=*/true);

    for (const FString& Line : Lines)
    {
        const FString Trimmed = Line.TrimStartAndEnd();

        // '#' 으로 시작하는 줄은 주석으로 무시
        if (Trimmed.IsEmpty() || Trimmed.StartsWith(TEXT("#")))
        {
            continue;
        }

        ForbiddenWords.Add(NormalizeForFilter(Trimmed));
    }

    PTB_RECORD(LogPTBProfile, TEXT("LoadForbiddenWords: %d개 금칙어 로드 완료"), ForbiddenWords.Num());
}

FString UPTBProfileSubsystem::NormalizeForFilter(const FString& Input)
{
    // ── 한글 자모 테이블 ────────────────────────────────────────────
    //
    // 한글 음절(가~힣, U+AC00~U+D7A3)을 초성·중성·종성 호환 자모로 분해한다.
    // 이렇게 하면 아래 세 입력이 동일한 문자열로 정규화된다:
    //   "시발"           → "ㅅㅣㅂㅏㄹ"   (음절 → 자모 분해)
    //   "ㅅ ㅣ ㅂ ㅏ ㄹ" → "ㅅㅣㅂㅏㄹ"   (공백 제거)
    //   "시×발"          → "ㅅㅣㅂㅏㄹ"   (특수문자 제거 후 분해)
    //
    // 초성 19개 (유니코드 한글 호환 자모 블록 U+3131~U+314E)
    static const TCHAR InitialJamo[19] = {
        0x3131, // ㄱ
        0x3132, // ㄲ
        0x3134, // ㄴ
        0x3137, // ㄷ
        0x3138, // ㄸ
        0x3139, // ㄹ
        0x3141, // ㅁ
        0x3142, // ㅂ
        0x3143, // ㅃ
        0x3145, // ㅅ
        0x3146, // ㅆ
        0x3147, // ㅇ
        0x3148, // ㅈ
        0x3149, // ㅉ
        0x314A, // ㅊ
        0x314B, // ㅋ
        0x314C, // ㅌ
        0x314D, // ㅍ
        0x314E, // ㅎ
    };

    // 중성 21개 (U+314F~U+3163)
    static const TCHAR MedialJamo[21] = {
        0x314F, // ㅏ
        0x3150, // ㅐ
        0x3151, // ㅑ
        0x3152, // ㅒ
        0x3153, // ㅓ
        0x3154, // ㅔ
        0x3155, // ㅕ
        0x3156, // ㅖ
        0x3157, // ㅗ
        0x3158, // ㅘ
        0x3159, // ㅙ
        0x315A, // ㅚ
        0x315B, // ㅛ
        0x315C, // ㅜ
        0x315D, // ㅝ
        0x315E, // ㅞ
        0x315F, // ㅟ
        0x3160, // ㅠ
        0x3161, // ㅡ
        0x3162, // ㅢ
        0x3163, // ㅣ
    };

    // 종성 28개 (인덱스 0 = 받침 없음)
    static const TCHAR FinalJamo[28] = {
        0,      // (없음)
        0x3131, // ㄱ
        0x3132, // ㄲ
        0x3133, // ㄳ
        0x3134, // ㄴ
        0x3135, // ㄵ
        0x3136, // ㄶ
        0x3137, // ㄷ
        0x3139, // ㄹ
        0x313A, // ㄺ
        0x313B, // ㄻ
        0x313C, // ㄼ
        0x313D, // ㄽ
        0x313E, // ㄾ
        0x313F, // ㄿ
        0x3140, // ㅀ
        0x3141, // ㅁ
        0x3142, // ㅂ
        0x3144, // ㅄ
        0x3145, // ㅅ
        0x3146, // ㅆ
        0x3147, // ㅇ
        0x3148, // ㅈ
        0x314A, // ㅊ
        0x314B, // ㅋ
        0x314C, // ㅌ
        0x314D, // ㅍ
        0x314E, // ㅎ
    };

    // ── 정규화 ───────────────────────────────────────────────────────
    FString Result;
    Result.Reserve(Input.Len() * 3); // 음절 1개 → 최대 자모 3개

    for (const TCHAR Ch : Input)
    {
        // ① 한글 음절(가~힣): 초성+중성+종성 자모로 분해
        if (Ch >= 0xAC00 && Ch <= 0xD7A3)
        {
            const int32 Offset     = Ch - 0xAC00;
            const int32 FinalIdx   = Offset % 28;
            const int32 MedialIdx  = (Offset / 28) % 21;
            const int32 InitialIdx = Offset / 28 / 21;

            Result.AppendChar(InitialJamo[InitialIdx]);
            Result.AppendChar(MedialJamo[MedialIdx]);
            if (FinalIdx != 0)
            {
                Result.AppendChar(FinalJamo[FinalIdx]);
            }
        }
        // ② 한글 호환 자모(ㄱ~ㅣ): 이미 자모 형태이므로 그대로 유지
        else if (Ch >= 0x3131 && Ch <= 0x3163)
        {
            Result.AppendChar(Ch);
        }
        // ③ 영숫자: 소문자로 변환
        else if (FChar::IsAlpha(Ch) || FChar::IsDigit(Ch))
        {
            Result.AppendChar(FChar::ToLower(Ch));
        }
        // ④ 공백·특수문자·이모지 등: 제거 (우회 차단)
    }

    return Result;
}

bool UPTBProfileSubsystem::ContainsForbiddenWord(const FString& Nickname) const
{
    if (ForbiddenWords.IsEmpty())
    {
        return false;
    }

    const FString Normalized = NormalizeForFilter(Nickname);

    for (const FString& Word : ForbiddenWords)
    {
        if (Normalized.Contains(Word, ESearchCase::CaseSensitive))
        {
            // 어떤 단어에 걸렸는지는 로그에 남기지 않음 (금칙어 노출 방지)
            PTB_WARNING(LogPTBProfile, TEXT("ContainsForbiddenWord: 금칙어 감지됨 (닉네임 길이=%d)"), Nickname.Len());
            return true;
        }
    }
    return false;
}
