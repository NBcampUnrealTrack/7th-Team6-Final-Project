/*
 * file: view_api.h
 * Flat C API function declarations provided by ViewSDK.
 */

#pragma once
#include "view_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  ViewSDK API
    // ============================================================


    // Displays an auto popup based on the provided parameters.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_AutoPopup(const IStovePopupParam* param,
                             OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                             void* userData1, void* userData2);


    // Displays a manual popup based on the provided parameters.
    // Consults IStoveManualPopupParam::GetResourceKey() to select the popup content.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_ManualPopup(const IStoveManualPopupParam* param,
                               OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                               void* userData1, void* userData2);


    // Displays a news popup based on the provided parameters.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_NewsPopup(const IStovePopupParam* param,
                             OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                             void* userData1, void* userData2);


    // Displays a coupon popup based on the provided parameters.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_CouponPopup(const IStovePopupParam* param,
                               OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                               void* userData1, void* userData2);


    // Displays a verify-identification popup based on the provided parameters.
    // Consults IStoveVerifyIdentificationPopupParam::GetCompareIdentifier() to decide whether
    // to compare the verified identifier against the signed-in user.
    // On success, onDestroy receives the issued SIM key.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_VerifyIdentificationPopup(const IStoveVerifyIdentificationPopupParam* param,
                                             OnViewPopupCallback onFinished,
                                             OnVerifyIdentificationPopupDestroyCallback onDestroy,
                                             void* userData1, void* userData2);


    // Suppresses (hides) the specified popup for a given number of days.
    // disallowed: Popup-id / duration parameters (IStoveSetPopupDisallowedParam)
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_SetPopupDisallowed(const IStoveSetPopupDisallowedParam* disallowed,
                                      OnSetPopupDisallowedCallback onFinished, void* userData);


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    // Popups
    typedef void(__cdecl* Stove_AutoPopup_t)(const IStovePopupParam* param,
                                                 OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                                                 void* userData1, void* userData2);

    typedef void(__cdecl* Stove_ManualPopup_t)(const IStoveManualPopupParam* param,
                                                   OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                                                   void* userData1, void* userData2);

    typedef void(__cdecl* Stove_NewsPopup_t)(const IStovePopupParam* param,
                                                 OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                                                 void* userData1, void* userData2);

    typedef void(__cdecl* Stove_CouponPopup_t)(const IStovePopupParam* param,
                                                   OnViewPopupCallback onFinished, OnViewPopupDestroyCallback onDestroy,
                                                   void* userData1, void* userData2);

    typedef void(__cdecl* Stove_VerifyIdentificationPopup_t)(const IStoveVerifyIdentificationPopupParam* param,
                                                                 OnViewPopupCallback onFinished,
                                                                 OnVerifyIdentificationPopupDestroyCallback onDestroy,
                                                                 void* userData1, void* userData2);

    // Popup management
    typedef void(__cdecl* Stove_SetPopupDisallowed_t)(const IStoveSetPopupDisallowedParam* disallowed,
                                                          OnSetPopupDisallowedCallback onFinished, void* userData);

#ifdef __cplusplus
}
#endif