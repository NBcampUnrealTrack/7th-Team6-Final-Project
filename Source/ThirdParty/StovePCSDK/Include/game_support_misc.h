/*
 * file: game_support_misc.h
 * Enum definitions used in GameSupportSDK.
 */

#pragma once
#include "base_misc.h"


 // ============================================================
 //  GameSupportSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance.
 // Used by Stove_CreateParam() to request a specific parameter object,
 // and by IStoveTypeBase::GetTypeKind() for runtime type identification.
 // Range (5000 ~ 5999)
typedef enum EStoveGameSupportTypeKind
{
    k_EStoveGameSupportTypeKind_Invalid = -1,

    // Result / Data types (returned or delivered by the SDK)
    k_EStoveGameSupportTypeKind_StatFullId = 5000,
    k_EStoveGameSupportTypeKind_Stat = 5001,
    k_EStoveGameSupportTypeKind_SetStatOutcome = 5002,
    k_EStoveGameSupportTypeKind_AchievementCondition = 5003,
    k_EStoveGameSupportTypeKind_Achievement = 5004,
    k_EStoveGameSupportTypeKind_Rank = 5005,
    k_EStoveGameSupportTypeKind_AchievementList = 5006,
    k_EStoveGameSupportTypeKind_Ranking = 5007,

    // Parameter types (created by the caller via Stove_CreateParam)
    k_EStoveGameSupportTypeKind_FetchStatParam = 5500,
    k_EStoveGameSupportTypeKind_SetStatParam = 5501,
    k_EStoveGameSupportTypeKind_FetchAchievementParam = 5502,
    k_EStoveGameSupportTypeKind_FetchAchievementsParam = 5503,
    k_EStoveGameSupportTypeKind_FetchRankingParam = 5504,

    k_EStoveGameSupportTypeKind_Max = 0x7fffffff
} EStoveGameSupportTypeKind;


// Identifies which GameSupportSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 5500~5999 are an internal diagnostic band and are not part of this enum.
typedef enum EStoveGameSupportMethodCode
{
    k_EStoveGameSupportMethodCode_Invalid = -1,

    // Public APIs (Stat, Achievement, Rank). Module block 5000~5999 (sequential).
    k_EStoveGameSupportMethodCode_FetchStat = 5000,
    k_EStoveGameSupportMethodCode_SetStat = 5001,
    k_EStoveGameSupportMethodCode_FetchAchievement = 5002,
    k_EStoveGameSupportMethodCode_FetchAchievements = 5003,
    k_EStoveGameSupportMethodCode_FetchRanking = 5004,

    k_EStoveGameSupportMethodCode_Max = 0x7fffffff
} EStoveGameSupportMethodCode;