/*
 * file: base_api.h
 * Flat C API function declarations provided by BaseSDK.
 */

#pragma once
#include "base_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  BaseSDK API
    // ============================================================

    // Unified parameter-object factory for the whole SDK.
    // kind: An EStove*TypeKind value of ANY module (the module-specific TypeKind
    //       enums occupy disjoint integer ranges, so a single factory can
    //       dispatch them). Pass e.g. k_EStoveBaseTypeKind_InitializeParam,
    //       k_EStoveGameSupportTypeKind_FetchStatParam, k_EStoveIAPTypeKind_FetchProductsParam, ...
    // Only *Param kinds (each module's ...Param TypeKind) are valid here.
    // Passing a result/data kind (e.g. an Outcome/Info/List TypeKind) or an
    // unknown value is NOT an error but returns nullptr -- always null-check
    // the returned pointer before use.
    // Returns IStoveTypeBase* created object pointer (must call Destroy() after use),
    //         or nullptr if the kind is unknown.b
    // NOTE: After the single-binary (BaseSDK) consolidation this replaces the former
    //       per-module Stove_<Module>CreateParam() factories.
    IStoveTypeBase* STOVE_API Stove_CreateParam(int kind);


    // Checks whether PCSDK was launched through the Stove launcher.
    // If not, relaunches the app via the launcher and exits the current process.
    // Must be called before Stove_Initialize().
    // Asynchronous: the outcome is delivered to callback only while Stove_RunCallback()
    // (or Stove_RunCallbackWithTimeout()) is being pumped.
    void STOVE_API Stove_RestartAppIfNecessary(const IStoveRestartAppIfNecessaryParam* initParam, OnRestartAppIfNecessaryCallback onFinished, void* userData);


    // Initializes the BaseSDK (and, after the single-binary consolidation, the View/IAP modules).
    // BaseSDK reuses the env/gameId/appKey cached by Stove_RestartAppIfNecessary(); additionally,
    // if initParam carries a main window handle (SetMainWndHandle) the View module is initialized,
    // and if it also carries a shop key (SetShopKey) the IAP module is initialized.
    // initParam may be null to initialize BaseSDK only.
    //
    // On failure the SDK may be left partially initialized (e.g. BaseSDK and View up but IAP
    // failed). The result identifies the failing module via GetSDKName()/GetResultCode().
    // Recovery: call Stove_Uninitialize() and then retry Stove_Initialize() from a clean
    // state — re-calling Stove_Initialize() without Uninitialize first will report
    // ALREADY_INITIALIZED and will NOT re-run the failed module's setup.
    IStoveResult* STOVE_API Stove_Initialize(const IStoveInitializeParam* initParam);


    // Cleans up the BaseSDK.
    IStoveResult* STOVE_API Stove_Uninitialize();


    // Dispatches results (callback functions) for asynchronous calls.
    // Must be called on the game's UI (main) thread.
    // All asynchronous API callbacks across the SDK are delivered through this pump
    // (or Stove_RunCallbackWithTimeout()) -- call it periodically to receive them.
    void STOVE_API Stove_RunCallback();


    // Timeout version of Stove_RunCallback.
    void STOVE_API Stove_RunCallbackWithTimeout(uint32_t timeoutMillisec);


    // Retrieves the currently valid AccessToken.
    // The SDK automatically refreshes the token internally to provide a valid one.
    IStoveResult* STOVE_API Stove_GetAccessToken(wchar_t* outAccessToken, uint32_t length);


    // Delivers the newly issued token via callback when the AccessToken is refreshed.
    // Use when work is needed at the moment the token is renewed.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_AccessTokenRenewed(const IStoveTypeBase* param, OnAccessTokenRenewedCallback onFinished, void* userData);


    // Retrieves information about the signed-in user.
    // outUser: Variable to receive the user info pointer (must call Destroy() after use)
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_GetUser(IStoveUser** outUser);


    // Retrieves the GDS (country/timezone) info of the signed-in user.
    // outGds: Variable to receive the GDS info pointer (must call Destroy() after use)
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_GetGds(IStoveGds** outGds);


    // Retrieves the Signin info of the signed-in user.
    // outSignin: Variable to receive the Signin info pointer (must call Destroy() after use)
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_GetSignin(IStoveSignin** outSignin);


    // Retrieves the cloud save path.
    // outPath: Buffer to receive the cloud save path
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_GetCloudSavingPath(wchar_t* outPath, uint32_t length);


    // Sets the PCSDK language.
    // language: Language info string
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_SetLanguage(const wchar_t* language);


    // Sets the game profile.
    // gameProfileParams: Game profile info (IStoveSetGameProfileParam)
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_SetGameProfile(const IStoveSetGameProfileParam* gameProfileParams);


    // Delivers hourly notifications to users subject to Korean over-immersion prevention.
    // Korea-only API.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_OverImmersionNotification(const IStoveTypeBase* param, OnOverImmersionNotificationCallback onFinished, void* userData);


    // Delivers shutdown notifications to users subject to shutdown.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_ShutdownNotification(const IStoveTypeBase* param, OnShutdownNotificationCallback onFinished, void* userData);


    // Delivers the Vietnam age rating notice via callback.
    // Vietnam-only API.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_VietnamAgeRatingNotification(const IStoveTypeBase* param, OnVietnamAgeRatingNotificationCallback onFinished, void* userData);


    // Delivers the Vietnam over-immersion notification via callback.
    // Vietnam-only API.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_VietnamOverimmersionNotification(const IStoveTypeBase* param, OnVietnamOverimmersionNotificationCallback onFinished, void* userData);


    // Opens a URL in an external browser.
    // Performs SSO handling when opening Onstove-related domains. (e.g., *.onstove.com")
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_OpenExternalUrl(const wchar_t* url, OnOpenExternalUrlCallback onFinished, void* userData);


    // Retrieves the BaseSDK version info.
    // outVersion: Buffer to receive the version string
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_GetVersion(wchar_t* outVersion, uint32_t length);


    // Closes all popups opened by the SDK (both IAP and View popups).
    // [Single-binary consolidation] Replaces the former per-module
    // Stove_IAP_CloseAllPopups() / Stove_View_CloseAllPopups(): now a single call
    // dismisses every SDK popup together.
    // Returns IStoveResult* function call result (must call Destroy() after use)
    IStoveResult* STOVE_API Stove_CloseAllPopups();


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef IStoveTypeBase* (__cdecl* Stove_CreateParam_t)(int kind);
    typedef void(__cdecl* Stove_RestartAppIfNecessary_t)(const IStoveRestartAppIfNecessaryParam* initParam, OnRestartAppIfNecessaryCallback onFinished, void* userData);
    typedef IStoveResult* (__cdecl* Stove_Initialize_t)(const IStoveInitializeParam* initParam);
    typedef IStoveResult* (__cdecl* Stove_Uninitialize_t)();

    typedef void(__cdecl* Stove_RunCallback_t)();
    typedef void(__cdecl* Stove_RunCallbackWithTimeout_t)(uint32_t timeoutMillisec);

    typedef IStoveResult* (__cdecl* Stove_GetAccessToken_t)(wchar_t* outAccessToken, uint32_t length);
    typedef void(__cdecl* Stove_AccessTokenRenewed_t)(const IStoveTypeBase* param, OnAccessTokenRenewedCallback onFinished, void* userData);

    typedef IStoveResult* (__cdecl* Stove_GetUser_t)(IStoveUser** outUser);
    typedef IStoveResult* (__cdecl* Stove_GetGds_t)(IStoveGds** outGds);
    typedef IStoveResult* (__cdecl* Stove_GetSignin_t)(IStoveSignin** outSignin);
    typedef IStoveResult* (__cdecl* Stove_GetCloudSavingPath_t)(wchar_t* outPath, uint32_t length);

    typedef IStoveResult* (__cdecl* Stove_SetLanguage_t)(const wchar_t* language);
    typedef IStoveResult* (__cdecl* Stove_SetGameProfile_t)(const IStoveSetGameProfileParam* gameProfileParams);

    typedef void(__cdecl* Stove_OverImmersionNotification_t)(const IStoveTypeBase* param, OnOverImmersionNotificationCallback onFinished, void* userData);
    typedef void(__cdecl* Stove_ShutdownNotification_t)(const IStoveTypeBase* param, OnShutdownNotificationCallback onFinished, void* userData);
    typedef void(__cdecl* Stove_VietnamAgeRatingNotification_t)(const IStoveTypeBase* param, OnVietnamAgeRatingNotificationCallback onFinished, void* userData);
    typedef void(__cdecl* Stove_VietnamOverimmersionNotification_t)(const IStoveTypeBase* param, OnVietnamOverimmersionNotificationCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_OpenExternalUrl_t)(const wchar_t* url, OnOpenExternalUrlCallback onFinished, void* userData);
    typedef IStoveResult* (__cdecl* Stove_GetVersion_t)(wchar_t* outVersion, uint32_t length);

    typedef IStoveResult* (__cdecl* Stove_CloseAllPopups_t)();

#ifdef __cplusplus
}
#endif