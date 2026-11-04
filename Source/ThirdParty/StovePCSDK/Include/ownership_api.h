/*
 * file: ownership_api.h
 * Flat C API function declarations provided by OwnershipSDK.
 */

#pragma once
#include "ownership_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  OwnershipSDK API
    // ============================================================


    // Queries the ownership entries of the currently running game
    // (basic game, DLCs, demo, ...) for the signed-in user.
    // The array of IStoveOwnership* is delivered to the callback.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchOwnerships(const IStoveTypeBase* param, OnOwnershipListCallback onFinished, void* userData);


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef void(__cdecl* Stove_FetchOwnerships_t)(const IStoveTypeBase* param, OnOwnershipListCallback onFinished, void* userData);

#ifdef __cplusplus
}
#endif