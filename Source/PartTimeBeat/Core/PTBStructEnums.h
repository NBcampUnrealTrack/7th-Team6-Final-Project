#pragma once
#include "CoreMinimal.h"

#include "PTBStructEnums.generated.h"

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
enum class EPTBJudgementType : uint8
{
    HighPerfect      UMETA(DisplayName = "21ms"),
    Perfect        UMETA(DisplayName = "50ms"),
    Good       UMETA(DisplayName = "70ms"),
    Miss    UMETA(DisplayName = "초과")
};

UENUM(BlueprintType)
enum class EPTBGradeType : uint8
{
    PerfectFullCombo      UMETA(DisplayName = "전노트 Perpect이상"),
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
    // 초기화 해주는 게 좋긴 한데 나중에
    /*FPTBNoteEvent()
        : Time(0.0f)
        , NoteType(0)
    {
    }*/

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 NoteId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BeatTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float TimeMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBActionType ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsLongNote;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float DurationBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName SectionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, FString> Payload;
};


USTRUCT(BlueprintType)
struct FPTBChartData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName ChartId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName SongId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BPM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float OffsetMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float SongLengthMs;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TArray TempoChanges;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TArray NoteEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName WwiseEventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName WwiseBankName;
};

USTRUCT(BlueprintType)
struct FPTBJudgementResult
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 NoteId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBActionType ActionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBJudgementType JudgementType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float DeltaMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ScoreDelta;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bBreaksCombo;

};


USTRUCT(BlueprintType)
struct FPTBProfileData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FGuid ProfileId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FString Nickname;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FDateTime Birthday;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TSet CompletedTutorialIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, int32> BestScoresByMiniGame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    TMap<FName, int32> EarnedStarsByMiniGame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TotalEarnedMoney;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TSet CompletedStoryIds;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //Set UnlockedCostumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FDateTime CreatedAt;
};


USTRUCT(BlueprintType)
struct FPTBUserSettings
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BGMVolume;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float SFXVolume;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float JudgementOffsetMs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float InputLatencyMs;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bFullscreen;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bVibrationEnabled;
};

USTRUCT(BlueprintType)
struct FPTBGameSessionRequest
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FGuid ProfileId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameCode;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBPlayMode PlayMode;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RandomSeed;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 ExpectedPlayerCount;

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
    int32 LocalPlayerIndex;
};


USTRUCT(BlueprintType)
struct FPTBMiniGameResultPayload
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName PayloadType;

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
    FName MiniGameId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 Score;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 HighPerfectCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 PerfectCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 GoodCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 MissCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 MaxCombo;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float AccuracyRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBGradeType Grade;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 StarCount;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedMoney;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool IsNewHighScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FPTBMiniGameResultPayload MiniGamePayload;
};

USTRUCT(BlueprintType)
struct FPTBRewardSummary
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedMoney;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 EarnedStars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TotalMoney;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 TotalStars;

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
    FName StageId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName MiniGameId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RequiredStars;

    //TArray PrerequisiteStageIds;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 BestStarRating;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 BestScore;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    EPTBGradeType BestGrade;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsUnlocked;
};


USTRUCT(BlueprintType)
struct FPTBStoryChapter
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FName ChapterId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    FText Title;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //TArray UnlockConditionStageIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 UnlockConditionStars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    int32 RequiredMoney;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    UDataTable* DialogueDataTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsViewed;
};

USTRUCT(BlueprintType)
struct FPTBWwiseSyncData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float CurrentPlaybackMs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float CurrentBeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BPM;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    bool bIsPlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    double DspTime;
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
    FName MiniGameId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    float BeatTime;

    //UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rhythm")
    //ENetRole NetRole;
};