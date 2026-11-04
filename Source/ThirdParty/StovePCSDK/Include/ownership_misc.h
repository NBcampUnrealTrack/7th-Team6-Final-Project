/*
 * file: ownership_misc.h
 * Enum definitions used in OwnershipSDK.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>


 // ============================================================
 //  OwnershipSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance owned by OwnershipSDK.
 // Used by IStoveTypeBase::GetTypeKind() for runtime type identification of
 // result/data objects delivered through Ownership async callbacks.
 // Range (4000 ~ 4999)
typedef enum EStoveOwnershipTypeKind
{
    k_EStoveOwnershipTypeKind_Invalid = -1,

    // Result / Data types (delivered by the SDK via callbacks)
    k_EStoveOwnershipTypeKind_StoveOwnership = 4000,
    k_EStoveOwnershipTypeKind_OwnershipList  = 4001,

    k_EStoveOwnershipTypeKind_Max = 0x7fffffff
} EStoveOwnershipTypeKind;


// Identifies which OwnershipSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 4500~4999 are an internal diagnostic band and are not part of this enum.
typedef enum EStoveOwnershipMethodCode
{
    k_EStoveOwnershipMethodCode_Invalid = -1,

    // Public APIs. Module block 4000~4999 (sequential).
    k_EStoveOwnershipMethodCode_FetchOwnerships = 4000,

    k_EStoveOwnershipMethodCode_Max = 0x7fffffff
} EStoveOwnershipMethodCode;


// Ownership state of a single game / DLC entry.
// Returned by IStoveOwnership::GetOwnershipCode().
typedef enum EStoveOwnershipCode
{
    k_EStoveOwnershipCode_Invalid = -1,

    k_EStoveOwnershipCode_None = 0,      // No ownership state recorded
    k_EStoveOwnershipCode_Acquire = 1,   // The user currently owns the entry
    k_EStoveOwnershipCode_Lose = 2,      // The user has lost ownership (e.g., refund)

    k_EStoveOwnershipCode_Max = 0x7fffffff
} EStoveOwnershipCode;


// Product category of an ownership entry.
// Returned by IStoveOwnership::GetGameCode().
// Values 1 and 2 are reserved / deprecated.
typedef enum EStoveOwnershipGameCode
{
    k_EStoveOwnershipGameCode_Invalid = -1,

    k_EStoveOwnershipGameCode_None = 0,  // Unspecified / not a product entry
    // 1, 2 : Deprecated
    k_EStoveOwnershipGameCode_Basic = 3, // Base game
    k_EStoveOwnershipGameCode_Demo = 4,  // Demo build
    k_EStoveOwnershipGameCode_Dlc = 5,   // Downloadable content tied to a base game

    k_EStoveOwnershipGameCode_Max = 0x7fffffff
} EStoveOwnershipGameCode;