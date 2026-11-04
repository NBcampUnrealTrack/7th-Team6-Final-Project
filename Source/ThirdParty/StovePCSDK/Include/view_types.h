/*
 * file: view_types.h
 * Pure virtual interface and callback definitions used in ViewSDK.
 */

#pragma once
#include "view_misc.h"
#include "base_types.h"


 // ============================================================
 //  ViewSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveViewTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_VIEW_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveViewTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStoveSetPopupDisallowedParam
// ============================================================

// Parameters for Stove_SetPopupDisallowed().
// Create via Stove_CreateParam(k_EStoveViewTypeKind_SetPopupDisallowedParam),
// populate the fields via setters, then Destroy() after the call returns.
// Specifies which popup to suppress and for how many days.
struct IStoveSetPopupDisallowedParam : IStoveTypeBase
{
    STOVE_VIEW_TYPE_KIND(k_EStoveViewTypeKind_SetPopupDisallowedParam);

    // Popup identifier to suppress.
    virtual uint32_t GetPopupId() const = 0;
    virtual void SetPopupId(uint32_t popupId) = 0;

    // Duration, in days, for which the popup will be suppressed.
    virtual uint32_t GetDays() const = 0;
    virtual void SetDays(uint32_t days) = 0;
};


// ============================================================
//  IStovePopupParam
// ============================================================

// Parameters shared by Stove_AutoPopup() / Stove_NewsPopup() / Stove_CouponPopup().
// Create via Stove_CreateParam(k_EStoveViewTypeKind_PopupParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStovePopupParam : IStoveTypeBase
{
    STOVE_VIEW_TYPE_KIND(k_EStoveViewTypeKind_PopupParam);

    // Webview display mode (see EStoveWebViewMode: External / Internal).
    virtual int32_t GetWebViewMode() const = 0;
    virtual void SetWebViewMode(int32_t webViewMode) = 0;
};


// ============================================================
//  IStoveManualPopupParam
// ============================================================

// Parameters for Stove_ManualPopup().
// Create via Stove_CreateParam(k_EStoveViewTypeKind_ManualPopupParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveManualPopupParam : IStoveTypeBase
{
    STOVE_VIEW_TYPE_KIND(k_EStoveViewTypeKind_ManualPopupParam);

    // Webview display mode (see EStoveWebViewMode: External / Internal).
    virtual int32_t GetWebViewMode() const = 0;
    virtual void SetWebViewMode(int32_t webViewMode) = 0;

    // Resource key identifying the manual popup to show.
    virtual const wchar_t* GetResourceKey() const = 0;
    virtual void SetResourceKey(const wchar_t* resourceKey) = 0;
};


// ============================================================
//  IStoveVerifyIdentificationPopupParam
// ============================================================

// Parameters for Stove_VerifyIdentificationPopup().
// Create via Stove_CreateParam(k_EStoveViewTypeKind_VerifyIdentificationPopupParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveVerifyIdentificationPopupParam : IStoveTypeBase
{
    STOVE_VIEW_TYPE_KIND(k_EStoveViewTypeKind_VerifyIdentificationPopupParam);

    // Webview display mode (see EStoveWebViewMode: External / Internal).
    virtual int32_t GetWebViewMode() const = 0;
    virtual void SetWebViewMode(int32_t webViewMode) = 0;

    // Whether to compare the verified identifier against the signed-in user.
    virtual bool GetCompareIdentifier() const = 0;
    virtual void SetCompareIdentifier(bool compareIdentifier) = 0;
};

// ============================================================
//  IStoveVerifyIdentificationPopupDestroyInfo
// ============================================================

// Payload delivered when the verify-identification popup is destroyed.
// SDK-owned; valid only for the duration of the callback invocation —
// do NOT call Destroy() on it and do NOT retain pointers beyond the callback.
struct IStoveVerifyIdentificationPopupDestroyInfo : IStoveTypeBase
{
    STOVE_VIEW_TYPE_KIND(k_EStoveViewTypeKind_VerifyIdentificationPopupDestroyInfo);

    // SIM key issued after successful verification.
    // Returns an empty string ("") if verification failed or the key is unavailable.
    virtual const wchar_t* GetSimKey() const = 0;
};

#else

typedef struct IStoveSetPopupDisallowedParam IStoveSetPopupDisallowedParam;
typedef struct IStovePopupParam IStovePopupParam;
typedef struct IStoveManualPopupParam IStoveManualPopupParam;
typedef struct IStoveVerifyIdentificationPopupParam IStoveVerifyIdentificationPopupParam;
typedef struct IStoveVerifyIdentificationPopupDestroyInfo IStoveVerifyIdentificationPopupDestroyInfo;

#endif // __cplusplus


// ============================================================
//  ViewSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and payload pointers are owned by the SDK and are
// valid only for the duration of the callback invocation; do NOT call Destroy()
// on them and do NOT retain pointers beyond the callback.

// Delivered once the popup finishes showing / has produced a result
// (e.g., user dismissed, network fetch completed).
// 'popupInfo' tells whether this call ever created a webview
// (popupInfo->IsPopupCreated()), and is valid regardless of callbackResult's
// success/failure -- always read it. When false, the popup was never shown and
// onDestroy will NEVER be delivered for this call -- release any onDestroy-side
// callback state right here instead of waiting for onDestroy.
typedef void(__cdecl* OnViewPopupCallback)(const IStoveCallbackResult* callbackResult, const IStovePopupCreationInfo* popupInfo);

// Delivered when the popup's native resources have been fully destroyed.
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnViewPopupDestroyCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);

// Delivered by Stove_SetPopupDisallowed().
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnSetPopupDisallowedCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);

// Delivered when the verify-identification popup is destroyed.
// 'info' carries the SIM key issued after successful verification; access via info->GetSimKey().
typedef void(__cdecl* OnVerifyIdentificationPopupDestroyCallback)(const IStoveCallbackResult* callbackResult, const IStoveVerifyIdentificationPopupDestroyInfo* info);