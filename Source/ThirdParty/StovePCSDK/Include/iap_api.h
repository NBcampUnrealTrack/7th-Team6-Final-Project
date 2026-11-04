/*
 * file: iap_api.h
 * Flat C API function declarations provided by IAPSDK.
 */

#pragma once
#include "iap_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IAPSDK API
    // ============================================================


    // Fetches the shop category tree.
    // The array of categories is delivered to onFinished.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchShopCategories(const IStoveTypeBase* param, OnFetchShopCategoriesCallback onFinished, void* userData);


    // Fetches products, optionally filtered by category and paginated.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchProducts(const IStoveFetchProductsParam* params, OnFetchProductsCallback onFinished, void* userData);


    // Starts a purchase flow for one or more products.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_StartPurchase(const IStoveStartPurchaseParam* params,
                                          OnStartPurchaseCallback onFinished, OnIAPPopupDestroyCallback onDestroy,
                                          void* userData1, void* userData2);


    // Confirms (finalizes) a purchase previously started with
    // Stove_StartPurchase() whose operation != WithWebViewAndConfirmResult.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_ConfirmPurchase(const IStoveConfirmPurchaseParam* param, OnConfirmPurchaseCallback onFinished, void* userData);


    // Fetches the purchase-history inventory of the signed-in user.
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchInventory(const IStoveTypeBase* param, OnFetchInventoryCallback onFinished, void* userData);


    // Fetches the user's terms-agreement status and, if not yet agreed,
    // either returns a URL (Default) or opens the terms page in a webview.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_FetchTermsAgreement(const IStoveFetchTermsAgreementParam* param,
                                                OnFetchTermsAgreementCallback onFinished, OnIAPPopupDestroyCallback onDestroy,
                                                void* userData1, void* userData2);


    // Starts a game-withdrawal (account detach) flow.
    // userData1: User data passed to onFinished
    // userData2: User data passed to onDestroy
    // Asynchronous: delivered only while Stove_RunCallback() is being pumped.
    void STOVE_API Stove_WithdrawGame(const IStoveWithdrawGameParam* param,
                                         OnWithdrawGameCallback onFinished,
                                         OnIAPPopupDestroyCallback onDestroy,
                                         void* userData1, void* userData2);



    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef void(__cdecl* Stove_FetchShopCategories_t)(const IStoveTypeBase* param, OnFetchShopCategoriesCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_FetchProducts_t)(const IStoveFetchProductsParam* params, OnFetchProductsCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_StartPurchase_t)(const IStoveStartPurchaseParam* params,
                                                    OnStartPurchaseCallback onFinished, OnIAPPopupDestroyCallback onDestroy,
                                                    void* userData1, void* userData2);

    typedef void(__cdecl* Stove_ConfirmPurchase_t)(const IStoveConfirmPurchaseParam* param, OnConfirmPurchaseCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_FetchInventory_t)(const IStoveTypeBase* param, OnFetchInventoryCallback onFinished, void* userData);

    typedef void(__cdecl* Stove_FetchTermsAgreement_t)(const IStoveFetchTermsAgreementParam* param,
                                                          OnFetchTermsAgreementCallback onFinished, OnIAPPopupDestroyCallback onDestroy,
                                                          void* userData1, void* userData2);

    typedef void(__cdecl* Stove_WithdrawGame_t)(const IStoveWithdrawGameParam* param,
                                                   OnWithdrawGameCallback onFinished, OnIAPPopupDestroyCallback onDestroy,
                                                   void* userData1, void* userData2);

#ifdef __cplusplus
}
#endif