/*
 * file: base_flat_api.h
 * Flat C accessor function declarations for BaseSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 * properties from C and C# environments where C++ virtual calls
 * (vtable dispatch) are not directly available.
 *
 * Each accessor follows the naming pattern
 * Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 * and forwards the call to the corresponding virtual method on the
 * interface defined in base_types.h.
 *
 * For the semantics, units, and valid ranges of each field,
 * refer to the matching interface definition in base_types.h —
 * this file intentionally does not duplicate that documentation
 * to keep the accessor list readable.
 */

#pragma once
#include "base_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveTypeBase  —  Root interface (type identification, destruction)
    // ============================================================
    int32_t STOVE_API Stove_IStoveTypeBase_GetTypeKind(const IStoveTypeBase* self);
    bool STOVE_API Stove_IStoveTypeBase_ShouldDestroy(const IStoveTypeBase* self);
    void STOVE_API Stove_IStoveTypeBase_Destroy(IStoveTypeBase* self); // Non-const
    void* STOVE_API Stove_IStoveTypeBase_QueryExt(const IStoveTypeBase* self, uint32_t extId);

    // ============================================================
    //  IStoveResult  —  Synchronous BaseSDK call result
    // ============================================================
    uint32_t STOVE_API Stove_IStoveResult_GetMethodCode(const IStoveResult* self);
    uint32_t STOVE_API Stove_IStoveResult_GetResultCode(const IStoveResult* self);
    bool STOVE_API Stove_IStoveResult_IsSuccessful(const IStoveResult* self);

    // ============================================================
    //  IStoveCallbackResult  —  Result delivered to async callbacks
    // ============================================================
    IStoveResult* STOVE_API Stove_IStoveCallbackResult_GetResult(const IStoveCallbackResult* self);
    const wchar_t* STOVE_API Stove_IStoveCallbackResult_GetErrorMessage(const IStoveCallbackResult* self);
    int32_t STOVE_API Stove_IStoveCallbackResult_GetExternalError(const IStoveCallbackResult* self);
    void* STOVE_API Stove_IStoveCallbackResult_GetUserData(const IStoveCallbackResult* self);

    // ============================================================
    //  IStovePopupCreationInfo  —  Shared "was a popup created" onFinished payload
    // ============================================================
    bool STOVE_API Stove_IStovePopupCreationInfo_IsPopupCreated(const IStovePopupCreationInfo* self);

    // ============================================================
    //  IStoveRestartAppIfNecessaryParam  —  Stove_RestartAppIfNecessary() parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetEnvironment(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetEnvironment(IStoveRestartAppIfNecessaryParam* self, const wchar_t* environment); // Non-const

    const wchar_t* STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetGameId(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetGameId(IStoveRestartAppIfNecessaryParam* self, const wchar_t* gameId); // Non-const

    const wchar_t* STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetAppKey(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetAppKey(IStoveRestartAppIfNecessaryParam* self, const wchar_t* appKey); // Non-const

    uint32_t STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetWaitTimeMilliSec(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetWaitTimeMilliSec(IStoveRestartAppIfNecessaryParam* self, uint32_t ms); // Non-const

    bool STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetLaunchStoveLauncher(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetLaunchStoveLauncher(IStoveRestartAppIfNecessaryParam* self, bool launchStoveLauncher); // Non-const

    const wchar_t* STOVE_API Stove_IStoveRestartAppIfNecessaryParam_GetPlatformName(const IStoveRestartAppIfNecessaryParam* self);
    void STOVE_API Stove_IStoveRestartAppIfNecessaryParam_SetPlatformName(IStoveRestartAppIfNecessaryParam* self, const wchar_t* platformName); // Non-const

    // ============================================================
    //  IStoveInitializeParam  —  Initialization parameters (get/set pairs adjacent)
    // ============================================================

    // [Single-binary consolidation] IAP shop key / main window handle, consumed by Stove_Initialize().
    const wchar_t* STOVE_API Stove_IStoveInitializeParam_GetShopKey(const IStoveInitializeParam* self);
    void STOVE_API Stove_IStoveInitializeParam_SetShopKey(IStoveInitializeParam* self, const wchar_t* shopKey); // Non-const

    const void* STOVE_API Stove_IStoveInitializeParam_GetMainWndHandle(const IStoveInitializeParam* self);
    void STOVE_API Stove_IStoveInitializeParam_SetMainWndHandle(IStoveInitializeParam* self, const void* mainWndHandle); // Non-const

    // ============================================================
    //  IStoveAccessToken  —  Access token delivered on token renewal
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveAccessToken_GetAccessToken(const IStoveAccessToken* self);
    int32_t STOVE_API Stove_IStoveAccessToken_GetExpireIn(const IStoveAccessToken* self);

    // ============================================================
    //  IStoveUser  —  Signed-in user profile
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveUser_GetNickName(const IStoveUser* self);
    uint64_t STOVE_API Stove_IStoveUser_GetUserId(const IStoveUser* self);

    // ============================================================
    //  IStoveGds  —  Country / regulation / timezone / language info
    // ============================================================
    bool STOVE_API Stove_IStoveGds_IsDefault(const IStoveGds* self);
    const wchar_t* STOVE_API Stove_IStoveGds_GetNation(const IStoveGds* self);
    const wchar_t* STOVE_API Stove_IStoveGds_GetRegulation(const IStoveGds* self);
    const wchar_t* STOVE_API Stove_IStoveGds_GetTimezone(const IStoveGds* self);
    int32_t STOVE_API Stove_IStoveGds_GetUtcOffset(const IStoveGds* self);
    const wchar_t* STOVE_API Stove_IStoveGds_GetLang(const IStoveGds* self);

    // ============================================================
    //  IStoveSignin  —  Sign-in verification flags and IDP info
    // ============================================================
    bool STOVE_API Stove_IStoveSignin_IsPersonVerified(const IStoveSignin* self);
    bool STOVE_API Stove_IStoveSignin_IsEmailVerified(const IStoveSignin* self);
    const wchar_t* STOVE_API Stove_IStoveSignin_GetRegisteredCountryCode(const IStoveSignin* self);
    const wchar_t* STOVE_API Stove_IStoveSignin_GetProviderCode(const IStoveSignin* self);
    int32_t STOVE_API Stove_IStoveSignin_GetAccountType(const IStoveSignin* self);

    // ============================================================
    //  IStoveOverImmersionInfo  —  Korean over-immersion notification
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveOverImmersionInfo_GetMsg(const IStoveOverImmersionInfo* self);
    int32_t STOVE_API Stove_IStoveOverImmersionInfo_GetElapsedHours(const IStoveOverImmersionInfo* self); // hours
    int32_t STOVE_API Stove_IStoveOverImmersionInfo_GetExposureTime(const IStoveOverImmersionInfo* self);

    // ============================================================
    //  IStoveVietnamAgeRatingInfo  —  Vietnam age-rating overlay info
    // ============================================================
    int32_t STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetOverlayMode(const IStoveVietnamAgeRatingInfo* self);
    int32_t STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetOverlayType(const IStoveVietnamAgeRatingInfo* self);
    float STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetOverlayScale(const IStoveVietnamAgeRatingInfo* self);
    float STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetOverlayOpacity(const IStoveVietnamAgeRatingInfo* self);
    int32_t STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetAgeRating(const IStoveVietnamAgeRatingInfo* self);
    const wchar_t* STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetMsg(const IStoveVietnamAgeRatingInfo* self);
    float STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetDisplayPositionX(const IStoveVietnamAgeRatingInfo* self);
    float STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetDisplayPositionY(const IStoveVietnamAgeRatingInfo* self);
    const wchar_t* STOVE_API Stove_IStoveVietnamAgeRatingInfo_GetLanguage(const IStoveVietnamAgeRatingInfo* self);

    // ============================================================
    //  IStoveVietnamOverimmersionInfo  —  Vietnam over-immersion overlay info
    // ============================================================
    int32_t STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetOverlayMode(const IStoveVietnamOverimmersionInfo* self);
    int32_t STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetOverlayType(const IStoveVietnamOverimmersionInfo* self);
    float STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetOverlayScale(const IStoveVietnamOverimmersionInfo* self);
    float STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetOverlayOpacity(const IStoveVietnamOverimmersionInfo* self);
    int32_t STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetAgeRating(const IStoveVietnamOverimmersionInfo* self);
    const wchar_t* STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetMsg(const IStoveVietnamOverimmersionInfo* self);
    const wchar_t* STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetStyledMsg(const IStoveVietnamOverimmersionInfo* self);
    int32_t STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetElapsedMinutes(const IStoveVietnamOverimmersionInfo* self); // minutes
    int32_t STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetExposureTime(const IStoveVietnamOverimmersionInfo* self);
    float STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetExpandAnimationTime(const IStoveVietnamOverimmersionInfo* self);
    float STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetDisplayPositionX(const IStoveVietnamOverimmersionInfo* self);
    float STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetDisplayPositionY(const IStoveVietnamOverimmersionInfo* self);
    const wchar_t* STOVE_API Stove_IStoveVietnamOverimmersionInfo_GetLanguage(const IStoveVietnamOverimmersionInfo* self);

    // ============================================================
    //  IStoveShutdownInfo  —  Shutdown notification
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveShutdownInfo_GetMsg(const IStoveShutdownInfo* self);
    int32_t STOVE_API Stove_IStoveShutdownInfo_GetExposureTime(const IStoveShutdownInfo* self);
    int32_t STOVE_API Stove_IStoveShutdownInfo_GetInadvanceMinutes(const IStoveShutdownInfo* self);

    // ============================================================
    //  IStoveRestartAppIfNecessaryOutcome  —  Outcome of Stove_RestartAppIfNecessary()
    // ============================================================
    bool STOVE_API Stove_IStoveRestartAppIfNecessaryOutcome_IsRestartRequired(const IStoveRestartAppIfNecessaryOutcome* self);

    // ============================================================
    //  IStoveSetGameProfileParam  —  Game profile parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveSetGameProfileParam_GetWorldId(const IStoveSetGameProfileParam* self);
    void STOVE_API Stove_IStoveSetGameProfileParam_SetWorldId(IStoveSetGameProfileParam* self, const wchar_t* worldId); // Non-const

    int64_t STOVE_API Stove_IStoveSetGameProfileParam_GetCharacterNo(const IStoveSetGameProfileParam* self);
    void STOVE_API Stove_IStoveSetGameProfileParam_SetCharacterNo(IStoveSetGameProfileParam* self, int64_t characterNo); // Non-const


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveTypeBase =====
    typedef int32_t(__cdecl* Stove_IStoveTypeBase_GetTypeKind_t)(const IStoveTypeBase* self);
    typedef bool(__cdecl* Stove_IStoveTypeBase_ShouldDestroy_t)(const IStoveTypeBase* self);
    typedef void(__cdecl* Stove_IStoveTypeBase_Destroy_t)(IStoveTypeBase* self); // Non-const
    typedef void* (__cdecl* Stove_IStoveTypeBase_QueryExt_t)(const IStoveTypeBase* self, uint32_t extId);

    // ===== IStoveResult =====
    typedef uint32_t(__cdecl* Stove_IStoveResult_GetMethodCode_t)(const IStoveResult* self);
    typedef uint32_t(__cdecl* Stove_IStoveResult_GetResultCode_t)(const IStoveResult* self);
    typedef bool(__cdecl* Stove_IStoveResult_IsSuccessful_t)(const IStoveResult* self);

    // ===== IStovePopupCreationInfo =====
    typedef bool(__cdecl* Stove_IStovePopupCreationInfo_IsPopupCreated_t)(const IStovePopupCreationInfo* self);

    // ===== IStoveCallbackResult =====
    typedef IStoveResult* (__cdecl* Stove_IStoveCallbackResult_GetResult_t)(const IStoveCallbackResult* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveCallbackResult_GetErrorMessage_t)(const IStoveCallbackResult* self);
    typedef int32_t(__cdecl* Stove_IStoveCallbackResult_GetExternalError_t)(const IStoveCallbackResult* self);
    typedef void* (__cdecl* Stove_IStoveCallbackResult_GetUserData_t)(const IStoveCallbackResult* self);

    // ===== IStoveRestartAppIfNecessaryParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetEnvironment_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetEnvironment_t)(IStoveRestartAppIfNecessaryParam* self, const wchar_t* environment); // Non-const

    typedef const wchar_t* (__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetGameId_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetGameId_t)(IStoveRestartAppIfNecessaryParam* self, const wchar_t* gameId); // Non-const

    typedef const wchar_t* (__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetAppKey_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetAppKey_t)(IStoveRestartAppIfNecessaryParam* self, const wchar_t* appKey); // Non-const

    typedef uint32_t(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetWaitTimeMilliSec_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetWaitTimeMilliSec_t)(IStoveRestartAppIfNecessaryParam* self, uint32_t ms); // Non-const

    typedef bool(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetLaunchStoveLauncher_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetLaunchStoveLauncher_t)(IStoveRestartAppIfNecessaryParam* self, bool launchStoveLauncher); // Non-const

    typedef const wchar_t* (__cdecl* Stove_IStoveRestartAppIfNecessaryParam_GetPlatformName_t)(const IStoveRestartAppIfNecessaryParam* self);
    typedef void(__cdecl* Stove_IStoveRestartAppIfNecessaryParam_SetPlatformName_t)(IStoveRestartAppIfNecessaryParam* self, const wchar_t* platformName); // Non-const

    // ===== IStoveInitializeParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveInitializeParam_GetShopKey_t)(const IStoveInitializeParam* self);
    typedef void(__cdecl* Stove_IStoveInitializeParam_SetShopKey_t)(IStoveInitializeParam* self, const wchar_t* shopKey); // Non-const

    typedef const void* (__cdecl* Stove_IStoveInitializeParam_GetMainWndHandle_t)(const IStoveInitializeParam* self);
    typedef void(__cdecl* Stove_IStoveInitializeParam_SetMainWndHandle_t)(IStoveInitializeParam* self, const void* mainWndHandle); // Non-const

    // ===== IStoveAccessToken =====
    typedef const wchar_t* (__cdecl* Stove_IStoveAccessToken_GetAccessToken_t)(const IStoveAccessToken* self);
    typedef int32_t(__cdecl* Stove_IStoveAccessToken_GetExpireIn_t)(const IStoveAccessToken* self);

    // ===== IStoveUser =====
    typedef const wchar_t* (__cdecl* Stove_IStoveUser_GetNickName_t)(const IStoveUser* self);
    typedef uint64_t(__cdecl* Stove_IStoveUser_GetUserId_t)(const IStoveUser* self);

    // ===== IStoveGds =====
    typedef bool(__cdecl* Stove_IStoveGds_IsDefault_t)(const IStoveGds* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveGds_GetNation_t)(const IStoveGds* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveGds_GetRegulation_t)(const IStoveGds* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveGds_GetTimezone_t)(const IStoveGds* self);
    typedef int32_t(__cdecl* Stove_IStoveGds_GetUtcOffset_t)(const IStoveGds* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveGds_GetLang_t)(const IStoveGds* self);

    // ===== IStoveSignin =====
    typedef bool(__cdecl* Stove_IStoveSignin_IsPersonVerified_t)(const IStoveSignin* self);
    typedef bool(__cdecl* Stove_IStoveSignin_IsEmailVerified_t)(const IStoveSignin* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveSignin_GetRegisteredCountryCode_t)(const IStoveSignin* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveSignin_GetProviderCode_t)(const IStoveSignin* self);
    typedef int32_t(__cdecl* Stove_IStoveSignin_GetAccountType_t)(const IStoveSignin* self);

    // ===== IStoveOverImmersionInfo =====
    typedef const wchar_t* (__cdecl* Stove_IStoveOverImmersionInfo_GetMsg_t)(const IStoveOverImmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveOverImmersionInfo_GetElapsedHours_t)(const IStoveOverImmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveOverImmersionInfo_GetExposureTime_t)(const IStoveOverImmersionInfo* self);

    // ===== IStoveVietnamAgeRatingInfo =====
    typedef int32_t(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetOverlayMode_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetOverlayType_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetOverlayScale_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetOverlayOpacity_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetAgeRating_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetMsg_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetDisplayPositionX_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetDisplayPositionY_t)(const IStoveVietnamAgeRatingInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveVietnamAgeRatingInfo_GetLanguage_t)(const IStoveVietnamAgeRatingInfo* self);

    // ===== IStoveVietnamOverimmersionInfo =====
    typedef int32_t(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetOverlayMode_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetOverlayType_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetOverlayScale_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetOverlayOpacity_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetAgeRating_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetMsg_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetStyledMsg_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetElapsedMinutes_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetExposureTime_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetExpandAnimationTime_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetDisplayPositionX_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef float(__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetDisplayPositionY_t)(const IStoveVietnamOverimmersionInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveVietnamOverimmersionInfo_GetLanguage_t)(const IStoveVietnamOverimmersionInfo* self);

    // ===== IStoveShutdownInfo =====
    typedef const wchar_t* (__cdecl* Stove_IStoveShutdownInfo_GetMsg_t)(const IStoveShutdownInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveShutdownInfo_GetExposureTime_t)(const IStoveShutdownInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveShutdownInfo_GetInadvanceMinutes_t)(const IStoveShutdownInfo* self);

    // ===== IStoveRestartAppIfNecessaryOutcome =====
    typedef bool(__cdecl* Stove_IStoveRestartAppIfNecessaryOutcome_IsRestartRequired_t)(const IStoveRestartAppIfNecessaryOutcome* self);

    // ===== IStoveSetGameProfileParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveSetGameProfileParam_GetWorldId_t)(const IStoveSetGameProfileParam* self);
    typedef void(__cdecl* Stove_IStoveSetGameProfileParam_SetWorldId_t)(IStoveSetGameProfileParam* self, const wchar_t* worldId); // Non-const

    typedef int64_t(__cdecl* Stove_IStoveSetGameProfileParam_GetCharacterNo_t)(const IStoveSetGameProfileParam* self);
    typedef void(__cdecl* Stove_IStoveSetGameProfileParam_SetCharacterNo_t)(IStoveSetGameProfileParam* self, int64_t characterNo); // Non-const

#ifdef __cplusplus
}
#endif