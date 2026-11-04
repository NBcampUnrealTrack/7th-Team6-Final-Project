/*
 * file: game_support_types.h
 * Pure virtual interface and callback definitions used in GameSupportSDK.
 */

#pragma once
#include "game_support_misc.h"
#include "base_types.h"


 // ============================================================
 //  GameSupportSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveGameSupportTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_GAMESUPPORT_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveGameSupportTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStoveStatFullId
// ============================================================

// Structure representing the full identification of a Stat.
struct IStoveStatFullId : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_StatFullId);

    // Returns the game ID associated with the stat.
    virtual const wchar_t* GetGameId() const = 0;

    // Returns the stat ID.
    // Format: [A-Z0-9\-_]*$ (uppercase letters, numbers, underscore, hyphen).
    virtual const wchar_t* GetStatsId() const = 0;
};


// ============================================================
//  IStoveFetchStatParam
// ============================================================

// Parameters for Stove_FetchStat().
// Create via Stove_CreateParam(k_EStoveGameSupportTypeKind_FetchStatParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveFetchStatParam : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_FetchStatParam);

    // Stat ID to query.
    // Format: [A-Z0-9\-_]*$ (uppercase letters, numbers, underscore, hyphen).
    virtual const wchar_t* GetStatsId() const = 0;
    virtual void SetStatsId(const wchar_t* statsId) = 0;
};


// ============================================================
//  IStoveSetStatParam
// ============================================================

// Parameters for Stove_SetStat().
// Create via Stove_CreateParam(k_EStoveGameSupportTypeKind_SetStatParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveSetStatParam : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_SetStatParam);

    // Stat ID to modify.
    // Format: [A-Z0-9\-_]*$ (uppercase letters, numbers, underscore, hyphen).
    virtual const wchar_t* GetStatsId() const = 0;
    virtual void SetStatsId(const wchar_t* statsId) = 0;

    // New value to apply to the Stat.
    virtual int32_t GetStatValue() const = 0;
    virtual void SetStatValue(int32_t statValue) = 0;
};


// ============================================================
//  IStoveFetchAchievementParam
// ============================================================

// Parameters for Stove_FetchAchievement().
// Create via Stove_CreateParam(k_EStoveGameSupportTypeKind_FetchAchievementParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveFetchAchievementParam : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_FetchAchievementParam);

    // Achievement ID to query.
    virtual const wchar_t* GetAchievementId() const = 0;
    virtual void SetAchievementId(const wchar_t* achievementId) = 0;
};


// ============================================================
//  IStoveFetchAchievementsParam
// ============================================================

// Parameters for Stove_FetchAchievements().
// Create via Stove_CreateParam(k_EStoveGameSupportTypeKind_FetchAchievementsParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveFetchAchievementsParam : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_FetchAchievementsParam);

    // Achievement ID to query.
    // Only used when PageSize is set to a valid value.
    // Pass an empty value for the first query.
    // For subsequent (paged) queries, use the achievement ID of the last item
    // in the previously returned achievement list.
    virtual const wchar_t* GetAchievementId() const = 0;
    virtual void SetAchievementId(const wchar_t* achievementId) = 0;

    // Count of achievements to be queried per page (0 < pageSize <= 100).
    // If left unmodified or set to a value outside the valid range, the full achievement list is queried.
    virtual uint32_t GetPageSize() const = 0;
    virtual void SetPageSize(uint32_t pageSize) = 0;
};


// ============================================================
//  IStoveStat
// ============================================================

// Structure containing Stat information received when calling the Stat API.
struct IStoveStat : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_Stat);

    // Returns the full ID of the Stat.
    virtual const IStoveStatFullId* GetStatsFullId() const = 0;

    // Returns the last update time of the Stat (epoch time in milliseconds).
    virtual uint64_t GetUpdatedAt() const = 0;

    // Returns the current value of the Stat.
    virtual int32_t GetCurrentValue() const = 0;
};


// ============================================================
//  IStoveSetStatOutcome
// ============================================================

// Structure containing the result received when calling the SetStat API.
struct IStoveSetStatOutcome : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_SetStatOutcome);

    // Returns the error message, if any occurred during modification.
    virtual const wchar_t* GetErrorMessage() const = 0;

    // Returns the current (updated) value of the Stat.
    virtual int32_t GetCurrentValue() const = 0;

    // Returns true if the value of the Stat has been successfully updated.
    virtual bool IsUpdated() const = 0;
};


// ============================================================
//  IStoveAchievementCondition
// ============================================================

// Structure representing the conditions required to complete an Achievement.
struct IStoveAchievementCondition : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_AchievementCondition);

    // Returns the value processing method.
    // Examples: "DELTA_SUM" (Cumulative data sum), "REPLACE" (Replace existing data),
    // "MAX" (Replace with larger value), "MIN" (Replace with smaller value).
    virtual const wchar_t* GetValueOperation() const = 0;

    // Returns the comparison method between the goal value and user value.
    // Examples: "GTE" (>=), "LTE" (<=).
    virtual const wchar_t* GetType() const = 0;

    // Returns the target goal value for the achievement.
    virtual int32_t GetGoalValue() const = 0;
};


// ============================================================
//  IStoveAchievement
// ============================================================

