/*
 * file: pcbang_types.h
 * Pure virtual interface and callback definitions used in PCBangSDK.
 */

#pragma once
#include "pcbang_misc.h"
#include "base_types.h"


 // ============================================================
 //  PCBangSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStovePCBangTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_PCBANG_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStovePCBangTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStovePCBangLoginOutcome  (legacy name: StovePCBangLogin)
// ============================================================

// Structure delivered to OnPCBangLoginCallback when Stove_PCBangLogin()
// completes the initial PC bang login.
struct IStovePCBangLoginOutcome : IStoveTypeBase
{
    STOVE_PCBANG_TYPE_KIND(k_EStovePCBangTypeKind_StovePCBangLoginOutcome);

    // PC bang premium state (see EStovePCBangPremium).
    virtual int32_t GetPremiumCheck() const = 0;

    // PC bang seat / session number (PSN) assigned to the user.
    virtual int32_t GetPsn() const = 0;

    // Remaining paid-benefit time, in seconds.
    virtual int32_t GetRemainTime() const = 0;
};


// ============================================================
//  IStovePCBangBenefitInfo  (legacy name: StovePCBangUserBenefit)
// ============================================================

// Structure delivered to OnRefreshPCBangBenefitCallback on each
// periodic benefit refresh (fired by the SDK every 4 minutes after
// a successful Stove_PCBangLogin()).
struct IStovePCBangBenefitInfo : IStoveTypeBase
{
    STOVE_PCBANG_TYPE_KIND(k_EStovePCBangTypeKind_StovePCBangBenefitInfo);

    // PC bang premium state (see EStovePCBangPremium).
    virtual int32_t GetPremiumCheck() const = 0;

    // Remaining paid-benefit time, in seconds.
    virtual int32_t GetRemainTime() const = 0;
};


// ============================================================
//  IStovePCBangStatus  (legacy name: StovePCBangUserStatus)
// ============================================================

// Structure delivered to OnPCBangCheckStatusCallback by
// Stove_PCBangCheckStatus() — reports the current PC bang context
// and the product the user is entitled to consume.
struct IStovePCBangStatus : IStoveTypeBase
{
    STOVE_PCBANG_TYPE_KIND(k_EStovePCBangTypeKind_StovePCBangStatus);

    // PC bang premium state (see EStovePCBangPremium).
    virtual int32_t GetPremiumCheck() const = 0;

    // PC bang seat / session number (PSN) assigned to the user.
    virtual int32_t GetPsn() const = 0;

    // Premium PC-bang product code. -1 = no product (error or not held);
    // a positive value is the server-assigned identifier of the product
    // currently in use (server-driven, not a fixed enum).
    virtual int32_t GetProductCode() const = 0;
};

#else

typedef struct IStovePCBangLoginOutcome IStovePCBangLoginOutcome;
typedef struct IStovePCBangBenefitInfo IStovePCBangBenefitInfo;
typedef struct IStovePCBangStatus IStovePCBangStatus;

#endif // __cplusplus


// ============================================================
//  PCBangSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and any accompanying object pointers
// (login, benefit, status) are owned by the SDK and are valid only for
// the duration of the callback invocation; do NOT call Destroy() on them.

// Delivered by Stove_PCBangLogin() when the initial login completes.
typedef void(__cdecl* OnPCBangLoginCallback)(const IStoveCallbackResult* callbackResult, const IStovePCBangLoginOutcome* loginOutcome);

// Delivered periodically (every 4 minutes) after a successful Stove_PCBangLogin(),
// carrying refreshed benefit information.
typedef void(__cdecl* OnRefreshPCBangBenefitCallback)(const IStoveCallbackResult* callbackResult, const IStovePCBangBenefitInfo* benefitInfo);

// Delivered by Stove_PCBangLogout().
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnPCBangLogoutCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);

// Delivered by Stove_PCBangCheckStatus().
typedef void(__cdecl* OnPCBangCheckStatusCallback)(const IStoveCallbackResult* callbackResult, const IStovePCBangStatus* status);