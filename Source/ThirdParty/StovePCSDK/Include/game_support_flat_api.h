/*
 * file: game_support_flat_api.h
 * Flat C accessor function declarations for GameSupportSDK interface members.
 * Provides wrapper functions for accessing interface properties
 * in C/C# environments where vtable calls are not directly available.
 *
 * Each accessor follows the naming pattern:
 * Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *
 * For the semantics, units, and valid ranges of each field,
 * refer to the matching interface definition in `game_support_types.h`.
 * Detailed comments are intentionally omitted here to maintain readability.
 */

#pragma once
#include "game_support_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveStatFullId  —  Full identification of a Stat
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveStatFullId_GetGameId(const IStoveStatFullId* self);
    const wchar_t* STOVE_API Stove_IStoveStatFullId_GetStatsId(const IStoveStatFullId* self);

    // ============================================================
    //  IStoveFetchStatParam  —  Stat query parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveFetchStatParam_GetStatsId(const IStoveFetchStatParam* self);
    void STOVE_API Stove_IStoveFetchStatParam_SetStatsId(IStoveFetchStatParam* self, const wchar_t* statsId); // Non-const

    // ============================================================
    //  IStoveSetStatParam  —  SetStat query parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveSetStatParam_GetStatsId(const IStoveSetStatParam* self);
    void STOVE_API Stove_IStoveSetStatParam_SetStatsId(IStoveSetStatParam* self, const wchar_t* statsId); // Non-const

    int32_t STOVE_API Stove_IStoveSetStatParam_GetStatValue(const IStoveSetStatParam* self);
    void STOVE_API Stove_IStoveSetStatParam_SetStatValue(IStoveSetStatParam* self, int32_t statValue); // Non-const

    // ============================================================
    //  IStoveFetchAchievementParam  —  Achievement query parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveFetchAchievementParam_GetAchievementId(const IStoveFetchAchievementParam* self);
    void STOVE_API Stove_IStoveFetchAchievementParam_SetAchievementId(IStoveFetchAchievementParam* self, const wchar_t* achievementId); // Non-const

    // ============================================================
    //  IStoveFetchAchievementsParam  —  Achievements (paged list) query parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveFetchAchievementsParam_GetAchievementId(const IStoveFetchAchievementsParam* self);
    void STOVE_API Stove_IStoveFetchAchievementsParam_SetAchievementId(IStoveFetchAchievementsParam* self, const wchar_t* achievementId); // Non-const

    uint32_t STOVE_API Stove_IStoveFetchAchievementsParam_GetPageSize(const IStoveFetchAchievementsParam* self);
    void STOVE_API Stove_IStoveFetchAchievementsParam_SetPageSize(IStoveFetchAchievementsParam* self, uint32_t pageSize); // Non-const

    // ============================================================
    //  IStoveStat  —  Stat information
    // ============================================================
    const IStoveStatFullId* STOVE_API Stove_IStoveStat_GetStatsFullId(const IStoveStat* self);
    uint64_t STOVE_API Stove_IStoveStat_GetUpdatedAt(const IStoveStat* self);
    int32_t STOVE_API Stove_IStoveStat_GetCurrentValue(const IStoveStat* self);

    // ============================================================
    //  IStoveSetStatOutcome  —  Result of a Stat modification
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveSetStatOutcome_GetErrorMessage(const IStoveSetStatOutcome* self);
    int32_t STOVE_API Stove_IStoveSetStatOutcome_GetCurrentValue(const IStoveSetStatOutcome* self);
    bool STOVE_API Stove_IStoveSetStatOutcome_IsUpdated(const IStoveSetStatOutcome* self);

    // ============================================================
    //  IStoveAchievementCondition  —  Achievement completion conditions
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveAchievementCondition_GetValueOperation(const IStoveAchievementCondition* self);
    const wchar_t* STOVE_API Stove_IStoveAchievementCondition_GetType(const IStoveAchievementCondition* self);
    int32_t STOVE_API Stove_IStoveAchievementCondition_GetGoalValue(const IStoveAchievementCondition* self);

    // ============================================================
    //  IStoveAchievement  —  Achievement information
    // ============================================================
    const IStoveAchievementCondition* STOVE_API Stove_IStoveAchievement_GetCondition(const IStoveAchievement* self);
    int32_t STOVE_API Stove_IStoveAchievement_GetValue(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetAchievementId(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetName(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetDescription(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetDefaultImage(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetAchievedImage(const IStoveAchievement* self);
    const wchar_t* STOVE_API Stove_IStoveAchievement_GetStatus(const IStoveAchievement* self);

    // ============================================================
    //  IStoveFetchRankingParam  —  Rank query parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveFetchRankingParam_GetLeaderboardId(const IStoveFetchRankingParam* self);
    void STOVE_API Stove_IStoveFetchRankingParam_SetLeaderboardId(IStoveFetchRankingParam* self, const wchar_t* leaderboardId); // Non-const

    uint32_t STOVE_API Stove_IStoveFetchRankingParam_GetPageIndex(const IStoveFetchRankingParam* self);
    void STOVE_API Stove_IStoveFetchRankingParam_SetPageIndex(IStoveFetchRankingParam* self, uint32_t pageIndex); // Non-const

    uint32_t STOVE_API Stove_IStoveFetchRankingParam_GetPageSize(const IStoveFetchRankingParam* self);
    void STOVE_API Stove_IStoveFetchRankingParam_SetPageSize(IStoveFetchRankingParam* self, uint32_t pageSize); // Non-const

    bool STOVE_API Stove_IStoveFetchRankingParam_GetIncludeMyRank(const IStoveFetchRankingParam* self);
    void STOVE_API Stove_IStoveFetchRankingParam_SetIncludeMyRank(IStoveFetchRankingParam* self, bool includeMyRank); // Non-const

    // ============================================================
    //  IStoveRank  —  Rank information
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveRank_GetNickname(const IStoveRank* self);
    const wchar_t* STOVE_API Stove_IStoveRank_GetProfileImage(const IStoveRank* self);
    int32_t STOVE_API Stove_IStoveRank_GetScore(const IStoveRank* self);
    uint32_t STOVE_API Stove_IStoveRank_GetRank(const IStoveRank* self);

    // ============================================================
    //  IStoveAchievementList  —  Paged achievement container
    // ============================================================
    uint32_t STOVE_API Stove_IStoveAchievementList_GetCount(const IStoveAchievementList* self);
    const IStoveAchievement* STOVE_API Stove_IStoveAchievementList_GetAt(const IStoveAchievementList* self, uint32_t index);
    uint32_t STOVE_API Stove_IStoveAchievementList_GetTotalCount(const IStoveAchievementList* self);

    // ============================================================
    //  IStoveRanking  —  Paged leaderboard container
    // ============================================================
    uint32_t STOVE_API Stove_IStoveRanking_GetCount(const IStoveRanking* self);
    const IStoveRank* STOVE_API Stove_IStoveRanking_GetAt(const IStoveRanking* self, uint32_t index);
    uint32_t STOVE_API Stove_IStoveRanking_GetTotalCount(const IStoveRanking* self);


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveStatFullId =====
    typedef const wchar_t* (__cdecl* Stove_IStoveStatFullId_GetGameId_t)(const IStoveStatFullId* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveStatFullId_GetStatsId_t)(const IStoveStatFullId* self);

    // ===== IStoveFetchStatParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveFetchStatParam_GetStatsId_t)(const IStoveFetchStatParam* self);
    typedef void(__cdecl* Stove_IStoveFetchStatParam_SetStatsId_t)(IStoveFetchStatParam* self, const wchar_t* statsId); // Non-const

    // ===== IStoveSetStatParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveSetStatParam_GetStatsId_t)(const IStoveSetStatParam* self);
    typedef void(__cdecl* Stove_IStoveSetStatParam_SetStatsId_t)(IStoveSetStatParam* self, const wchar_t* statsId); // Non-const
    typedef int32_t(__cdecl* Stove_IStoveSetStatParam_GetStatValue_t)(const IStoveSetStatParam* self);
    typedef void(__cdecl* Stove_IStoveSetStatParam_SetStatValue_t)(IStoveSetStatParam* self, int32_t statValue); // Non-const

    // ===== IStoveFetchAchievementParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveFetchAchievementParam_GetAchievementId_t)(const IStoveFetchAchievementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchAchievementParam_SetAchievementId_t)(IStoveFetchAchievementParam* self, const wchar_t* achievementId); // Non-const

    // ===== IStoveFetchAchievementsParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveFetchAchievementsParam_GetAchievementId_t)(const IStoveFetchAchievementsParam* self);
    typedef void(__cdecl* Stove_IStoveFetchAchievementsParam_SetAchievementId_t)(IStoveFetchAchievementsParam* self, const wchar_t* achievementId); // Non-const
    typedef uint32_t(__cdecl* Stove_IStoveFetchAchievementsParam_GetPageSize_t)(const IStoveFetchAchievementsParam* self);
    typedef void(__cdecl* Stove_IStoveFetchAchievementsParam_SetPageSize_t)(IStoveFetchAchievementsParam* self, uint32_t pageSize); // Non-const

    // ===== IStoveStat =====
    typedef const IStoveStatFullId* (__cdecl* Stove_IStoveStat_GetStatsFullId_t)(const IStoveStat* self);
    typedef uint64_t(__cdecl* Stove_IStoveStat_GetUpdatedAt_t)(const IStoveStat* self);
    typedef int32_t(__cdecl* Stove_IStoveStat_GetCurrentValue_t)(const IStoveStat* self);

    // ===== IStoveSetStatOutcome =====
    typedef const wchar_t* (__cdecl* Stove_IStoveSetStatOutcome_GetErrorMessage_t)(const IStoveSetStatOutcome* self);
    typedef int32_t(__cdecl* Stove_IStoveSetStatOutcome_GetCurrentValue_t)(const IStoveSetStatOutcome* self);
    typedef bool(__cdecl* Stove_IStoveSetStatOutcome_IsUpdated_t)(const IStoveSetStatOutcome* self);

    // ===== IStoveAchievementCondition =====
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievementCondition_GetValueOperation_t)(const IStoveAchievementCondition* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievementCondition_GetType_t)(const IStoveAchievementCondition* self);
    typedef int32_t(__cdecl* Stove_IStoveAchievementCondition_GetGoalValue_t)(const IStoveAchievementCondition* self);

    // ===== IStoveAchievement =====
    typedef const IStoveAchievementCondition* (__cdecl* Stove_IStoveAchievement_GetCondition_t)(const IStoveAchievement* self);
    typedef int32_t(__cdecl* Stove_IStoveAchievement_GetValue_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetAchievementId_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetName_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetDescription_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetDefaultImage_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetAchievedImage_t)(const IStoveAchievement* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveAchievement_GetStatus_t)(const IStoveAchievement* self);

    // ===== IStoveFetchRankingParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveFetchRankingParam_GetLeaderboardId_t)(const IStoveFetchRankingParam* self);
    typedef void(__cdecl* Stove_IStoveFetchRankingParam_SetLeaderboardId_t)(IStoveFetchRankingParam* self, const wchar_t* leaderboardId); // Non-const
    typedef uint32_t(__cdecl* Stove_IStoveFetchRankingParam_GetPageIndex_t)(const IStoveFetchRankingParam* self);
    typedef void(__cdecl* Stove_IStoveFetchRankingParam_SetPageIndex_t)(IStoveFetchRankingParam* self, uint32_t pageIndex); // Non-const
    typedef uint32_t(__cdecl* Stove_IStoveFetchRankingParam_GetPageSize_t)(const IStoveFetchRankingParam* self);
    typedef void(__cdecl* Stove_IStoveFetchRankingParam_SetPageSize_t)(IStoveFetchRankingParam* self, uint32_t pageSize); // Non-const
    typedef bool(__cdecl* Stove_IStoveFetchRankingParam_GetIncludeMyRank_t)(const IStoveFetchRankingParam* self);
    typedef void(__cdecl* Stove_IStoveFetchRankingParam_SetIncludeMyRank_t)(IStoveFetchRankingParam* self, bool includeMyRank); // Non-const

    // ===== IStoveRank =====
    typedef const wchar_t* (__cdecl* Stove_IStoveRank_GetNickname_t)(const IStoveRank* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveRank_GetProfileImage_t)(const IStoveRank* self);
    typedef int32_t(__cdecl* Stove_IStoveRank_GetScore_t)(const IStoveRank* self);
    typedef uint32_t(__cdecl* Stove_IStoveRank_GetRank_t)(const IStoveRank* self);

    // ===== IStoveAchievementList =====
    typedef uint32_t(__cdecl* Stove_IStoveAchievementList_GetCount_t)(const IStoveAchievementList* self);
    typedef const IStoveAchievement* (__cdecl* Stove_IStoveAchievementList_GetAt_t)(const IStoveAchievementList* self, uint32_t index);
    typedef uint32_t(__cdecl* Stove_IStoveAchievementList_GetTotalCount_t)(const IStoveAchievementList* self);

    // ===== IStoveRanking =====
    typedef uint32_t(__cdecl* Stove_IStoveRanking_GetCount_t)(const IStoveRanking* self);
    typedef const IStoveRank* (__cdecl* Stove_IStoveRanking_GetAt_t)(const IStoveRanking* self, uint32_t index);
    typedef uint32_t(__cdecl* Stove_IStoveRanking_GetTotalCount_t)(const IStoveRanking* self);

#ifdef __cplusplus
}
#endif