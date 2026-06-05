#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "InputCoreTypes.h"

#include "PTBStructEnums.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeatTick, float, Beat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBarTick, int32, Bar);

UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
    MainMenu      UMETA(DisplayName = "게임 실행 직후"),
    ProfileSelect        UMETA(DisplayName = "메인에서 시작 클릭"),
    ProfileCreate       UMETA(DisplayName = "신규 프로필 생성"),
    ModeSelect    UMETA(DisplayName = "프로필 선택 완료"),
    MiniGameSelect    UMETA(DisplayName = "싱글 모드 선택"),
    DifficultySelect    UMETA(DisplayName = "미니게임 선택"),
    Tutorial    UMETA(DisplayName = "첫 플레이 or 사용자 요청"),
    InGame    UMETA(DisplayName = "카운트다운 완료"),
    Paused    UMETA(DisplayName = "일시정지"),
    Result    UMETA(DisplayName = "라운드 종료"),
    StoryViewer    UMETA(DisplayName = "스토리 챕터 해금"),
    Settings    UMETA(DisplayName = "설정 진입"),
    MultiLobby    UMETA(DisplayName = "멀티 진입")
};


UENUM(BlueprintType)
enum class EPTBActionType : uint8
{
    None      UMETA(DisplayName = "기본값/미사용"),
    ActionA        UMETA(DisplayName = "ActionA"),
    ActionB       UMETA(DisplayName = "ActionB"),
    ActionC    UMETA(DisplayName = "ActionC"),
    ActionD    UMETA(DisplayName = "ActionD"),
    ActionE    UMETA(DisplayName = "ActionE")
};

UENUM(BlueprintType)
enum class EPTBNoteType : uint8
{
    Tap      UMETA(DisplayName = "Tap"),
    Hold        UMETA(DisplayName = "Hold"),
    Release       UMETA(DisplayName = "Release")
};

UENUM(BlueprintType)
enum class EPTBJudgementType : uint8
{
    HighPerfect      UMETA(DisplayName = "21ms(High Perfect)"),
    Perfect        UMETA(DisplayName = "50ms(Perfect)"),
    Good       UMETA(DisplayName = "70ms(Good)"),
    Miss    UMETA(DisplayName = "초과(Miss)")
};

UENUM(BlueprintType)
enum class EPTBJudgementReason : uint8
{
    Note       UMETA(DisplayName = "Note"),
    ExpiredNote        UMETA(DisplayName = "Expired Note"),
    EmptyInput       UMETA(DisplayName = "Empty Input"),
    EarlyRelease       UMETA(DisplayName = "Early Release")
};

UENUM(BlueprintType)
enum class EPTBEmptyInputPolicy : uint8
{
    Ignore       UMETA(DisplayName = "Ignore(공입력 무시)"),
    Miss        UMETA(DisplayName = "Miss(공입력시 미스)")
};

UENUM(BlueprintType)
enum class EPTBCueLeadTimeMode : uint8
{
    Beat       UMETA(DisplayName = "Beat"),
    MS        UMETA(DisplayName = "ms")
};

UENUM(BlueprintType)
enum class EPTBGradeType : uint8
{
    PerfectFullCombo      UMETA(DisplayName = "전노트 Perfect이상"),
    FullCombo        UMETA(DisplayName = "Miss0"),
    S       UMETA(DisplayName = "95%이상"),
    A    UMETA(DisplayName = "85%이상"),
    B    UMETA(DisplayName = "70% 이상"),
    C    UMETA(DisplayName = "60% 이상"),
    Clear    UMETA(DisplayName = "50%이상"),
    Fail    UMETA(DisplayName = "50%미만")
};

UENUM(BlueprintType)
enum class EPTBDifficulty : uint8
{
    Easy      UMETA(DisplayName = "초보모드"),
    Standard        UMETA(DisplayName = "통상모드"),
    Insane       UMETA(DisplayName = "발광모드")
};

UENUM(BlueprintType)
enum class EPTBPlayMode : uint8
{
    Single      UMETA(DisplayName = "싱글 플레이"),
    Multiplayer        UMETA(DisplayName = "멀티 플레이")
};

UENUM(BlueprintType)
enum class EPTBRoundEndReason : uint8
{
    Completed      UMETA(DisplayName = "채보 끝까지 진행"),
    Failed        UMETA(DisplayName = "실패 조건 충족 "),
    Aborted        UMETA(DisplayName = "사용자 중단"),
    Disconnected        UMETA(DisplayName = "멀티 연결 끊김")
};


USTRUCT(BlueprintType)
struct FPTBNoteEvent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 NoteId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BeatTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float TimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBActionType ActionType = EPTBActionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBNoteType NoteType = EPTBNoteType::Tap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 Lane = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsLongNote = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float DurationBeat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ReleaseNoteId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float ReleaseBeatTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float ReleaseTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName SectionName = NAME_None;
};


