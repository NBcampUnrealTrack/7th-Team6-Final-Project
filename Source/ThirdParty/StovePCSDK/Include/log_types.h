/*
 * file: log_types.h
 * Pure virtual interface and callback definitions used in LogSDK.
 */

#pragma once
#include "log_misc.h"
#include "base_types.h"


 // ============================================================
 //  LogSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveLogTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_LOG_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveLogTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStoveSendLogParam
// ============================================================

// Parameters for Stove_SendLog().
// Create via Stove_CreateParam(k_EStoveLogTypeKind_SendLogParam),
// populate the fields via setters, then Destroy() after the call returns.
//
// Fields whose value is not known for a given log entry may be left at
// their default (0 for numeric fields, nullptr for string fields).
struct IStoveSendLogParam : IStoveTypeBase
{
    STOVE_LOG_TYPE_KIND(k_EStoveLogTypeKind_SendLogParam);

    // Account UID (Stove account identifier of the user).
    virtual int64_t GetAuid() const = 0;
    virtual void SetAuid(int64_t auid) = 0;

    // Character UID (in-game character identifier of the user).
    virtual int64_t GetCuid() const = 0;
    virtual void SetCuid(int64_t cuid) = 0;

    // Name of an integrated 3rd-party marketing service (slot 1).
    // MktType1/MktId1 and MktType2/MktId2 are two independent slots -- they
    // are NOT primary / fallback; set only the slots that apply.
    virtual const wchar_t* GetMktType1() const = 0;
    virtual void SetMktType1(const wchar_t* mktType1) = 0;

    // Identifier issued by the 3rd-party marketing service in slot 1
    // (campaign / referrer id specific to MktType1).
    virtual const wchar_t* GetMktId1() const = 0;
    virtual void SetMktId1(const wchar_t* mktId1) = 0;

    // Name of an integrated 3rd-party marketing service (slot 2).
    virtual const wchar_t* GetMktType2() const = 0;
    virtual void SetMktType2(const wchar_t* mktType2) = 0;

    // Identifier issued by the 3rd-party marketing service in slot 2
    // (campaign / referrer id specific to MktType2).
    virtual const wchar_t* GetMktId2() const = 0;
    virtual void SetMktId2(const wchar_t* mktId2) = 0;

    // Game client version string (e.g., L"1.2.3").
    virtual const wchar_t* GetGameVersion() const = 0;
    virtual void SetGameVersion(const wchar_t* gameVersion) = 0;

    // Correlation ID used to group a set of related log entries together
    // (e.g., all logs produced within a single in-game transaction or flow).
    // Entries sharing the same LogGroupId are treated as one logical set
    // by the log backend.
    virtual const wchar_t* GetLogGroupId() const = 0;
    virtual void SetLogGroupId(const wchar_t* logGroupId) = 0;

    // Server code (world / region server the user is connected to).
    virtual const wchar_t* GetServerCode() const = 0;
    virtual void SetServerCode(const wchar_t* serverCd) = 0;

    // Server code -- detail (sub-server / channel / shard under ServerCode).
    virtual const wchar_t* GetServerCodeDetail() const = 0;
    virtual void SetServerCodeDetail(const wchar_t* serverCdDet) = 0;

    // Account-level value captured at the time the log entry was recorded.
    virtual const wchar_t* GetLevelCode() const = 0;
    virtual void SetLevelCode(const wchar_t* lvCd) = 0;

    // Character-level value captured at the time the log entry was recorded.
    // Despite the "Detail" suffix this is NOT a sub-level of LevelCode -- it is
    // the character-scoped counterpart of the account-scoped LevelCode.
    virtual const wchar_t* GetLevelCodeDetail() const = 0;
    virtual void SetLevelCodeDetail(const wchar_t* lvCdDet) = 0;

    // Identifier used to link and track user actions from ad attribution to in-game behavior
    // across Singular (MMP), Stove SDK, Log SDK, and in-game logs.
    // For games integrated with Singular, the SDID (Singular Device ID) should be passed through this field.
    virtual const wchar_t* GetExternalId() const = 0;
    virtual void SetExternalId(const wchar_t* externalId) = 0;

    // Free-form log payload (typically a JSON document encoded as a wide-string)
    // for fields not covered by the typed members above.
    // Corresponds to the legacy 81plug 'action_param' field.
    virtual const wchar_t* GetContents() const = 0;
    virtual void SetContents(const wchar_t* contents) = 0;
};

#else

typedef struct IStoveSendLogParam IStoveSendLogParam;

#endif // __cplusplus


// ============================================================
//  LogSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* is owned by the SDK and is valid only for
// the duration of the callback invocation; do NOT call Destroy() on it.

// Delivered by Stove_SendLog() once the log entry has been submitted
// to the backend (or failed to submit).
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnSendLogCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);