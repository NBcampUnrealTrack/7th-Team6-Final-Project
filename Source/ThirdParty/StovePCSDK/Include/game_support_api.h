/*
 * file: game_support_api.h
 * Flat C API function declarations provided by GameSupportSDK.
 */

#pragma once
#include "game_support_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  GameSupport API
    // ============================================================


    // Retrieves the stat information for the given stat ID.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchStat(const IStoveFetchStatParam* param, OnGetStatCallback onFinished, void* userData);


    // Modifies the value of the specified stat.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_SetStat(const IStoveSetStatParam* param, OnSetStatCallback onFinished, void* userData);


    // Retrieves the achievement information for the given achievement ID.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchAchievement(const IStoveFetchAchievementParam* param, OnGetAchievementCallback onFinished, void* userData);


    // Retrieves all achievement information for the current user.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchAchievements(const IStoveFetchAchievementsParam* param, OnGetAchievementsCallback onFinished, void* userData);


    // Retrieves the rank information based on the provided parameters.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchRanking(const IStoveFetchRankingParam* rankParams, OnGetRankingCallback onFinished, void* userData);


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef void(__cdecl* Stove_FetchStat_t)(const IStoveFetchStatParam*, OnGetStatCallback, void*);
    typedef void(__cdecl* Stove_SetStat_t)(const IStoveSetStatParam*, OnSetStatCallback, void*);
    typedef void(__cdecl* Stove_FetchAchievement_t)(const IStoveFetchAchievementParam*, OnGetAchievementCallback, void*);
    typedef void(__cdecl* Stove_FetchAchievements_t)(const IStoveFetchAchievementsParam*, OnGetAchievementsCallback, void*);
    typedef void(__cdecl* Stove_FetchRanking_t)(const IStoveFetchRankingParam*, OnGetRankingCallback, void*);

#ifdef __cplusplus
}
#endif