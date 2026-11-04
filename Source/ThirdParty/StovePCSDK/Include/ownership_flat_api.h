/*
 * file: ownership_flat_api.h
 * Flat C accessor function declarations for OwnershipSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 *           properties from C and C# environments where C++ virtual calls
 *           (vtable dispatch) are not directly available.
 *
 *           Each accessor follows the naming pattern
 *               Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *           and forwards the call to the corresponding virtual method on the
 *           interface defined in ownership_types.h.
 *
 *           For the semantics, units, and valid ranges of each field,
 *           refer to the matching interface definition in ownership_types.h —
 *           this file intentionally does not duplicate that documentation
 *           to keep the accessor list readable.
 */

#pragma once
#include "ownership_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveOwnership  —  Single ownership entry (base game / DLC / demo)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveOwnership_GetGameId(const IStoveOwnership* self);
    int32_t STOVE_API Stove_IStoveOwnership_GetGameCode(const IStoveOwnership* self);        // EStoveOwnershipGameCode
    int32_t STOVE_API Stove_IStoveOwnership_GetOwnershipCode(const IStoveOwnership* self);   // EStoveOwnershipCode
    uint64_t STOVE_API Stove_IStoveOwnership_GetPurchaseDate(const IStoveOwnership* self);


    // ============================================================
    //  IStoveOwnershipList  —  List of IStoveOwnership entries
    // ============================================================
    uint32_t STOVE_API Stove_IStoveOwnershipList_GetCount(const IStoveOwnershipList* self);
    const IStoveOwnership* STOVE_API Stove_IStoveOwnershipList_GetAt(const IStoveOwnershipList* self, uint32_t index);


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveOwnership =====
    typedef const wchar_t* (__cdecl* Stove_IStoveOwnership_GetGameId_t)(const IStoveOwnership* self);
    typedef int32_t(__cdecl* Stove_IStoveOwnership_GetGameCode_t)(const IStoveOwnership* self);
    typedef int32_t(__cdecl* Stove_IStoveOwnership_GetOwnershipCode_t)(const IStoveOwnership* self);
    typedef uint64_t(__cdecl* Stove_IStoveOwnership_GetPurchaseDate_t)(const IStoveOwnership* self);

    // ===== IStoveOwnershipList =====
    typedef uint32_t(__cdecl* Stove_IStoveOwnershipList_GetCount_t)(const IStoveOwnershipList* self);
    typedef const IStoveOwnership* (__cdecl* Stove_IStoveOwnershipList_GetAt_t)(const IStoveOwnershipList* self, uint32_t index);

#ifdef __cplusplus
}
#endif