USTRUCT(BlueprintType)
struct FPTBChartData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ChartVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TimeSignatureNumerator = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TimeSignatureDenominator = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TArray<float> TempoChangeBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TArray<float> TempoChangeBpms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TArray<float> TimeSignatureChangeBeats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TArray<int32> TimeSignatureChangeNumerators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TArray<int32> TimeSignatureChangeDenominators;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName ChartId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName SongId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty = EPTBDifficulty::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BPM = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float OffsetMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float SongLengthMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName WwiseEventName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName WwiseBankName = NAME_None;
};

USTRUCT(BlueprintType)
struct FPTBJudgementResult
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 NoteId = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBActionType ActionType = EPTBActionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBJudgementType JudgementType = EPTBJudgementType::Miss;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBJudgementReason Reason = EPTBJudgementReason::Note;

    /** 판정 기준 채보 시간(ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float ChartTimeMs = 0.0f;

    /** 판정에 사용된 입력 시간(ms) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float InputTimeMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float DeltaMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ScoreDelta = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bBreaksCombo = false;

};


/**
 * 리듬 게임 입력 키 바인딩.
 * 기본값: Z / X / C / V / B (ActionA ~ ActionE 순서)
 * 옵션 메뉴에서 변경 후 FPTBUserSettings와 함께 저장된다.
 */
USTRUCT(BlueprintType)
struct FPTBRhythmKeyBindings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FKey ActionA = EKeys::Z;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FKey ActionB = EKeys::X;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FKey ActionC = EKeys::C;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FKey ActionD = EKeys::V;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FKey ActionE = EKeys::B;

    /** FKey → EPTBActionType 변환. 매핑되지 않은 키는 None 반환. */
    EPTBActionType ResolveKey(const FKey& Key) const
    {
        if (Key == ActionA) return EPTBActionType::ActionA;
        if (Key == ActionB) return EPTBActionType::ActionB;
        if (Key == ActionC) return EPTBActionType::ActionC;
        if (Key == ActionD) return EPTBActionType::ActionD;
        if (Key == ActionE) return EPTBActionType::ActionE;
        return EPTBActionType::None;
    }

    /** EPTBActionType → 현재 바인딩된 FKey 반환 */
    FKey ResolveAction(EPTBActionType Action) const
    {
        switch (Action)
        {
        case EPTBActionType::ActionA: return ActionA;
        case EPTBActionType::ActionB: return ActionB;
        case EPTBActionType::ActionC: return ActionC;
        case EPTBActionType::ActionD: return ActionD;
        case EPTBActionType::ActionE: return ActionE;
        default:                      return EKeys::Invalid;
        }
    }
};

USTRUCT(BlueprintType)
struct FPTBUserSettings
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BGMVolume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float SFXVolume = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float JudgementOffsetMs = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float InputLatencyMs = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bFullscreen = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bVibrationEnabled = true;

    /** 리듬 게임 키 바인딩 (기본: Z/X/C/V/B) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PTB|Keys")
    FPTBRhythmKeyBindings RhythmKeys;
};

UENUM(BlueprintType)
enum class EPTBGender : uint8
{
    Unset = 0  UMETA(DisplayName = "Unset"),
    Male = 1  UMETA(DisplayName = "Male"),
    Female = 2  UMETA(DisplayName = "Female"),
    Other = 3  UMETA(DisplayName = "Other")
};

UENUM(BlueprintType)
enum class EPTBNicknameValidationResult : uint8
{
    Valid                  UMETA(DisplayName = "Valid"),
    Empty                  UMETA(DisplayName = "Empty"),
    TooShort               UMETA(DisplayName = "Too Short"),
    TooLong                UMETA(DisplayName = "Too Long"),
    ContainsForbiddenWord  UMETA(DisplayName = "Contains Forbidden Word"),
    AlreadyTaken           UMETA(DisplayName = "Already Taken")
};


USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBProfileData
{
    GENERATED_BODY()

    // Profile

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Identity")
    FGuid ProfileId;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Profile|Identity")
    FString Nickname;

    /** 프로필 선택 화면의 슬롯 인덱스 (0~2) */
    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Profile|Identity")
    int32 SlotIndex = -1;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Profile|Identity")
    EPTBGender Gender = EPTBGender::Unset;

    UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Profile|Identity")
    FDateTime Birthday;

    //Progress

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    TSet<FName> CompletedTutorialIds;

    /** 미니게임 전체 최고 점수 (난이도 무관) */
    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    TMap<FName, int32> BestScoresByMiniGame;

    /**
     * 난이도별 최고 점수.
     * 키 형식: "MiniGameId_Difficulty" (예: "TG_Standard")
     */
    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    TMap<FName, int32> BestScoresByDifficulty;

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    TMap<FName, int32> EarnedStarsByMiniGame;

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    int32 TotalEarnedMoney = 0;

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    TSet<FName> CompletedStoryIds;

    // 코스튬 추가 시
    //UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile|Progress")
    //TSet<FName> UnlockedCostumes;


    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile")
    FDateTime CreatedAt;

    UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Profile")
    FDateTime LastPlayedAt;

    FPTBProfileData()
        : Nickname(TEXT(""))
        , Gender(EPTBGender::Unset)
        , Birthday(FDateTime::Now())
        , TotalEarnedMoney(0)
        , CreatedAt(FDateTime::Now())
        , LastPlayedAt(FDateTime::Now())
    {
        // ProfileId는 CreateProfile에서 명시적으로 FGuid::NewGuid()로 할당
    }

    static FPTBProfileData MakeInvalid()
    {
        FPTBProfileData Empty;
        Empty.ProfileId = FGuid();
        Empty.Nickname = TEXT("");
        return Empty;
    }

    bool IsValid() const
    {
        return ProfileId.IsValid() && !Nickname.IsEmpty();
    }
};

