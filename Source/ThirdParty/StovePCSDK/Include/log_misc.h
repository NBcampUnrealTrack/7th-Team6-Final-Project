/*
 * file: log_misc.h
 * Enum definitions used in LogSDK.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>


 // ============================================================
 //  LogSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance owned by LogSDK.
 // Used by Stove_CreateParam() to request a specific parameter object,
 // and by IStoveTypeBase::GetTypeKind() for runtime type identification.
 // Range (6000 ~ 6999)
typedef enum EStoveLogTypeKind
{
    k_EStoveLogTypeKind_Invalid = -1,

    // Parameter types (created by the caller via Stove_CreateParam)
    k_EStoveLogTypeKind_SendLogParam = 6500,

    k_EStoveLogTypeKind_Max = 0x7fffffff
} EStoveLogTypeKind;


// Identifies which LogSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 6500~6999 are an internal diagnostic band and are not part of this enum.
typedef enum EStoveLogMethodCode
{
    k_EStoveLogMethodCode_Invalid = -1,

    // Public APIs. Module block 6000~6999 (sequential).
    k_EStoveLogMethodCode_SendLog = 6000,

    k_EStoveLogMethodCode_Max = 0x7fffffff
} EStoveLogMethodCode;

// Result codes are global: common 0~299 (EStoveCommonResultCode),
// module-specific 300~ (EStoveResultCode) — both in base_misc.h.