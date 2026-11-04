/*
 * file: pcbang_misc.h
 * Enum definitions used in PCBangSDK.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>


 // ============================================================
 //  PCBangSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance owned by PCBangSDK.
 // Used by IStoveTypeBase::GetTypeKind() for runtime type identification of
 // result/data objects delivered through PCBang async callbacks.
 // Range (3000 ~ 3999)
typedef enum EStovePCBangTypeKind
{
    k_EStovePCBangTypeKind_Invalid = -1,

    // Result / Data types (delivered by the SDK via callbacks)
    k_EStovePCBangTypeKind_StovePCBangLoginOutcome = 3000,
    k_EStovePCBangTypeKind_StovePCBangBenefitInfo = 3001,
    k_EStovePCBangTypeKind_StovePCBangStatus = 3002,

    k_EStovePCBangTypeKind_Max = 0x7fffffff
} EStovePCBangTypeKind;


// Identifies which PCBangSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 3500~3999 are an internal diagnostic band and are not part of this enum.
typedef enum EStovePCBangMethodCode
{
    k_EStovePCBangMethodCode_Invalid = -1,

    // Public APIs. Module block 3000~3999 (sequential).
    k_EStovePCBangMethodCode_Login = 3000,
    k_EStovePCBangMethodCode_Logout = 3001,
    k_EStovePCBangMethodCode_CheckStatus = 3002,
    k_EStovePCBangMethodCode_RefreshBenefit = 3003,

    k_EStovePCBangMethodCode_Max = 0x7fffffff
} EStovePCBangMethodCode;


// PC bang premium state of the signed-in user.
// Returned by IStovePCBangLoginOutcome::GetPremiumCheck(),
// IStovePCBangBenefitInfo::GetPremiumCheck(),
// and IStovePCBangStatus::GetPremiumCheck().
typedef enum EStovePCBangPremium
{
    k_EStovePCBangPremium_Error = -1,        // Server error / indeterminate state
    k_EStovePCBangPremium_Premium = 1,       // Premium (paid) PC bang benefits available
    k_EStovePCBangPremium_Free = 2,          // Free tier
    k_EStovePCBangPremium_FreeOther = 3,     // Free tier, served by a partner/other provider

    k_EStovePCBangPremium_Max = 0x7fffffff
} EStovePCBangPremium;