//진행도 채울 struct
USTRUCT(BlueprintType)
struct PARTTIMEBEAT_API FPTBProfileProgressUpdate
{
    GENERATED_BODY()

    /** 어떤 미니게임의 결과인지 */
    UPROPERTY(BlueprintReadWrite, Category = "Profile|Update")
    FName MiniGameId;

    /** 플레이한 난이도 */
    UPROPERTY(BlueprintReadWrite, Category = "Profile|Update")
    EPTBDifficulty Difficulty = EPTBDifficulty::Standard;

    /** 이번 라운드 점수. 기존 최고점보다 높으면 갱신. */
    UPROPERTY(BlueprintReadWrite, Category = "Profile|Update")
    int32 Score = 0;

    /** 이번 라운드 별점. 기존 별보다 높으면 갱신. */
    UPROPERTY(BlueprintReadWrite, Category = "Profile|Update")
    int32 EarnedStars = 0;

    /** 이번 라운드 알바비. TotalEarnedMoney에 누적. */
    UPROPERTY(BlueprintReadWrite, Category = "Profile|Update")
    int32 EarnedMoney = 0;
};

USTRUCT(BlueprintType)
struct FPTBGameSessionRequest
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FGuid ProfileId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameCode = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty = EPTBDifficulty::Standard;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBPlayMode PlayMode = EPTBPlayMode::Single;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RandomSeed = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ExpectedPlayerCount = 1;
};

USTRUCT(BlueprintType)
struct FPTBMiniGameContext
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FPTBGameSessionRequest SessionRequest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FPTBChartData ChartData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FPTBUserSettings UserSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 LocalPlayerIndex = 0;
};


USTRUCT(BlueprintType)
struct FPTBMiniGameResultPayload
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName PayloadType = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, float> FloatValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, int32> IntValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, FString> StringValues;
};

USTRUCT(BlueprintType)
struct FPTBRoundResult
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FGuid ProfileId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty = EPTBDifficulty::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBPlayMode PlayMode;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 Score = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 HighPerfectCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 PerfectCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 GoodCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 MissCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 MaxCombo = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float AccuracyRate = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBGradeType Grade = EPTBGradeType::Fail;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 StarCount = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedMoney = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool IsNewHighScore = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FPTBMiniGameResultPayload MiniGamePayload;
};

USTRUCT(BlueprintType)
struct FPTBRewardSummary
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedMoney = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedStars = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TotalMoney = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TotalStars = 0;

    //TArray NewlyUnlockedMiniGames;
    //TArray NewlyUnlockedStories;
    //TArray NewlyUnlockedCostumes;

};


USTRUCT(BlueprintType)
struct FPTBStageInfo
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName StageId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RequiredStars = 0;

    //TArray PrerequisiteStageIds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 BestStarRating = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 BestScore = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBGradeType BestGrade = EPTBGradeType::Fail;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsUnlocked = false;
};


USTRUCT(BlueprintType)
struct FPTBStoryChapter
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName ChapterId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FText Title;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TArray UnlockConditionStageIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 UnlockConditionStars = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RequiredMoney = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    UDataTable* DialogueDataTable = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsViewed = false;
};

USTRUCT(BlueprintType)
struct FPTBWwiseSyncData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float CurrentPlaybackMs = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float CurrentBeat = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BPM = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsPlaying = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    double DspTime = 0.0;
};

USTRUCT(BlueprintType)
struct FPTBLogContext
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FGuid SessionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FString PlayerId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BeatTime = 0.0f;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //ENetRole NetRole;
};

USTRUCT(BlueprintType)
struct FPTBLobbyPlayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
    FString Nickname;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
    bool bReady = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Lobby")
    bool bIsHost = false;
};

USTRUCT(BlueprintType)
struct FPTBPlayerRanking
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Ranking")
    FString PlayerId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Ranking")
    int32 Rank = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Ranking")
    int32 Score = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Ranking")
    int32 MaxCombo = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "PTB|Ranking")
    float AccuracyRate = 0.0f;
};
