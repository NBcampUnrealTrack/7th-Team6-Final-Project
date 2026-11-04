/*
 * file: iap_misc.h
 * Enum definitions used in IAPSDK.
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <wchar.h>


 // ============================================================
 //  IAPSDK enums
 // ============================================================

 // Identifies the concrete type of an IStoveTypeBase instance owned by IAPSDK.
 // Used by Stove_CreateParam() to request a specific parameter object,
 // and by IStoveTypeBase::GetTypeKind() for runtime type identification.
 // Range (2000 ~ 2999)
typedef enum EStoveIAPTypeKind
{
    k_EStoveIAPTypeKind_Invalid = -1,

    // Result / Data types (delivered by the SDK via callbacks)
    k_EStoveIAPTypeKind_ShopCategory = 2000,
    k_EStoveIAPTypeKind_Product = 2001,
    k_EStoveIAPTypeKind_StartPurchaseOutcome = 2002,      // renamed from PurchaseResult
    k_EStoveIAPTypeKind_PurchasedProduct = 2003,
    k_EStoveIAPTypeKind_ChargeInfo = 2004,
    k_EStoveIAPTypeKind_InventoryItem = 2005,
    // 2006 reserved: moved to Internal/C (internal_iap_types.h)
    k_EStoveIAPTypeKind_ConfirmPurchaseOutcome = 2008,
    k_EStoveIAPTypeKind_WithdrawGameOutcome = 2009,
    k_EStoveIAPTypeKind_TermsAgreementOutcome = 2010,
    k_EStoveIAPTypeKind_ShopCategoryList = 2011,
    k_EStoveIAPTypeKind_ProductList = 2012,
    k_EStoveIAPTypeKind_InventoryList = 2013,

    // Parameter types (created by the caller via Stove_CreateParam)
    k_EStoveIAPTypeKind_FetchProductsParam = 2500,
    k_EStoveIAPTypeKind_OrderProductParam = 2501,
    k_EStoveIAPTypeKind_PurchaseParam = 2502,
    k_EStoveIAPTypeKind_StartPurchaseParam = 2503,
    k_EStoveIAPTypeKind_FetchTermsAgreementParam = 2504,
    // 2505 reserved: moved to Internal/C (internal_iap_types.h)
    k_EStoveIAPTypeKind_WithdrawGameParam = 2506,
    k_EStoveIAPTypeKind_ConfirmPurchaseParam = 2507,

    k_EStoveIAPTypeKind_Max = 0x7fffffff
} EStoveIAPTypeKind;


// Identifies which IAPSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
// Values in 2500~2999 are an internal diagnostic band and are not part of this enum.
typedef enum EStoveIAPMethodCode
{
    k_EStoveIAPMethodCode_Invalid = -1,

    // Public APIs. Module block 2000~2999 (sequential).
    k_EStoveIAPMethodCode_FetchShopCategories = 2000,
    k_EStoveIAPMethodCode_FetchProducts = 2001,
    k_EStoveIAPMethodCode_StartPurchase = 2002,
    k_EStoveIAPMethodCode_ConfirmPurchase = 2003,
    k_EStoveIAPMethodCode_FetchInventory = 2004,
    k_EStoveIAPMethodCode_FetchTermsAgreement = 2005,
    k_EStoveIAPMethodCode_WithdrawGame = 2006,

    k_EStoveIAPMethodCode_Max = 0x7fffffff
} EStoveIAPMethodCode;

// Result codes are global: common 0~299 (EStoveCommonResultCode),
// module-specific 300~ (EStoveResultCode) — both in base_misc.h.


// Behavior mode for Stove_StartPurchase().
// Selects between manual URL flow, Stove-hosted webview, and
// webview + auto-confirm.
typedef enum EStovePurchaseOperation
{
    // Default (no Stove Webview). The caller opens the Stove web payment
    // page manually using the one-time URL returned in the purchase result,
    // then calls Stove_ConfirmPurchase() to finalize the purchase.
    k_EStovePurchaseOperation_Default = 0,

    // Opens the Stove web payment page inside the Stove Webview.
    // The caller must still call Stove_ConfirmPurchase() after payment
    // to finalize the purchase.
    k_EStovePurchaseOperation_WithWebView = 1,

    // Opens the Stove web payment page inside the Stove Webview AND
    // automatically calls Stove_ConfirmPurchase() on success,
    // returning the final confirmed purchase result.
    k_EStovePurchaseOperation_WithWebViewAndConfirmResult = 2,

    k_EStovePurchaseOperation_Max = 0x7fffffff
} EStovePurchaseOperation;


// Behavior mode for Stove_FetchTermsAgreement().
typedef enum EStoveTermsOperation
{
    // Default (no Stove Webview). The caller opens the terms page manually
    // using the one-time URL returned in the result.
    k_EStoveTermsOperation_Default = 0,

    // Opens the terms agreement page inside the Stove Webview.
    k_EStoveTermsOperation_WithWebView = 1,

    k_EStoveTermsOperation_Max = 0x7fffffff
} EStoveTermsOperation;


// Product category code.
// Returned by IStoveProduct::GetProductTypeCode().
typedef enum EStoveProductTypeCode
{
    k_EStoveProductTypeCode_None = 0,                // Unspecified
    k_EStoveProductTypeCode_IndiePackageGameItem = 1, // Indie package game item
    k_EStoveProductTypeCode_InGameItem = 2,          // In-game item
    k_EStoveProductTypeCode_PackageItem = 3,         // Package item

    k_EStoveProductTypeCode_Max = 0x7fffffff
} EStoveProductTypeCode;


// Discount calculation type.
// Returned by IStoveProduct::GetDiscountType().
typedef enum EStoveDiscountType
{
    k_EStoveDiscountType_None = 0,       // No discount
    k_EStoveDiscountType_FixedRate = 1,  // Percentage discount (e.g., 10% off)
    k_EStoveDiscountType_FlatRate = 2,   // Fixed-amount discount (e.g., $2 off)

    k_EStoveDiscountType_Max = 0x7fffffff
} EStoveDiscountType;


// Purchase-limit policy for a product.
// Returned by IStoveProduct::GetPurchaseLimitTypeCode().
typedef enum EStovePurchaseLimitTypeCode
{
    k_EStovePurchaseLimitTypeCode_None = 0,      // No limit policy defined
    k_EStovePurchaseLimitTypeCode_Unlimited = 1, // Unlimited purchases
    k_EStovePurchaseLimitTypeCode_Member = 2,    // Per-member limit
    k_EStovePurchaseLimitTypeCode_Character = 3, // Per-character limit

    k_EStovePurchaseLimitTypeCode_Max = 0x7fffffff
} EStovePurchaseLimitTypeCode;


// Progress state returned by Stove_StartPurchase().
// Indicates whether the caller still needs to open a payment window.
typedef enum EStovePurchaseProgress
{
    k_EStovePurchaseProgress_None = 0,

    // A payment window must be opened by the caller using the one-time
    // payment URL in the purchase result.
    k_EStovePurchaseProgress_NeedPaymentWindow = 1,

    // No manual payment window is required — either the purchase completed
    // at zero cost, or the Webview-based flow already handled the payment.
    k_EStovePurchaseProgress_NotNeedPaymentWindow = 2,

    k_EStovePurchaseProgress_Max = 0x7fffffff
} EStovePurchaseProgress;