// Structure containing Achievement information received when calling the Achievement APIs.
struct IStoveAchievement : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_Achievement);

    // Returns the condition details of the Achievement.
    virtual const IStoveAchievementCondition* GetCondition() const = 0;

    // Returns the current progress value of the Achievement.
    virtual int32_t GetValue() const = 0;

    // Returns the ID of the Achievement.
    virtual const wchar_t* GetAchievementId() const = 0;

    // Returns the localized name of the Achievement.
    virtual const wchar_t* GetName() const = 0;

    // Returns the localized description of the Achievement.
    virtual const wchar_t* GetDescription() const = 0;

    // Returns the URL of the default (unachieved) image.
    virtual const wchar_t* GetDefaultImage() const = 0;

    // Returns the URL of the achieved image.
    virtual const wchar_t* GetAchievedImage() const = 0;

    // Returns the current status of the Achievement.
    // Examples: "ONGOING" (In progress), "ACHIEVED" (Achievement unlocked).
    virtual const wchar_t* GetStatus() const = 0;
};


// ============================================================
//  IStoveFetchRankingParam
// ============================================================

// Parameters for Stove_FetchRanking().
// Create via Stove_CreateParam(k_EStoveGameSupportTypeKind_FetchRankingParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveFetchRankingParam : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_FetchRankingParam);

    // Leaderboard identifier created by the studio.
    virtual const wchar_t* GetLeaderboardId() const = 0;
    virtual void SetLeaderboardId(const wchar_t* leaderboardId) = 0;

    // Page number to be queried (1 <= pageIndex).
    virtual uint32_t GetPageIndex() const = 0;
    virtual void SetPageIndex(uint32_t pageIndex) = 0;

    // Count of rankings to be queried per page (1 <= pageSize <= 50).
    virtual uint32_t GetPageSize() const = 0;
    virtual void SetPageSize(uint32_t pageSize) = 0;

    // Whether to include the logged-in user's ranking in the query results.
    virtual bool GetIncludeMyRank() const = 0;
    virtual void SetIncludeMyRank(bool includeMyRank) = 0;
};


// ============================================================
//  IStoveRank
// ============================================================

// Structure containing Rank information received when calling the Rank API.
struct IStoveRank : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_Rank);

    // Returns the user's nickname.
    virtual const wchar_t* GetNickname() const = 0;

    // Returns the URL of the user's profile image.
    virtual const wchar_t* GetProfileImage() const = 0;

    // Returns the user's score on the leaderboard.
    virtual int32_t GetScore() const = 0;

    // Returns the user's current rank.
    virtual uint32_t GetRank() const = 0;
};


// ============================================================
//  IStoveAchievementList
// ============================================================

// Container delivered by OnGetAchievementsCallback.
// Wraps the paged list of achievements returned by Stove_FetchAchievements().
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveAchievementList : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_AchievementList);

    // Returns the number of achievements in this page.
    virtual uint32_t GetCount() const = 0;

    // Returns the achievement at the given zero-based index.
    // Returns nullptr if index >= GetCount().
    virtual const IStoveAchievement* GetAt(uint32_t index) const = 0;

    // Returns the total number of achievements across all pages.
    // When paging is not applied, returns the same value as GetCount().
    virtual uint32_t GetTotalCount() const = 0;
};


// ============================================================
//  IStoveRanking
// ============================================================

// Container delivered by OnGetRankingCallback.
// Wraps the paged leaderboard returned by Stove_FetchRanking().
// "Ranking" denotes the whole leaderboard; "Rank" denotes a single entry.
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveRanking : IStoveTypeBase
{
    STOVE_GAMESUPPORT_TYPE_KIND(k_EStoveGameSupportTypeKind_Ranking);

    // Returns the number of rank entries in this page.
    virtual uint32_t GetCount() const = 0;

    // Returns the rank entry at the given zero-based index.
    // Returns nullptr if index >= GetCount().
    virtual const IStoveRank* GetAt(uint32_t index) const = 0;

    // Returns the total number of rank entries across all pages.
    virtual uint32_t GetTotalCount() const = 0;
};


#else

typedef struct IStoveStatFullId IStoveStatFullId;
typedef struct IStoveFetchStatParam IStoveFetchStatParam;
typedef struct IStoveSetStatParam IStoveSetStatParam;
typedef struct IStoveFetchAchievementParam IStoveFetchAchievementParam;
typedef struct IStoveStat IStoveStat;
typedef struct IStoveSetStatOutcome IStoveSetStatOutcome;
typedef struct IStoveAchievementCondition IStoveAchievementCondition;
typedef struct IStoveAchievement IStoveAchievement;
typedef struct IStoveAchievementList IStoveAchievementList;
typedef struct IStoveFetchAchievementsParam IStoveFetchAchievementsParam;
typedef struct IStoveFetchRankingParam IStoveFetchRankingParam;
typedef struct IStoveRank IStoveRank;
typedef struct IStoveRanking IStoveRanking;

#endif // __cplusplus


// ============================================================
//  GameSupportSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and any accompanying object pointers
// (stat, achievement, ranks, ...) are owned by the SDK and are valid only for
// the duration of the callback invocation; do NOT call Destroy() on them.

// Delivered by Stove_FetchStat().
typedef void(__cdecl* OnGetStatCallback)(const IStoveCallbackResult* callbackResult, const IStoveStat* stat);

// Delivered by Stove_SetStat().
typedef void(__cdecl* OnSetStatCallback)(const IStoveCallbackResult* callbackResult, const IStoveSetStatOutcome* statOutcome);

// Delivered by Stove_FetchAchievement().
typedef void(__cdecl* OnGetAchievementCallback)(const IStoveCallbackResult* callbackResult, const IStoveAchievement* achievement);

// Delivered by Stove_FetchAchievements().
typedef void(__cdecl* OnGetAchievementsCallback)(const IStoveCallbackResult* callbackResult, const IStoveAchievementList* list);

// Delivered by Stove_FetchRanking().
typedef void(__cdecl* OnGetRankingCallback)(const IStoveCallbackResult* callbackResult, const IStoveRanking* ranking);
