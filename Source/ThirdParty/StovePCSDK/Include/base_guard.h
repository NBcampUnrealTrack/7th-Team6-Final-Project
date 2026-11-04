/*
 * file: base_guard.h
 * Suggested location: BaseSDK/Public/C/base_guard.h
 *
 * C++-only RAII helpers over the flat C surface. Header-only, compiled by the
 * caller, not part of the exported ABI. Entirely optional: base_api.h /
 * base_flat_api.h and the per-module *_api.h headers are usable on their own.
 */

#pragma once

#ifndef __cplusplus
#error "base_guard.h is C++ only. Use base_api.h / base_flat_api.h from C."
#endif

#include "base_api.h"

#include <type_traits>


 // ============================================================
 //  Ownership guard
 // ============================================================
 //
 // Covers every object the caller owns (ShouldDestroy() == true):
 //   - objects from Stove_CreateParam()          (any module's *Param kind)
 //   - objects handed back through an out parameter
 //     (Stove_GetUser / Stove_GetGds / Stove_GetSignin, ...)
 //
 // Safe on SDK-owned objects too: Reset() honours ShouldDestroy(), so wrapping a
 // callback payload by mistake is a no-op rather than a double free.
 //
 // No operator bool here on purpose -- see StoveResultGuard, where bool means
 // "succeeded". One name with two meanings would be a trap.

template <typename T>
class StoveGuard
{
public:
    StoveGuard() : _p(nullptr) {}
    explicit StoveGuard(T* p) : _p(p) {}

    StoveGuard(StoveGuard&& other) STOVE_NOEXCEPT : _p(other._p)
    {
        other._p = nullptr;
    }

    StoveGuard& operator=(StoveGuard&& other) STOVE_NOEXCEPT
    {
        if (this != &other && _p != other._p)
        {
            Reset();
            _p = other._p;
            other._p = nullptr;
        }
        return *this;
    }

    ~StoveGuard() { Reset(); }

    StoveGuard(const StoveGuard&) = delete;
    StoveGuard& operator=(const StoveGuard&) = delete;

    void Reset(T* p = nullptr)
    {
        if (_p != nullptr && _p != p)
        {
            if (_p->ShouldDestroy())
                _p->Destroy();
        }
        _p = p;
    }

    // For out-parameter APIs: releases any current object and hands back the
    // address to write into.
    //
    //   StoveGuard<IStoveUser> user;
    //   StoveResultGuard r(Stove_GetUser(user.Put()));
    //   if (r) { user->GetNickName(); }
    T** Put()
    {
        Reset();
        return &_p;
    }

    // Hands ownership back to the caller (e.g. to return the raw pointer across
    // a C boundary). The guard no longer destroys it.
    T* Detach()
    {
        T* p = _p;
        _p = nullptr;
        return p;
    }

    bool IsValid() const { return _p != nullptr; }

    T* Get() const { return _p; }
    T* operator->() const { return _p; }

private:
    T* _p;
};


// Creates a param object through the unified factory. The kind comes from the
// interface's own TypeKind, so there is no cast and no kind argument to get wrong.
// Returns an empty guard when the SDK refuses the kind.
//
//   // Base
//   auto initParam = StoveMakeParam<IStoveInitializeParam>();
//   if (!initParam.IsValid()) return;
//   initParam->SetMainWndHandle(hwnd);
//   StoveResultGuard r(Stove_Initialize(initParam.Get()));
//
//   // View -- same template, no view-specific helper needed
//   auto popupParam = StoveMakeParam<IStovePopupParam>();
//   if (!popupParam.IsValid()) return;
//   popupParam->SetWebViewMode(k_EStoveWebViewMode_Internal);
//   Stove_AutoPopup(popupParam.Get(), OnFinished, OnDestroy, ud1, ud2);
template <typename TParam>
StoveGuard<TParam> StoveMakeParam()
{
    static_assert(std::is_base_of<IStoveTypeBase, TParam>::value,
                  "StoveMakeParam<T>: T must derive from IStoveTypeBase.");

    IStoveTypeBase* base = Stove_CreateParam(static_cast<int>(TParam::k_eStaticTypeKind));

    if (base == nullptr)
        return StoveGuard<TParam>();

    if (base->GetTypeKind() != static_cast<int>(TParam::k_eStaticTypeKind))
    {
        if (base->ShouldDestroy())
            base->Destroy();
        return StoveGuard<TParam>();
    }

    return StoveGuard<TParam>(static_cast<TParam*>(base));
}


// ============================================================
//  Result guard
// ============================================================
//
// Same ownership rule as StoveGuard, plus null-safe outcome queries: a result
// pointer can legitimately be null, and every accessor below answers sensibly in
// that case instead of dereferencing.
//
// operator bool means "the call succeeded", so `if (!r)` covers both a failed
// call and a missing result object.
//
// GetErrorMessage() / GetExternalError() live on IStoveCallbackResult, not on
// IStoveResult, so they are not exposed here. Callback results are SDK-owned
// (ShouldDestroy() == false) and need no guard -- read them directly inside the
// callback.

class StoveResultGuard
{
public:
    StoveResultGuard() {}
    explicit StoveResultGuard(IStoveResult* r) : _guard(r) {}

    StoveResultGuard(StoveResultGuard&&) = default;
    StoveResultGuard& operator=(StoveResultGuard&&) = default;

    StoveResultGuard(const StoveResultGuard&) = delete;
    StoveResultGuard& operator=(const StoveResultGuard&) = delete;

    void Reset(IStoveResult* r = nullptr) { _guard.Reset(r); }
    IStoveResult* Detach() { return _guard.Detach(); }

    bool IsValid() const { return _guard.IsValid(); }
    const IStoveResult* Get() const { return _guard.Get(); }

    bool IsSuccessful() const
    {
        return _guard.IsValid() && _guard.Get()->IsSuccessful();
    }

    explicit operator bool() const { return IsSuccessful(); }

    // Success is 0 in this SDK, so a null result must NOT report 0 here --
    // that would read as success.
    uint32_t GetResultCode() const
    {
        return _guard.IsValid()
            ? _guard.Get()->GetResultCode()
            : static_cast<uint32_t>(k_EStoveCommonResultCode_UnknownError);
    }

    // 0 when unavailable, matching Stove_IStoveResult_GetMethodCode().
    uint32_t GetMethodCode() const
    {
        return _guard.IsValid() ? _guard.Get()->GetMethodCode() : 0u;
    }

private:
    StoveGuard<IStoveResult> _guard;
};