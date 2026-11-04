/*
 * file: log_flat_api.h
 * Flat C accessor function declarations for LogSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 *           properties from C and C# environments where C++ virtual calls
 *           (vtable dispatch) are not directly available.
 *
 *           Each accessor follows the naming pattern
 *               Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *           and forwards the call to the corresponding virtual method on the
 *           interface defined in log_types.h.
 *
 *           For the semantics, units, and valid ranges of each field,
 *           refer to the matching interface definition in log_types.h —
 *           this file intentionally does not duplicate that documentation
 *           to keep the accessor list readable.
 */

#pragma once
#include "log_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveSendLogParam  —  Log-entry parameters (get/set pairs adjacent)
    // ============================================================
    int64_t STOVE_API Stove_IStoveSendLogParam_GetAuid(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetAuid(IStoveSendLogParam* self, int64_t auid);

    int64_t STOVE_API Stove_IStoveSendLogParam_GetCuid(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetCuid(IStoveSendLogParam* self, int64_t cuid);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetMktType1(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetMktType1(IStoveSendLogParam* self, const wchar_t* mktType1);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetMktId1(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetMktId1(IStoveSendLogParam* self, const wchar_t* mktId1);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetMktType2(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetMktType2(IStoveSendLogParam* self, const wchar_t* mktType2);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetMktId2(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetMktId2(IStoveSendLogParam* self, const wchar_t* mktId2);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetGameVersion(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetGameVersion(IStoveSendLogParam* self, const wchar_t* gameVersion);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetLogGroupId(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetLogGroupId(IStoveSendLogParam* self, const wchar_t* logGroupId);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetServerCode(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetServerCode(IStoveSendLogParam* self, const wchar_t* serverCd);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetServerCodeDetail(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetServerCodeDetail(IStoveSendLogParam* self, const wchar_t* serverCdDet);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetLevelCode(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetLevelCode(IStoveSendLogParam* self, const wchar_t* lvCd);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetLevelCodeDetail(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetLevelCodeDetail(IStoveSendLogParam* self, const wchar_t* lvCdDet);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetExternalId(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetExternalId(IStoveSendLogParam* self, const wchar_t* externalId);

    const wchar_t* STOVE_API Stove_IStoveSendLogParam_GetContents(const IStoveSendLogParam* self);
    void STOVE_API Stove_IStoveSendLogParam_SetContents(IStoveSendLogParam* self, const wchar_t* contents);


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveSendLogParam =====
    typedef int64_t(__cdecl* Stove_IStoveSendLogParam_GetAuid_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetAuid_t)(IStoveSendLogParam* self, int64_t auid);

    typedef int64_t(__cdecl* Stove_IStoveSendLogParam_GetCuid_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetCuid_t)(IStoveSendLogParam* self, int64_t cuid);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetMktType1_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetMktType1_t)(IStoveSendLogParam* self, const wchar_t* mktType1);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetMktId1_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetMktId1_t)(IStoveSendLogParam* self, const wchar_t* mktId1);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetMktType2_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetMktType2_t)(IStoveSendLogParam* self, const wchar_t* mktType2);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetMktId2_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetMktId2_t)(IStoveSendLogParam* self, const wchar_t* mktId2);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetGameVersion_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetGameVersion_t)(IStoveSendLogParam* self, const wchar_t* gameVersion);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetLogGroupId_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetLogGroupId_t)(IStoveSendLogParam* self, const wchar_t* logGroupId);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetServerCode_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetServerCode_t)(IStoveSendLogParam* self, const wchar_t* serverCd);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetServerCodeDetail_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetServerCodeDetail_t)(IStoveSendLogParam* self, const wchar_t* serverCdDet);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetLevelCode_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetLevelCode_t)(IStoveSendLogParam* self, const wchar_t* lvCd);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetLevelCodeDetail_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetLevelCodeDetail_t)(IStoveSendLogParam* self, const wchar_t* lvCdDet);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetExternalId_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetExternalId_t)(IStoveSendLogParam* self, const wchar_t* externalId);

    typedef const wchar_t* (__cdecl* Stove_IStoveSendLogParam_GetContents_t)(const IStoveSendLogParam* self);
    typedef void(__cdecl* Stove_IStoveSendLogParam_SetContents_t)(IStoveSendLogParam* self, const wchar_t* contents);

#ifdef __cplusplus
}
#endif