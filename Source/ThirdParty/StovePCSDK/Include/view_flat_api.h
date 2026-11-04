/*
 * file: view_flat_api.h
 * Flat C accessor function declarations for ViewSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 *           properties from C and C# environments where C++ virtual calls
 *           (vtable dispatch) are not directly available.
 *
 *           Each accessor follows the naming pattern
 *               Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *           and forwards the call to the corresponding virtual method on the
 *           interface defined in view_types.h.
 *
 *           For the semantics, units, and valid ranges of each field,
 *           refer to the matching interface definition in view_types.h —
 *           this file intentionally does not duplicate that documentation
 *           to keep the accessor list readable.
 */

#pragma once
#include "view_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveSetPopupDisallowedParam  —  Popup-suppression parameters
    // ============================================================
    uint32_t STOVE_API Stove_IStoveSetPopupDisallowedParam_GetPopupId(const IStoveSetPopupDisallowedParam* self);
    void STOVE_API Stove_IStoveSetPopupDisallowedParam_SetPopupId(IStoveSetPopupDisallowedParam* self, uint32_t popupId); // Non-const

    uint32_t STOVE_API Stove_IStoveSetPopupDisallowedParam_GetDays(const IStoveSetPopupDisallowedParam* self);
    void STOVE_API Stove_IStoveSetPopupDisallowedParam_SetDays(IStoveSetPopupDisallowedParam* self, uint32_t days); // Non-const

    // ============================================================
    //  IStovePopupParam  —  Auto/News/Coupon popup parameters
    // ============================================================
    int32_t STOVE_API Stove_IStovePopupParam_GetWebViewMode(const IStovePopupParam* self);
    void STOVE_API Stove_IStovePopupParam_SetWebViewMode(IStovePopupParam* self, int32_t webViewMode); // Non-const

    // ============================================================
    //  IStoveManualPopupParam  —  Manual popup parameters
    // ============================================================
    int32_t STOVE_API Stove_IStoveManualPopupParam_GetWebViewMode(const IStoveManualPopupParam* self);
    void STOVE_API Stove_IStoveManualPopupParam_SetWebViewMode(IStoveManualPopupParam* self, int32_t webViewMode); // Non-const

    const wchar_t* STOVE_API Stove_IStoveManualPopupParam_GetResourceKey(const IStoveManualPopupParam* self);
    void STOVE_API Stove_IStoveManualPopupParam_SetResourceKey(IStoveManualPopupParam* self, const wchar_t* resourceKey); // Non-const

    // ============================================================
    //  IStoveVerifyIdentificationPopupParam  —  Verify-identification popup parameters
    // ============================================================
    int32_t STOVE_API Stove_IStoveVerifyIdentificationPopupParam_GetWebViewMode(const IStoveVerifyIdentificationPopupParam* self);
    void STOVE_API Stove_IStoveVerifyIdentificationPopupParam_SetWebViewMode(IStoveVerifyIdentificationPopupParam* self, int32_t webViewMode); // Non-const

    bool STOVE_API Stove_IStoveVerifyIdentificationPopupParam_GetCompareIdentifier(const IStoveVerifyIdentificationPopupParam* self);
    void STOVE_API Stove_IStoveVerifyIdentificationPopupParam_SetCompareIdentifier(IStoveVerifyIdentificationPopupParam* self, bool compareIdentifier); // Non-const

    // ============================================================
    //  IStoveVerifyIdentificationPopupDestroyInfo  —  Destroy-info payload
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveVerifyIdentificationPopupDestroyInfo_GetSimKey(const IStoveVerifyIdentificationPopupDestroyInfo* self);


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveSetPopupDisallowedParam =====
    typedef uint32_t(__cdecl* Stove_IStoveSetPopupDisallowedParam_GetPopupId_t)(const IStoveSetPopupDisallowedParam* self);
    typedef void(__cdecl* Stove_IStoveSetPopupDisallowedParam_SetPopupId_t)(IStoveSetPopupDisallowedParam* self, uint32_t popupId); // Non-const

    typedef uint32_t(__cdecl* Stove_IStoveSetPopupDisallowedParam_GetDays_t)(const IStoveSetPopupDisallowedParam* self);
    typedef void(__cdecl* Stove_IStoveSetPopupDisallowedParam_SetDays_t)(IStoveSetPopupDisallowedParam* self, uint32_t days); // Non-const

    // ===== IStovePopupParam =====
    typedef int32_t(__cdecl* Stove_IStovePopupParam_GetWebViewMode_t)(const IStovePopupParam* self);
    typedef void(__cdecl* Stove_IStovePopupParam_SetWebViewMode_t)(IStovePopupParam* self, int32_t webViewMode); // Non-const

    // ===== IStoveManualPopupParam =====
    typedef int32_t(__cdecl* Stove_IStoveManualPopupParam_GetWebViewMode_t)(const IStoveManualPopupParam* self);
    typedef void(__cdecl* Stove_IStoveManualPopupParam_SetWebViewMode_t)(IStoveManualPopupParam* self, int32_t webViewMode); // Non-const

    typedef const wchar_t* (__cdecl* Stove_IStoveManualPopupParam_GetResourceKey_t)(const IStoveManualPopupParam* self);
    typedef void(__cdecl* Stove_IStoveManualPopupParam_SetResourceKey_t)(IStoveManualPopupParam* self, const wchar_t* resourceKey); // Non-const

    // ===== IStoveVerifyIdentificationPopupParam =====
    typedef int32_t(__cdecl* Stove_IStoveVerifyIdentificationPopupParam_GetWebViewMode_t)(const IStoveVerifyIdentificationPopupParam* self);
    typedef void(__cdecl* Stove_IStoveVerifyIdentificationPopupParam_SetWebViewMode_t)(IStoveVerifyIdentificationPopupParam* self, int32_t webViewMode); // Non-const

    typedef bool(__cdecl* Stove_IStoveVerifyIdentificationPopupParam_GetCompareIdentifier_t)(const IStoveVerifyIdentificationPopupParam* self);
    typedef void(__cdecl* Stove_IStoveVerifyIdentificationPopupParam_SetCompareIdentifier_t)(IStoveVerifyIdentificationPopupParam* self, bool compareIdentifier); // Non-const

    // ===== IStoveVerifyIdentificationPopupDestroyInfo =====
    typedef const wchar_t* (__cdecl* Stove_IStoveVerifyIdentificationPopupDestroyInfo_GetSimKey_t)(const IStoveVerifyIdentificationPopupDestroyInfo* self);

#ifdef __cplusplus
}
#endif