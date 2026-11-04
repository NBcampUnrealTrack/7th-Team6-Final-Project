/*
 * file: ownership_types.h
 * Pure virtual interface and callback definitions used in OwnershipSDK.
 */

#pragma once
#include "ownership_misc.h"
#include "base_types.h"


 // ============================================================
 //  OwnershipSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveOwnershipTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_OWNERSHIP_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveOwnershipTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStoveOwnership  (legacy name: StovePCOwnership)
// ============================================================

// Structure representing a single ownership entry (base game, DLC, demo, ...)
// for the signed-in user. Delivered via IStoveOwnershipList to
// OnOwnershipListCallback by Stove_FetchOwnerships().
struct IStoveOwnership : IStoveTypeBase
{
    STOVE_OWNERSHIP_TYPE_KIND(k_EStoveOwnershipTypeKind_StoveOwnership);

    // Stove game ID associated with this ownership entry.
    virtual const wchar_t* GetGameId() const = 0;

    // Product category of the entry (see EStoveOwnershipGameCode:
    // Basic / Demo / Dlc, ...).
    virtual int32_t GetGameCode() const = 0;

    // Ownership state of the entry (see EStoveOwnershipCode:
    // Acquire / Lose / None).
    virtual int32_t GetOwnershipCode() const = 0;

    // Purchase time as a Unix epoch timestamp in milliseconds (UTC),
    // passed through from the server unchanged (same convention as other timestamp fields).
    virtual uint64_t GetPurchaseDate() const = 0;
};


// ============================================================
//  IStoveOwnershipList
// ============================================================

// List wrapper for ownership entries delivered by Stove_FetchOwnerships().
// The list object is SDK-owned and valid only for the duration of the callback;
// copy any field values you need to retain.
struct IStoveOwnershipList : IStoveTypeBase
{
    STOVE_OWNERSHIP_TYPE_KIND(k_EStoveOwnershipTypeKind_OwnershipList);

    // Total number of ownership entries in this list.
    virtual uint32_t GetCount() const = 0;

    // Returns the ownership entry at 'index'. Returns nullptr if out of bounds.
    virtual const IStoveOwnership* GetAt(uint32_t index) const = 0;
};

#else

typedef struct IStoveOwnership IStoveOwnership;
typedef struct IStoveOwnershipList IStoveOwnershipList;

#endif // __cplusplus


// ============================================================
//  OwnershipSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and any accompanying object pointers are owned
// by the SDK and are valid only for the duration of the callback invocation;
// do NOT call Destroy() on them.

// Delivered by Stove_FetchOwnerships() with the ownership entries of the
// currently running game (base game + DLCs + ...).
//
// 'ownerships' points to an IStoveOwnershipList containing all ownership entries.
// The list object is SDK-owned and valid only for the duration of this callback;
// copy any field values you need to retain.
typedef void(__cdecl* OnOwnershipListCallback)(const IStoveCallbackResult* callbackResult, const IStoveOwnershipList* ownerships);