/*
 * file: iap_flat_api.h
 * Flat C accessor function declarations for IAPSDK interface members.
 * Provides flat wrapper functions for accessing IStove* interface
 *           properties from C and C# environments where C++ virtual calls
 *           (vtable dispatch) are not directly available.
 *
 *           Each accessor follows the naming pattern
 *               Stove_<InterfaceName>_<MethodName>(<InterfaceName>* self, ...)
 *           and forwards the call to the corresponding virtual method on the
 *           interface defined in iap_types.h.
 *
 *           For the semantics, units, and valid ranges of each field,
 *           refer to the matching interface definition in iap_types.h —
 *           this file intentionally does not duplicate that documentation
 *           to keep the accessor list readable.
 *
 *           The common IStoveTypeBase accessors (GetTypeKind, ShouldDestroy,
 *           Destroy, QueryExt) are declared in base_flat_api.h and are not
 *           redeclared here.
 */

#pragma once
#include "iap_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  IStoveShopCategory  —  One shop category entry
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveShopCategory_GetCategoryId(const IStoveShopCategory* self);
    const wchar_t* STOVE_API Stove_IStoveShopCategory_GetCategoryParentId(const IStoveShopCategory* self);
    int32_t STOVE_API Stove_IStoveShopCategory_GetCategoryDisplayNo(const IStoveShopCategory* self);
    const wchar_t* STOVE_API Stove_IStoveShopCategory_GetCategoryName(const IStoveShopCategory* self);
    int32_t STOVE_API Stove_IStoveShopCategory_GetCategoryDepth(const IStoveShopCategory* self);

    // ============================================================
    //  IStoveProduct  —  One product entry
    // ============================================================
    int64_t STOVE_API Stove_IStoveProduct_GetProductId(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetInserviceItemId(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetProductName(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetProductDescription(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetQuantity(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetProductTypeCode(const IStoveProduct* self);     // EStoveProductTypeCode
    const wchar_t* STOVE_API Stove_IStoveProduct_GetCategoryId(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetCategoryName(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetCurrencyCode(const IStoveProduct* self);
    double STOVE_API Stove_IStoveProduct_GetPrice(const IStoveProduct* self);
    double STOVE_API Stove_IStoveProduct_GetDisplayPrice(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetStrDisplayPrice(const IStoveProduct* self);
    double STOVE_API Stove_IStoveProduct_GetSalePrice(const IStoveProduct* self);
    double STOVE_API Stove_IStoveProduct_GetDisplaySalePrice(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetStrDisplaySalePrice(const IStoveProduct* self);
    bool STOVE_API Stove_IStoveProduct_IsDiscounted(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetDiscountType(const IStoveProduct* self);        // EStoveDiscountType
    int32_t STOVE_API Stove_IStoveProduct_GetDiscountTypeValue(const IStoveProduct* self);
    int64_t STOVE_API Stove_IStoveProduct_GetDiscountStartDate(const IStoveProduct* self);
    int64_t STOVE_API Stove_IStoveProduct_GetDiscountEndDate(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetTotalQuantity(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetMemberQuantity(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetGuidQuantity(const IStoveProduct* self);
    const wchar_t* STOVE_API Stove_IStoveProduct_GetThumbnailUrl(const IStoveProduct* self);
    bool STOVE_API Stove_IStoveProduct_IsWithdrawable(const IStoveProduct* self);
    bool STOVE_API Stove_IStoveProduct_HasPurchased(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetPurchaseLimitTypeCode(const IStoveProduct* self); // EStovePurchaseLimitTypeCode
    int32_t STOVE_API Stove_IStoveProduct_GetPurchaseLimitCount(const IStoveProduct* self);
    int32_t STOVE_API Stove_IStoveProduct_GetSaleLimitCount(const IStoveProduct* self);
    int64_t STOVE_API Stove_IStoveProduct_GetSalesStartDate(const IStoveProduct* self);
    int64_t STOVE_API Stove_IStoveProduct_GetSalesEndDate(const IStoveProduct* self);
    int16_t STOVE_API Stove_IStoveProduct_GetPurchaseAvailabilityCode(const IStoveProduct* self);

    // ============================================================
    //  IStovePurchasedProduct  —  Per-product entry of a completed purchase
    // ============================================================
    int64_t STOVE_API Stove_IStovePurchasedProduct_GetTxnDetailNo(const IStovePurchasedProduct* self);
    int64_t STOVE_API Stove_IStovePurchasedProduct_GetProductId(const IStovePurchasedProduct* self);
    const wchar_t* STOVE_API Stove_IStovePurchasedProduct_GetCategoryId(const IStovePurchasedProduct* self);
    int32_t STOVE_API Stove_IStovePurchasedProduct_GetTotalQuantity(const IStovePurchasedProduct* self);
    int32_t STOVE_API Stove_IStovePurchasedProduct_GetMemberQuantity(const IStovePurchasedProduct* self);
    int32_t STOVE_API Stove_IStovePurchasedProduct_GetGuidQuantity(const IStovePurchasedProduct* self);

    // ============================================================
    //  IStoveChargeInfo  —  Per-currency charge entry used in a purchase
    // ============================================================
    double STOVE_API Stove_IStoveChargeInfo_GetChargeDeductVal(const IStoveChargeInfo* self);
    double STOVE_API Stove_IStoveChargeInfo_GetChargeDisplayDeductVal(const IStoveChargeInfo* self);
    int32_t STOVE_API Stove_IStoveChargeInfo_GetChargeType(const IStoveChargeInfo* self);
    const wchar_t* STOVE_API Stove_IStoveChargeInfo_GetChargeTypeName(const IStoveChargeInfo* self);

    // ============================================================
    //  IStoveStartPurchaseOutcome  —  Outcome of Stove_StartPurchase()
    // ============================================================
    int64_t STOVE_API Stove_IStoveStartPurchaseOutcome_GetTxnMasterNo(const IStoveStartPurchaseOutcome* self);
    const int64_t* STOVE_API Stove_IStoveStartPurchaseOutcome_GetTxnDetailNos(const IStoveStartPurchaseOutcome* self);
    uint32_t STOVE_API Stove_IStoveStartPurchaseOutcome_GetTxnDetailNosCount(const IStoveStartPurchaseOutcome* self);
    const wchar_t* STOVE_API Stove_IStoveStartPurchaseOutcome_GetTempPaymentUrl(const IStoveStartPurchaseOutcome* self);
    int32_t STOVE_API Stove_IStoveStartPurchaseOutcome_GetPurchaseProgress(const IStoveStartPurchaseOutcome* self); // EStovePurchaseProgress
    bool STOVE_API Stove_IStoveStartPurchaseOutcome_IsPurchased(const IStoveStartPurchaseOutcome* self);
    const wchar_t* STOVE_API Stove_IStoveStartPurchaseOutcome_GetExtraData(const IStoveStartPurchaseOutcome* self);
    uint32_t STOVE_API Stove_IStoveStartPurchaseOutcome_GetPurchasedProductCount(const IStoveStartPurchaseOutcome* self);
    const IStovePurchasedProduct* STOVE_API Stove_IStoveStartPurchaseOutcome_GetPurchasedProductAt(const IStoveStartPurchaseOutcome* self, uint32_t index);
    uint32_t STOVE_API Stove_IStoveStartPurchaseOutcome_GetChargeInfoCount(const IStoveStartPurchaseOutcome* self);
    const IStoveChargeInfo* STOVE_API Stove_IStoveStartPurchaseOutcome_GetChargeInfoAt(const IStoveStartPurchaseOutcome* self, uint32_t index);
    bool STOVE_API Stove_IStoveStartPurchaseOutcome_IsPopupCreated(const IStoveStartPurchaseOutcome* self);

    // ============================================================
    //  IStoveConfirmPurchaseOutcome  —  Outcome of Stove_ConfirmPurchase()
    // ============================================================
    bool STOVE_API Stove_IStoveConfirmPurchaseOutcome_IsConfirmed(const IStoveConfirmPurchaseOutcome* self);
    uint32_t STOVE_API Stove_IStoveConfirmPurchaseOutcome_GetPurchasedProductCount(const IStoveConfirmPurchaseOutcome* self);
    const IStovePurchasedProduct* STOVE_API Stove_IStoveConfirmPurchaseOutcome_GetPurchasedProductAt(const IStoveConfirmPurchaseOutcome* self, uint32_t index);
    uint32_t STOVE_API Stove_IStoveConfirmPurchaseOutcome_GetChargeInfoCount(const IStoveConfirmPurchaseOutcome* self);
    const IStoveChargeInfo* STOVE_API Stove_IStoveConfirmPurchaseOutcome_GetChargeInfoAt(const IStoveConfirmPurchaseOutcome* self, uint32_t index);

    // ============================================================
    //  IStoveWithdrawGameOutcome  —  Outcome of Stove_WithdrawGame()
    // ============================================================
    bool STOVE_API Stove_IStoveWithdrawGameOutcome_IsGameWithdrawn(const IStoveWithdrawGameOutcome* self);
    bool STOVE_API Stove_IStoveWithdrawGameOutcome_IsPopupCreated(const IStoveWithdrawGameOutcome* self);

    // ============================================================
    //  IStoveTermsAgreementOutcome  —  Outcome of Stove_FetchTermsAgreement()
    // ============================================================
    bool STOVE_API Stove_IStoveTermsAgreementOutcome_IsAgreed(const IStoveTermsAgreementOutcome* self);
    const wchar_t* STOVE_API Stove_IStoveTermsAgreementOutcome_GetUrl(const IStoveTermsAgreementOutcome* self);
    bool STOVE_API Stove_IStoveTermsAgreementOutcome_IsPopupCreated(const IStoveTermsAgreementOutcome* self);

    // ============================================================
    //  IStoveShopCategoryList  —  Container of Stove_FetchShopCategories()
    // ============================================================
    uint32_t STOVE_API Stove_IStoveShopCategoryList_GetCount(const IStoveShopCategoryList* self);
    const IStoveShopCategory* STOVE_API Stove_IStoveShopCategoryList_GetAt(const IStoveShopCategoryList* self, uint32_t index);

    // ============================================================
    //  IStoveProductList  —  Container of Stove_FetchProducts()
    // ============================================================
    uint32_t STOVE_API Stove_IStoveProductList_GetCount(const IStoveProductList* self);
    const IStoveProduct* STOVE_API Stove_IStoveProductList_GetAt(const IStoveProductList* self, uint32_t index);

    // ============================================================
    //  IStoveInventoryList  —  Container of Stove_FetchInventory()
    // ============================================================
    uint32_t STOVE_API Stove_IStoveInventoryList_GetCount(const IStoveInventoryList* self);
    const IStoveInventoryItem* STOVE_API Stove_IStoveInventoryList_GetAt(const IStoveInventoryList* self, uint32_t index);

    // ============================================================
    //  IStoveInventoryItem  —  Single purchase-history entry
    // ============================================================
    int64_t STOVE_API Stove_IStoveInventoryItem_GetTxnMasterNo(const IStoveInventoryItem* self);
    int64_t STOVE_API Stove_IStoveInventoryItem_GetTxnDetailNo(const IStoveInventoryItem* self);
    int64_t STOVE_API Stove_IStoveInventoryItem_GetProductId(const IStoveInventoryItem* self);
    const wchar_t* STOVE_API Stove_IStoveInventoryItem_GetInserviceItemId(const IStoveInventoryItem* self);
    const wchar_t* STOVE_API Stove_IStoveInventoryItem_GetProductName(const IStoveInventoryItem* self);
    int32_t STOVE_API Stove_IStoveInventoryItem_GetQuantity(const IStoveInventoryItem* self);
    const wchar_t* STOVE_API Stove_IStoveInventoryItem_GetThumbnailUrl(const IStoveInventoryItem* self);

    // ============================================================
    //  IStoveFetchProductsParam  —  FetchProducts parameters (get/set pairs adjacent)
    // ============================================================
    const wchar_t* STOVE_API Stove_IStoveFetchProductsParam_GetCategoryId(const IStoveFetchProductsParam* self);
    void STOVE_API Stove_IStoveFetchProductsParam_SetCategoryId(IStoveFetchProductsParam* self, const wchar_t* categoryId);

    uint32_t STOVE_API Stove_IStoveFetchProductsParam_GetPageIndex(const IStoveFetchProductsParam* self);
    void STOVE_API Stove_IStoveFetchProductsParam_SetPageIndex(IStoveFetchProductsParam* self, uint32_t pageIndex);

    uint32_t STOVE_API Stove_IStoveFetchProductsParam_GetPageSize(const IStoveFetchProductsParam* self);
    void STOVE_API Stove_IStoveFetchProductsParam_SetPageSize(IStoveFetchProductsParam* self, uint32_t pageSize);

    // ============================================================
    //  IStoveOrderProductParam  —  Per-product order entry (get/set pairs adjacent)
    // ============================================================
    int64_t STOVE_API Stove_IStoveOrderProductParam_GetProductId(const IStoveOrderProductParam* self);
    void STOVE_API Stove_IStoveOrderProductParam_SetProductId(IStoveOrderProductParam* self, int64_t productId);

    double STOVE_API Stove_IStoveOrderProductParam_GetSalePrice(const IStoveOrderProductParam* self);
    void STOVE_API Stove_IStoveOrderProductParam_SetSalePrice(IStoveOrderProductParam* self, double salePrice);

    int32_t STOVE_API Stove_IStoveOrderProductParam_GetQuantity(const IStoveOrderProductParam* self);
    void STOVE_API Stove_IStoveOrderProductParam_SetQuantity(IStoveOrderProductParam* self, int32_t quantity);

    // ============================================================
    //  IStovePurchaseParam  —  StartPurchase behavior / webview options
    // ============================================================
    int32_t STOVE_API Stove_IStovePurchaseParam_GetOperation(const IStovePurchaseParam* self);  // EStovePurchaseOperation
    void STOVE_API Stove_IStovePurchaseParam_SetOperation(IStovePurchaseParam* self, int32_t operation);

    int32_t STOVE_API Stove_IStovePurchaseParam_GetWebViewMode(const IStovePurchaseParam* self); // EStoveWebViewMode
    void STOVE_API Stove_IStovePurchaseParam_SetWebViewMode(IStovePurchaseParam* self, int32_t webViewMode);

    int32_t STOVE_API Stove_IStovePurchaseParam_GetWebViewPosX(const IStovePurchaseParam* self);
    void STOVE_API Stove_IStovePurchaseParam_SetWebViewPosX(IStovePurchaseParam* self, int32_t posX);

    int32_t STOVE_API Stove_IStovePurchaseParam_GetWebViewPosY(const IStovePurchaseParam* self);
    void STOVE_API Stove_IStovePurchaseParam_SetWebViewPosY(IStovePurchaseParam* self, int32_t posY);

    int32_t STOVE_API Stove_IStovePurchaseParam_GetWebViewWidth(const IStovePurchaseParam* self);
    void STOVE_API Stove_IStovePurchaseParam_SetWebViewWidth(IStovePurchaseParam* self, int32_t width);

    int32_t STOVE_API Stove_IStovePurchaseParam_GetWebViewHeight(const IStovePurchaseParam* self);
    void STOVE_API Stove_IStovePurchaseParam_SetWebViewHeight(IStovePurchaseParam* self, int32_t height);

    // ============================================================
    //  IStoveStartPurchaseParam  —  StartPurchase parameters (get/set pairs adjacent)
    // ============================================================
    const IStoveOrderProductParam* const* STOVE_API Stove_IStoveStartPurchaseParam_GetProducts(const IStoveStartPurchaseParam* self);
    uint32_t STOVE_API Stove_IStoveStartPurchaseParam_GetProductsCount(const IStoveStartPurchaseParam* self);
    void STOVE_API Stove_IStoveStartPurchaseParam_SetProducts(IStoveStartPurchaseParam* self, IStoveOrderProductParam** products, uint32_t count);

    const IStovePurchaseParam* STOVE_API Stove_IStoveStartPurchaseParam_GetPurchaseParam(const IStoveStartPurchaseParam* self);
    void STOVE_API Stove_IStoveStartPurchaseParam_SetPurchaseParam(IStoveStartPurchaseParam* self, IStovePurchaseParam* param);

    const wchar_t* STOVE_API Stove_IStoveStartPurchaseParam_GetServiceTxnNo(const IStoveStartPurchaseParam* self);
    void STOVE_API Stove_IStoveStartPurchaseParam_SetServiceTxnNo(IStoveStartPurchaseParam* self, const wchar_t* serviceTxnNo);

    const wchar_t* STOVE_API Stove_IStoveStartPurchaseParam_GetExtraData(const IStoveStartPurchaseParam* self);
    void STOVE_API Stove_IStoveStartPurchaseParam_SetExtraData(IStoveStartPurchaseParam* self, const wchar_t* extraData);

    // ============================================================
    //  IStoveFetchTermsAgreementParam  —  FetchTermsAgreement behavior / webview options
    // ============================================================
    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetOperation(const IStoveFetchTermsAgreementParam* self);       // EStoveTermsOperation
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetOperation(IStoveFetchTermsAgreementParam* self, int32_t operation);

    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetWebViewMode(const IStoveFetchTermsAgreementParam* self);     // EStoveWebViewMode
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetWebViewMode(IStoveFetchTermsAgreementParam* self, int32_t webViewMode);

    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetWebViewPosX(const IStoveFetchTermsAgreementParam* self);
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetWebViewPosX(IStoveFetchTermsAgreementParam* self, int32_t posX);

    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetWebViewPosY(const IStoveFetchTermsAgreementParam* self);
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetWebViewPosY(IStoveFetchTermsAgreementParam* self, int32_t posY);

    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetWebViewWidth(const IStoveFetchTermsAgreementParam* self);
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetWebViewWidth(IStoveFetchTermsAgreementParam* self, int32_t width);

    int32_t STOVE_API Stove_IStoveFetchTermsAgreementParam_GetWebViewHeight(const IStoveFetchTermsAgreementParam* self);
    void STOVE_API Stove_IStoveFetchTermsAgreementParam_SetWebViewHeight(IStoveFetchTermsAgreementParam* self, int32_t height);

    // ============================================================
    //  IStoveWithdrawGameParam  —  WithdrawGame webview options
    // ============================================================
    int32_t STOVE_API Stove_IStoveWithdrawGameParam_GetWebViewMode(const IStoveWithdrawGameParam* self); // EStoveWebViewMode
    void STOVE_API Stove_IStoveWithdrawGameParam_SetWebViewMode(IStoveWithdrawGameParam* self, int32_t webViewMode);

    int32_t STOVE_API Stove_IStoveWithdrawGameParam_GetWebViewPosX(const IStoveWithdrawGameParam* self);
    void STOVE_API Stove_IStoveWithdrawGameParam_SetWebViewPosX(IStoveWithdrawGameParam* self, int32_t posX);

    int32_t STOVE_API Stove_IStoveWithdrawGameParam_GetWebViewPosY(const IStoveWithdrawGameParam* self);
    void STOVE_API Stove_IStoveWithdrawGameParam_SetWebViewPosY(IStoveWithdrawGameParam* self, int32_t posY);

    int32_t STOVE_API Stove_IStoveWithdrawGameParam_GetWebViewWidth(const IStoveWithdrawGameParam* self);
    void STOVE_API Stove_IStoveWithdrawGameParam_SetWebViewWidth(IStoveWithdrawGameParam* self, int32_t width);

    int32_t STOVE_API Stove_IStoveWithdrawGameParam_GetWebViewHeight(const IStoveWithdrawGameParam* self);
    void STOVE_API Stove_IStoveWithdrawGameParam_SetWebViewHeight(IStoveWithdrawGameParam* self, int32_t height);


    // ============================================================
    //  IStoveConfirmPurchaseParam  —  ConfirmPurchase query parameters (get/set pairs adjacent)
    // ============================================================
    int64_t STOVE_API Stove_IStoveConfirmPurchaseParam_GetTxnMasterNo(const IStoveConfirmPurchaseParam* self);
    void STOVE_API Stove_IStoveConfirmPurchaseParam_SetTxnMasterNo(IStoveConfirmPurchaseParam* self, int64_t txnMasterNo); // Non-const


    // ============================================================
    //  Function Pointer Typedefs — Flat API (Interface Accessors)
    // ============================================================

    // ===== IStoveShopCategory =====
    typedef const wchar_t* (__cdecl* Stove_IStoveShopCategory_GetCategoryId_t)(const IStoveShopCategory* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveShopCategory_GetCategoryParentId_t)(const IStoveShopCategory* self);
    typedef int32_t(__cdecl* Stove_IStoveShopCategory_GetCategoryDisplayNo_t)(const IStoveShopCategory* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveShopCategory_GetCategoryName_t)(const IStoveShopCategory* self);
    typedef int32_t(__cdecl* Stove_IStoveShopCategory_GetCategoryDepth_t)(const IStoveShopCategory* self);

    // ===== IStoveProduct =====
    typedef int64_t(__cdecl* Stove_IStoveProduct_GetProductId_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetInserviceItemId_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetProductName_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetProductDescription_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetQuantity_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetProductTypeCode_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetCategoryId_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetCategoryName_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetCurrencyCode_t)(const IStoveProduct* self);
    typedef double(__cdecl* Stove_IStoveProduct_GetPrice_t)(const IStoveProduct* self);
    typedef double(__cdecl* Stove_IStoveProduct_GetDisplayPrice_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetStrDisplayPrice_t)(const IStoveProduct* self);
    typedef double(__cdecl* Stove_IStoveProduct_GetSalePrice_t)(const IStoveProduct* self);
    typedef double(__cdecl* Stove_IStoveProduct_GetDisplaySalePrice_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetStrDisplaySalePrice_t)(const IStoveProduct* self);
    typedef bool(__cdecl* Stove_IStoveProduct_IsDiscounted_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetDiscountType_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetDiscountTypeValue_t)(const IStoveProduct* self);
    typedef int64_t(__cdecl* Stove_IStoveProduct_GetDiscountStartDate_t)(const IStoveProduct* self);
    typedef int64_t(__cdecl* Stove_IStoveProduct_GetDiscountEndDate_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetTotalQuantity_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetMemberQuantity_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetGuidQuantity_t)(const IStoveProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveProduct_GetThumbnailUrl_t)(const IStoveProduct* self);
    typedef bool(__cdecl* Stove_IStoveProduct_IsWithdrawable_t)(const IStoveProduct* self);
    typedef bool(__cdecl* Stove_IStoveProduct_HasPurchased_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetPurchaseLimitTypeCode_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetPurchaseLimitCount_t)(const IStoveProduct* self);
    typedef int32_t(__cdecl* Stove_IStoveProduct_GetSaleLimitCount_t)(const IStoveProduct* self);
    typedef int64_t(__cdecl* Stove_IStoveProduct_GetSalesStartDate_t)(const IStoveProduct* self);
    typedef int64_t(__cdecl* Stove_IStoveProduct_GetSalesEndDate_t)(const IStoveProduct* self);
    typedef int16_t(__cdecl* Stove_IStoveProduct_GetPurchaseAvailabilityCode_t)(const IStoveProduct* self);

    // ===== IStovePurchasedProduct =====
    typedef int64_t(__cdecl* Stove_IStovePurchasedProduct_GetTxnDetailNo_t)(const IStovePurchasedProduct* self);
    typedef int64_t(__cdecl* Stove_IStovePurchasedProduct_GetProductId_t)(const IStovePurchasedProduct* self);
    typedef const wchar_t* (__cdecl* Stove_IStovePurchasedProduct_GetCategoryId_t)(const IStovePurchasedProduct* self);
    typedef int32_t(__cdecl* Stove_IStovePurchasedProduct_GetTotalQuantity_t)(const IStovePurchasedProduct* self);
    typedef int32_t(__cdecl* Stove_IStovePurchasedProduct_GetMemberQuantity_t)(const IStovePurchasedProduct* self);
    typedef int32_t(__cdecl* Stove_IStovePurchasedProduct_GetGuidQuantity_t)(const IStovePurchasedProduct* self);

    // ===== IStoveChargeInfo =====
    typedef double(__cdecl* Stove_IStoveChargeInfo_GetChargeDeductVal_t)(const IStoveChargeInfo* self);
    typedef double(__cdecl* Stove_IStoveChargeInfo_GetChargeDisplayDeductVal_t)(const IStoveChargeInfo* self);
    typedef int32_t(__cdecl* Stove_IStoveChargeInfo_GetChargeType_t)(const IStoveChargeInfo* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveChargeInfo_GetChargeTypeName_t)(const IStoveChargeInfo* self);

    // ===== IStoveStartPurchaseOutcome =====
    typedef int64_t(__cdecl* Stove_IStoveStartPurchaseOutcome_GetTxnMasterNo_t)(const IStoveStartPurchaseOutcome* self);
    typedef const int64_t* (__cdecl* Stove_IStoveStartPurchaseOutcome_GetTxnDetailNos_t)(const IStoveStartPurchaseOutcome* self);
    typedef uint32_t(__cdecl* Stove_IStoveStartPurchaseOutcome_GetTxnDetailNosCount_t)(const IStoveStartPurchaseOutcome* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveStartPurchaseOutcome_GetTempPaymentUrl_t)(const IStoveStartPurchaseOutcome* self);
    typedef int32_t(__cdecl* Stove_IStoveStartPurchaseOutcome_GetPurchaseProgress_t)(const IStoveStartPurchaseOutcome* self);
    typedef bool(__cdecl* Stove_IStoveStartPurchaseOutcome_IsPurchased_t)(const IStoveStartPurchaseOutcome* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveStartPurchaseOutcome_GetExtraData_t)(const IStoveStartPurchaseOutcome* self);
    typedef uint32_t(__cdecl* Stove_IStoveStartPurchaseOutcome_GetPurchasedProductCount_t)(const IStoveStartPurchaseOutcome* self);
    typedef const IStovePurchasedProduct* (__cdecl* Stove_IStoveStartPurchaseOutcome_GetPurchasedProductAt_t)(const IStoveStartPurchaseOutcome* self, uint32_t index);
    typedef uint32_t(__cdecl* Stove_IStoveStartPurchaseOutcome_GetChargeInfoCount_t)(const IStoveStartPurchaseOutcome* self);
    typedef const IStoveChargeInfo* (__cdecl* Stove_IStoveStartPurchaseOutcome_GetChargeInfoAt_t)(const IStoveStartPurchaseOutcome* self, uint32_t index);
    typedef bool(__cdecl* Stove_IStoveStartPurchaseOutcome_IsPopupCreated_t)(const IStoveStartPurchaseOutcome* self);

    // ===== IStoveConfirmPurchaseOutcome =====
    typedef bool(__cdecl* Stove_IStoveConfirmPurchaseOutcome_IsConfirmed_t)(const IStoveConfirmPurchaseOutcome* self);
    typedef uint32_t(__cdecl* Stove_IStoveConfirmPurchaseOutcome_GetPurchasedProductCount_t)(const IStoveConfirmPurchaseOutcome* self);
    typedef const IStovePurchasedProduct* (__cdecl* Stove_IStoveConfirmPurchaseOutcome_GetPurchasedProductAt_t)(const IStoveConfirmPurchaseOutcome* self, uint32_t index);
    typedef uint32_t(__cdecl* Stove_IStoveConfirmPurchaseOutcome_GetChargeInfoCount_t)(const IStoveConfirmPurchaseOutcome* self);
    typedef const IStoveChargeInfo* (__cdecl* Stove_IStoveConfirmPurchaseOutcome_GetChargeInfoAt_t)(const IStoveConfirmPurchaseOutcome* self, uint32_t index);

    // ===== IStoveWithdrawGameOutcome =====
    typedef bool(__cdecl* Stove_IStoveWithdrawGameOutcome_IsGameWithdrawn_t)(const IStoveWithdrawGameOutcome* self);
    typedef bool(__cdecl* Stove_IStoveWithdrawGameOutcome_IsPopupCreated_t)(const IStoveWithdrawGameOutcome* self);

    // ===== IStoveTermsAgreementOutcome =====
    typedef bool(__cdecl* Stove_IStoveTermsAgreementOutcome_IsAgreed_t)(const IStoveTermsAgreementOutcome* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveTermsAgreementOutcome_GetUrl_t)(const IStoveTermsAgreementOutcome* self);
    typedef bool(__cdecl* Stove_IStoveTermsAgreementOutcome_IsPopupCreated_t)(const IStoveTermsAgreementOutcome* self);

    // ===== IStoveShopCategoryList =====
    typedef uint32_t(__cdecl* Stove_IStoveShopCategoryList_GetCount_t)(const IStoveShopCategoryList* self);
    typedef const IStoveShopCategory* (__cdecl* Stove_IStoveShopCategoryList_GetAt_t)(const IStoveShopCategoryList* self, uint32_t index);

    // ===== IStoveProductList =====
    typedef uint32_t(__cdecl* Stove_IStoveProductList_GetCount_t)(const IStoveProductList* self);
    typedef const IStoveProduct* (__cdecl* Stove_IStoveProductList_GetAt_t)(const IStoveProductList* self, uint32_t index);

    // ===== IStoveInventoryList =====
    typedef uint32_t(__cdecl* Stove_IStoveInventoryList_GetCount_t)(const IStoveInventoryList* self);
    typedef const IStoveInventoryItem* (__cdecl* Stove_IStoveInventoryList_GetAt_t)(const IStoveInventoryList* self, uint32_t index);

    // ===== IStoveInventoryItem =====
    typedef int64_t(__cdecl* Stove_IStoveInventoryItem_GetTxnMasterNo_t)(const IStoveInventoryItem* self);
    typedef int64_t(__cdecl* Stove_IStoveInventoryItem_GetTxnDetailNo_t)(const IStoveInventoryItem* self);
    typedef int64_t(__cdecl* Stove_IStoveInventoryItem_GetProductId_t)(const IStoveInventoryItem* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveInventoryItem_GetInserviceItemId_t)(const IStoveInventoryItem* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveInventoryItem_GetProductName_t)(const IStoveInventoryItem* self);
    typedef int32_t(__cdecl* Stove_IStoveInventoryItem_GetQuantity_t)(const IStoveInventoryItem* self);
    typedef const wchar_t* (__cdecl* Stove_IStoveInventoryItem_GetThumbnailUrl_t)(const IStoveInventoryItem* self);

    // ===== IStoveFetchProductsParam =====
    typedef const wchar_t* (__cdecl* Stove_IStoveFetchProductsParam_GetCategoryId_t)(const IStoveFetchProductsParam* self);
    typedef void(__cdecl* Stove_IStoveFetchProductsParam_SetCategoryId_t)(IStoveFetchProductsParam* self, const wchar_t* categoryId);

    typedef uint32_t(__cdecl* Stove_IStoveFetchProductsParam_GetPageIndex_t)(const IStoveFetchProductsParam* self);
    typedef void(__cdecl* Stove_IStoveFetchProductsParam_SetPageIndex_t)(IStoveFetchProductsParam* self, uint32_t pageIndex);

    typedef uint32_t(__cdecl* Stove_IStoveFetchProductsParam_GetPageSize_t)(const IStoveFetchProductsParam* self);
    typedef void(__cdecl* Stove_IStoveFetchProductsParam_SetPageSize_t)(IStoveFetchProductsParam* self, uint32_t pageSize);

    // ===== IStoveOrderProductParam =====
    typedef int64_t(__cdecl* Stove_IStoveOrderProductParam_GetProductId_t)(const IStoveOrderProductParam* self);
    typedef void(__cdecl* Stove_IStoveOrderProductParam_SetProductId_t)(IStoveOrderProductParam* self, int64_t productId);

    typedef double(__cdecl* Stove_IStoveOrderProductParam_GetSalePrice_t)(const IStoveOrderProductParam* self);
    typedef void(__cdecl* Stove_IStoveOrderProductParam_SetSalePrice_t)(IStoveOrderProductParam* self, double salePrice);

    typedef int32_t(__cdecl* Stove_IStoveOrderProductParam_GetQuantity_t)(const IStoveOrderProductParam* self);
    typedef void(__cdecl* Stove_IStoveOrderProductParam_SetQuantity_t)(IStoveOrderProductParam* self, int32_t quantity);

    // ===== IStovePurchaseParam =====
    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetOperation_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetOperation_t)(IStovePurchaseParam* self, int32_t operation);

    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetWebViewMode_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetWebViewMode_t)(IStovePurchaseParam* self, int32_t webViewMode);

    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetWebViewPosX_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetWebViewPosX_t)(IStovePurchaseParam* self, int32_t posX);

    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetWebViewPosY_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetWebViewPosY_t)(IStovePurchaseParam* self, int32_t posY);

    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetWebViewWidth_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetWebViewWidth_t)(IStovePurchaseParam* self, int32_t width);

    typedef int32_t(__cdecl* Stove_IStovePurchaseParam_GetWebViewHeight_t)(const IStovePurchaseParam* self);
    typedef void(__cdecl* Stove_IStovePurchaseParam_SetWebViewHeight_t)(IStovePurchaseParam* self, int32_t height);

    // ===== IStoveStartPurchaseParam =====
    typedef const IStoveOrderProductParam* const* (__cdecl* Stove_IStoveStartPurchaseParam_GetProducts_t)(const IStoveStartPurchaseParam* self);
    typedef uint32_t(__cdecl* Stove_IStoveStartPurchaseParam_GetProductsCount_t)(const IStoveStartPurchaseParam* self);
    typedef void(__cdecl* Stove_IStoveStartPurchaseParam_SetProducts_t)(IStoveStartPurchaseParam* self, IStoveOrderProductParam** products, uint32_t count);

    typedef const IStovePurchaseParam* (__cdecl* Stove_IStoveStartPurchaseParam_GetPurchaseParam_t)(const IStoveStartPurchaseParam* self);
    typedef void(__cdecl* Stove_IStoveStartPurchaseParam_SetPurchaseParam_t)(IStoveStartPurchaseParam* self, IStovePurchaseParam* param);

    typedef const wchar_t* (__cdecl* Stove_IStoveStartPurchaseParam_GetServiceTxnNo_t)(const IStoveStartPurchaseParam* self);
    typedef void(__cdecl* Stove_IStoveStartPurchaseParam_SetServiceTxnNo_t)(IStoveStartPurchaseParam* self, const wchar_t* serviceTxnNo);

    typedef const wchar_t* (__cdecl* Stove_IStoveStartPurchaseParam_GetExtraData_t)(const IStoveStartPurchaseParam* self);
    typedef void(__cdecl* Stove_IStoveStartPurchaseParam_SetExtraData_t)(IStoveStartPurchaseParam* self, const wchar_t* extraData);

    // ===== IStoveFetchTermsAgreementParam =====
    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetOperation_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetOperation_t)(IStoveFetchTermsAgreementParam* self, int32_t operation);

    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetWebViewMode_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetWebViewMode_t)(IStoveFetchTermsAgreementParam* self, int32_t webViewMode);

    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetWebViewPosX_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetWebViewPosX_t)(IStoveFetchTermsAgreementParam* self, int32_t posX);

    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetWebViewPosY_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetWebViewPosY_t)(IStoveFetchTermsAgreementParam* self, int32_t posY);

    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetWebViewWidth_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetWebViewWidth_t)(IStoveFetchTermsAgreementParam* self, int32_t width);

    typedef int32_t(__cdecl* Stove_IStoveFetchTermsAgreementParam_GetWebViewHeight_t)(const IStoveFetchTermsAgreementParam* self);
    typedef void(__cdecl* Stove_IStoveFetchTermsAgreementParam_SetWebViewHeight_t)(IStoveFetchTermsAgreementParam* self, int32_t height);

    // ===== IStoveWithdrawGameParam =====
    typedef int32_t(__cdecl* Stove_IStoveWithdrawGameParam_GetWebViewMode_t)(const IStoveWithdrawGameParam* self);
    typedef void(__cdecl* Stove_IStoveWithdrawGameParam_SetWebViewMode_t)(IStoveWithdrawGameParam* self, int32_t webViewMode);

    typedef int32_t(__cdecl* Stove_IStoveWithdrawGameParam_GetWebViewPosX_t)(const IStoveWithdrawGameParam* self);
    typedef void(__cdecl* Stove_IStoveWithdrawGameParam_SetWebViewPosX_t)(IStoveWithdrawGameParam* self, int32_t posX);

    typedef int32_t(__cdecl* Stove_IStoveWithdrawGameParam_GetWebViewPosY_t)(const IStoveWithdrawGameParam* self);
    typedef void(__cdecl* Stove_IStoveWithdrawGameParam_SetWebViewPosY_t)(IStoveWithdrawGameParam* self, int32_t posY);

    typedef int32_t(__cdecl* Stove_IStoveWithdrawGameParam_GetWebViewWidth_t)(const IStoveWithdrawGameParam* self);
    typedef void(__cdecl* Stove_IStoveWithdrawGameParam_SetWebViewWidth_t)(IStoveWithdrawGameParam* self, int32_t width);

    typedef int32_t(__cdecl* Stove_IStoveWithdrawGameParam_GetWebViewHeight_t)(const IStoveWithdrawGameParam* self);
    typedef void(__cdecl* Stove_IStoveWithdrawGameParam_SetWebViewHeight_t)(IStoveWithdrawGameParam* self, int32_t height);

    // ===== IStoveConfirmPurchaseParam =====
    typedef int64_t(__cdecl* Stove_IStoveConfirmPurchaseParam_GetTxnMasterNo_t)(const IStoveConfirmPurchaseParam* self);
    typedef void(__cdecl* Stove_IStoveConfirmPurchaseParam_SetTxnMasterNo_t)(IStoveConfirmPurchaseParam* self, int64_t txnMasterNo);

#ifdef __cplusplus
}
#endif