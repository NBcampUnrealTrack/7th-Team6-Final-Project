/*
 * file: view_misc.h
 * Enum definitions used in ViewSDK.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>


 // ============================================================
 //  ViewSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance owned by ViewSDK.
 // Used by Stove_CreateParam() to request a specific parameter object,
 // and by IStoveTypeBase::GetTypeKind() for runtime type identification.
 // Range (1000 ~ 1999)
typedef enum EStoveViewTypeKind
{
    k_EStoveViewTypeKind_Invalid = -1,

    // Parameter types (created by the caller via Stove_CreateParam)
    k_EStoveViewTypeKind_SetPopupDisallowedParam = 1500,
    k_EStoveViewTypeKind_PopupParam = 1501,
    k_EStoveViewTypeKind_ManualPopupParam = 1502,
    k_EStoveViewTypeKind_VerifyIdentificationPopupParam = 1503,

    // Callback payload types (SDK-owned, delivered via callback)
    k_EStoveViewTypeKind_VerifyIdentificationPopupDestroyInfo = 1504,

    k_EStoveViewTypeKind_Max = 0x7fffffff
} EStoveViewTypeKind;


// Identifies which ViewSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 1500~1999 are an internal diagnostic band and are not part of this enum.
typedef enum EStoveViewMethodCode
{
    k_EStoveViewMethodCode_Invalid = -1,

    // Popup APIs. Module block 1000~1999 (sequential).
    k_EStoveViewMethodCode_AutoPopup = 1000,
    k_EStoveViewMethodCode_ManualPopup = 1001,
    k_EStoveViewMethodCode_NewsPopup = 1002,
    k_EStoveViewMethodCode_CouponPopup = 1003,
    k_EStoveViewMethodCode_VerifyIdentificationPopup = 1004,

    // Popup management / utility APIs
    k_EStoveViewMethodCode_SetPopupDisallowed = 1005,

    k_EStoveViewMethodCode_Max = 0x7fffffff
} EStoveViewMethodCode;

// Result codes are global: common 0~299 (EStoveCommonResultCode),
// module-specific 300~ (EStoveResultCode) — both in base_misc.h.