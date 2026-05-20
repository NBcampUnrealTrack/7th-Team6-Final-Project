#pragma once

#include "CoreMinimal.h"
#include "PTBProfileTypes.generated.h"

UENUM(BlueprintType)
enum class EPTBGender : uint8
{
    Unset   = 0  UMETA(DisplayName = "Unset"),
    Male    = 1  UMETA(DisplayName = "Male"),
    Female  = 2  UMETA(DisplayName = "Female"),
    Other   = 3  UMETA(DisplayName = "Other")
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

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Identity")
    FGuid ProfileId;

    UPROPERTY(BlueprintReadWrite, Category = "Profile|Identity")
    FString Nickname;

    UPROPERTY(BlueprintReadWrite, Category = "Profile|Identity")
    EPTBGender Gender = EPTBGender::Unset;

    UPROPERTY(BlueprintReadWrite, Category = "Profile|Identity")
    FDateTime Birthday;

    //Progress

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    TSet<FName> CompletedTutorialIds;

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    TMap<FName, int32> BestScoresByMiniGame;

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    TMap<FName, int32> EarnedStarsByMiniGame;

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    int32 TotalEarnedMoney = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    TSet<FName> CompletedStoryIds;

    // 코스튬 추가 시
    //UPROPERTY(BlueprintReadOnly, Category = "Profile|Progress")
    //TSet<FName> UnlockedCostumes;


    UPROPERTY(BlueprintReadOnly, Category = "Profile")
    FDateTime CreatedAt;

    UPROPERTY(BlueprintReadOnly, Category = "Profile")
    FDateTime LastPlayedAt;

    FPTBProfileData()
        : ProfileId(FGuid::NewGuid())
        , Nickname(TEXT(""))
        , Gender(EPTBGender::Unset)
        , Birthday(FDateTime::Now())
        , TotalEarnedMoney(0)
        , CreatedAt(FDateTime::Now())
        , LastPlayedAt(FDateTime::Now())
    {
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
