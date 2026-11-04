/*
 * file: pcbang_api.h
 * Flat C API function declarations provided by PCBangSDK.
 */

#pragma once
#include "pcbang_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  PCBangSDK API
    // ============================================================


    // Logs in the signed-in game user to the PC bang service.
    // onUserLogin:      Callback delivering the initial login result
    // onRefreshBenefit: Callback delivering refreshed benefit info every 4 minutes
    //                   while the PC bang session is active
    // userData1:        User data passed to onUserLogin
    // userData2:        User data passed to onRefreshBenefit
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_PCBangLogin(const IStoveTypeBase* param,
                           OnPCBangLoginCallback onUserLogin, OnRefreshPCBangBenefitCallback onRefreshBenefit,
                           void* userData1, void* userData2);


    // Logs the game user out of the PC bang service.
    // Stops the periodic benefit-refresh callbacks started by Stove_PCBangLogin().
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_PCBangLogout(const IStoveTypeBase* param, OnPCBangLogoutCallback onFinished, void* userData);


    // Queries the current PC bang context and product entitlement of the user.
    // The result is delivered to the callback as an IStovePCBangStatus.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_PCBangCheckStatus(const IStoveTypeBase* param, OnPCBangCheckStatusCallback onFinished, void* userData);


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef void(__cdecl* Stove_PCBangLogin_t)(const IStoveTypeBase* param,
                                               OnPCBangLoginCallback onUserLogin, OnRefreshPCBangBenefitCallback onRefreshBenefit,
                                               void* userData1, void* userData2);
    typedef void(__cdecl* Stove_PCBangLogout_t)(const IStoveTypeBase* param, OnPCBangLogoutCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_PCBangCheckStatus_t)(const IStoveTypeBase* param, OnPCBangCheckStatusCallback onFinished, void* userData);

#ifdef __cplusplus
}
#endif