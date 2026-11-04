/*
 * file: pcbang_flat_api.h
 * Flat C accessor function declarations for PCBangSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 *           properties from C and C# environments where C++ virtual calls
 *           (vtable dispatch) are not directly available.
 *
 *           Each accessor follows the naming pattern
 *               Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *           and forwards the call to the corresponding virtual method on the
 *           interface defined in pcbang_types.h.
 *
 *           For the semantics, units, and valid ranges of each field,
 *           refer to the matching interface definition in pcbang_types.h —
 *           this file intentionally does not duplicate that documentation
 *           to keep the accessor list readable.
 */

#pragma once
#include "pcbang_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStovePCBangLoginOutcome  —  Initial PC bang login result
    // ============================================================
    int32_t STOVE_API Stove_IStovePCBangLoginOutcome_GetPremiumCheck(const IStovePCBangLoginOutcome* self);  // EStovePCBangPremium
    int32_t STOVE_API Stove_IStovePCBangLoginOutcome_GetPsn(const IStovePCBangLoginOutcome* self);
    int32_t STOVE_API Stove_IStovePCBangLoginOutcome_GetRemainTime(const IStovePCBangLoginOutcome* self);

    // ============================================================
    //  IStovePCBangBenefitInfo  —  Periodic benefit refresh payload
    // ============================================================
    int32_t STOVE_API Stove_IStovePCBangBenefitInfo_GetPremiumCheck(const IStovePCBangBenefitInfo* self);  // EStovePCBangPremium
    int32_t STOVE_API Stove_IStovePCBangBenefitInfo_GetRemainTime(const IStovePCBangBenefitInfo* self);

    // ============================================================
    //  IStovePCBangStatus  —  On-demand PC bang status query result
    // ============================================================
    int32_t STOVE_API Stove_IStovePCBangStatus_GetPremiumCheck(const IStovePCBangStatus* self);  // EStovePCBangPremium
    int32_t STOVE_API Stove_IStovePCBangStatus_GetPsn(const IStovePCBangStatus* self);
    int32_t STOVE_API Stove_IStovePCBangStatus_GetProductCode(const IStovePCBangStatus* self);


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStovePCBangLoginOutcome =====
    typedef int32_t(__cdecl* Stove_IStovePCBangLoginOutcome_GetPremiumCheck_t)(const IStovePCBangLoginOutcome* self);
    typedef int32_t(__cdecl* Stove_IStovePCBangLoginOutcome_GetPsn_t)(const IStovePCBangLoginOutcome* self);
    typedef int32_t(__cdecl* Stove_IStovePCBangLoginOutcome_GetRemainTime_t)(const IStovePCBangLoginOutcome* self);

    // ===== IStovePCBangBenefitInfo =====
    typedef int32_t(__cdecl* Stove_IStovePCBangBenefitInfo_GetPremiumCheck_t)(const IStovePCBangBenefitInfo* self);
    typedef int32_t(__cdecl* Stove_IStovePCBangBenefitInfo_GetRemainTime_t)(const IStovePCBangBenefitInfo* self);

    // ===== IStovePCBangStatus =====
    typedef int32_t(__cdecl* Stove_IStovePCBangStatus_GetPremiumCheck_t)(const IStovePCBangStatus* self);
    typedef int32_t(__cdecl* Stove_IStovePCBangStatus_GetPsn_t)(const IStovePCBangStatus* self);
    typedef int32_t(__cdecl* Stove_IStovePCBangStatus_GetProductCode_t)(const IStovePCBangStatus* self);

#ifdef __cplusplus
}
#endif