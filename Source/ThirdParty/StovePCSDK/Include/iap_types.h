/*
 * file: iap_types.h
 * Pure virtual interface and callback definitions used in IAPSDK.
 */

#pragma once
#include "iap_misc.h"
#include "base_types.h"


 // ============================================================
 //  IAPSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveIAPTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_IAP_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveIAPTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  Result / Data interfaces (delivered via callbacks — getter only)
// ============================================================

// ============================================================
//  IStoveShopCategory
// ============================================================

// One shop category entry. Delivered as an array to OnFetchShopCategoriesCallback
// by Stove_FetchShopCategories().
struct IStoveShopCategory : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ShopCategory);

    // Category identifier.
    virtual const wchar_t* GetCategoryId() const = 0;

    // Parent category identifier (empty for top-level categories).
    virtual const wchar_t* GetCategoryParentId() const = 0;

    // Display order of this category among its siblings.
    virtual int32_t GetCategoryDisplayNo() const = 0;

    // Localized category name.
    virtual const wchar_t* GetCategoryName() const = 0;

    // Depth in the category tree (0 = root).
    virtual int32_t GetCategoryDepth() const = 0;
};


// ============================================================
//  IStoveProduct
// ============================================================

// One product entry. Delivered as an array to OnFetchProductsCallback
// by Stove_FetchProducts().
struct IStoveProduct : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_Product);

    // Platform-unique product identifier.
    virtual int64_t GetProductId() const = 0;

    // In-service (game-side) item identifier mapped to this product.
    virtual const wchar_t* GetInserviceItemId() const = 0;

    // Localized product name.
    virtual const wchar_t* GetProductName() const = 0;

    // Localized product description.
    virtual const wchar_t* GetProductDescription() const = 0;

    // Quantity of items included per single purchase of this product.
    virtual int32_t GetQuantity() const = 0;

    // Product category code (see EStoveProductTypeCode).
    virtual int32_t GetProductTypeCode() const = 0;

    // Identifier of the shop category this product belongs to.
    virtual const wchar_t* GetCategoryId() const = 0;

    // Localized name of the shop category this product belongs to.
    virtual const wchar_t* GetCategoryName() const = 0;

    // ISO 4217 currency code (e.g., L"USD", L"KRW").
    virtual const wchar_t* GetCurrencyCode() const = 0;

    // List price used for payment processing.
    virtual double GetPrice() const = 0;

    // List price used for display (may differ from Price due to rounding).
    virtual double GetDisplayPrice() const = 0;

    // Pre-formatted display list-price string (includes currency formatting).
    virtual const wchar_t* GetStrDisplayPrice() const = 0;

    // Sale price used for payment processing (same as Price when not on sale).
    virtual double GetSalePrice() const = 0;

    // Sale price used for display.
    virtual double GetDisplaySalePrice() const = 0;

    // Pre-formatted display sale-price string.
    virtual const wchar_t* GetStrDisplaySalePrice() const = 0;

    // True if the product is currently on sale (discounted).
    virtual bool IsDiscounted() const = 0;

    // Discount calculation type (see EStoveDiscountType:
    // None / FixedRate / FlatRate).
    virtual int32_t GetDiscountType() const = 0;

    // Discount amount — interpretation depends on DiscountType:
    //   FixedRate -> percentage (e.g., 10 means 10% off)
    //   FlatRate  -> flat amount in the product's currency
    virtual int32_t GetDiscountTypeValue() const = 0;

    // Discount start time (epoch milliseconds, UTC).
    virtual int64_t GetDiscountStartDate() const = 0;

    // Discount end time (epoch milliseconds, UTC).
    virtual int64_t GetDiscountEndDate() const = 0;

    // Total quantity purchased for this product across all dimensions.
    virtual int32_t GetTotalQuantity() const = 0;

    // Quantity purchased by the signed-in member (account scope).
    virtual int32_t GetMemberQuantity() const = 0;

    // Quantity purchased tied to the current character GUID (character scope).
    virtual int32_t GetGuidQuantity() const = 0;

    // URL of the product's representative thumbnail image.
    virtual const wchar_t* GetThumbnailUrl() const = 0;

    // True if the product is subject to "right-of-withdrawal" rules
    // (consumer-protection refund eligibility).
    virtual bool IsWithdrawable() const = 0;

    // True if the user has at least one purchase history for this product.
    virtual bool HasPurchased() const = 0;

    // Purchase-limit policy (see EStovePurchaseLimitTypeCode).
    virtual int32_t GetPurchaseLimitTypeCode() const = 0;

    // Purchase-count limit under the current policy.
    virtual int32_t GetPurchaseLimitCount() const = 0;

    // Total sales-count limit for the product (0 means unlimited).
    virtual int32_t GetSaleLimitCount() const = 0;

    // Sales-period start time (epoch milliseconds, UTC).
    virtual int64_t GetSalesStartDate() const = 0;

    // Sales-period end time (epoch milliseconds, UTC).
    virtual int64_t GetSalesEndDate() const = 0;

    // Purchase availability code.
    //   1 : Purchasable
    //   2 : Not purchasable (purchase limit exceeded)
    //   3 : Not purchasable (sold out, web store)
    //   4 : Not purchasable (already claimed/received, web store)
    // The value set may be extended by the server (additional codes may be defined later).
    // Reflects a single-product purchase request; the result may differ when requesting 2 or more products at once.
    // int16_t because the server API schema types this field (and other code fields) as Int16.
    virtual int16_t GetPurchaseAvailabilityCode() const = 0;
};


// ============================================================
//  IStovePurchasedProduct
// ============================================================

// One product entry within a completed purchase. Delivered as an array
// to OnConfirmPurchaseCallback and embedded inside IStovePurchaseResult.
struct IStovePurchasedProduct : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_PurchasedProduct);

    // Transaction-detail number (per-product TID within a master transaction).
    virtual int64_t GetTxnDetailNo() const = 0;

    // Platform-unique product identifier.
    virtual int64_t GetProductId() const = 0;

    // Identifier of the shop category this product belongs to.
    virtual const wchar_t* GetCategoryId() const = 0;

    // Total quantity purchased for this product.
    virtual int32_t GetTotalQuantity() const = 0;

    // Quantity purchased at the member (account) scope.
    virtual int32_t GetMemberQuantity() const = 0;

    // Quantity purchased tied to the current character GUID.
    virtual int32_t GetGuidQuantity() const = 0;
};


// ============================================================
//  IStoveChargeInfo
// ============================================================

// One currency/charge entry used in a purchase (e.g., Stove Cash, points,
// PG). Delivered as an array inside IStovePurchaseResult and to
// OnConfirmPurchaseCallback.
struct IStoveChargeInfo : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ChargeInfo);

    // Amount actually deducted at payment time (in payment units).
    virtual double GetChargeDeductVal() const = 0;

    // Cash-equivalent display value of the deducted amount.
    virtual double GetChargeDisplayDeductVal() const = 0;

    // Payment-method code (charge_type, Int32).
    //   2      : Coupon
    //   98     : Stove Cash
    //   99     : Point
    //   Others : PG (external) payment methods
    // Only 2/98/99 are fixed reserved values; other PG codes are server-driven
    // and may be extended. Use GetChargeTypeName() for the human-readable name.
    virtual int32_t GetChargeType() const = 0;

    // Localized payment-method name.
    virtual const wchar_t* GetChargeTypeName() const = 0;
};


// ============================================================
//  IStoveStartPurchaseOutcome
// ============================================================

// Outcome of Stove_StartPurchase(). Delivered to OnStartPurchaseCallback.
// Which fields are populated depends on the EStovePurchaseOperation used:
//   Default                       -> TempPaymentUrl is provided for manual checkout.
//   WithWebViewAndConfirmResult   -> Purchased, PurchasedProducts, ChargeInfos are
//                                    populated on successful payment.
struct IStoveStartPurchaseOutcome : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_StartPurchaseOutcome);

    // Transaction-master number (per-purchase TID).
    virtual int64_t GetTxnMasterNo() const = 0;

    // Pointer to the array of per-product transaction-detail numbers.
    // The array is owned by the SDK and is valid only for the duration of
    // the callback; copy the values you need to retain.
    virtual const int64_t* GetTxnDetailNos() const = 0;

    // Number of elements pointed to by GetTxnDetailNos().
    virtual uint32_t GetTxnDetailNosCount() const = 0;

    // One-time payment URL. Provided when operation == Default.
    virtual const wchar_t* GetTempPaymentUrl() const = 0;

    // Purchase-progress state (see EStovePurchaseProgress).
    virtual int32_t GetPurchaseProgress() const = 0;

    // True when operation == WithWebViewAndConfirmResult and payment
    // completed successfully; false otherwise.
    virtual bool IsPurchased() const = 0;

    // Echo of the ExtraData string passed to Stove_StartPurchase().
    virtual const wchar_t* GetExtraData() const = 0;

    // Number of purchased product entries.
    // Populated when operation == WithWebViewAndConfirmResult and
    // payment succeeded.
    virtual uint32_t GetPurchasedProductCount() const = 0;

    // Returns the purchased product at position index (0-based).
    // Returns nullptr if index >= GetPurchasedProductCount().
    virtual const IStovePurchasedProduct* GetPurchasedProductAt(uint32_t index) const = 0;

    // Number of charge-info entries describing the currencies used for payment.
    virtual uint32_t GetChargeInfoCount() const = 0;

    // Returns the charge-info entry at position index (0-based).
    // Returns nullptr if index >= GetChargeInfoCount().
    virtual const IStoveChargeInfo* GetChargeInfoAt(uint32_t index) const = 0;

    // Whether this call created at least one webview. Valid regardless of
    // callbackResult's success/failure -- always read it. false means onDestroy
    // will never be delivered for this call -- the caller must release any
    // onDestroy-side callback state right here instead of waiting for it.
    // (Declared last on purpose: appending a method keeps the vtable slots of
    //  the members above stable for already-compiled integrations.)
    virtual bool IsPopupCreated() const = 0;
};


// ============================================================
//  IStoveConfirmPurchaseOutcome
// ============================================================

// Outcome of Stove_ConfirmPurchase(). Delivered to OnConfirmPurchaseCallback.
// Wraps the status flag and the two result arrays (purchased products and charge
// infos) that were previously passed as flat arguments.
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveConfirmPurchaseOutcome : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ConfirmPurchaseOutcome);

    // True if the purchase finalized successfully.
    virtual bool IsConfirmed() const = 0;

    // Number of purchased product entries.
    virtual uint32_t GetPurchasedProductCount() const = 0;

    // Returns the purchased product at position index (0-based).
    // Returns nullptr if index >= GetPurchasedProductCount().
    virtual const IStovePurchasedProduct* GetPurchasedProductAt(uint32_t index) const = 0;

    // Number of charge-info entries.
    virtual uint32_t GetChargeInfoCount() const = 0;

    // Returns the charge-info entry at position index (0-based).
    // Returns nullptr if index >= GetChargeInfoCount().
    virtual const IStoveChargeInfo* GetChargeInfoAt(uint32_t index) const = 0;
};


// ============================================================
//  IStoveWithdrawGameOutcome
// ============================================================

// Outcome of Stove_WithdrawGame(). Delivered to OnWithdrawGameCallback.
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveWithdrawGameOutcome : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_WithdrawGameOutcome);

    // True if the user completed the game-withdrawal flow.
    virtual bool IsGameWithdrawn() const = 0;

    // Whether this call created at least one webview. Valid regardless of
    // callbackResult's success/failure -- always read it. false means onDestroy
    // will never be delivered for this call -- the caller must release any
    // onDestroy-side callback state right here instead of waiting for it.
    // (Declared last on purpose: appending a method keeps the vtable slots of
    //  the members above stable for already-compiled integrations.)
    virtual bool IsPopupCreated() const = 0;
};


// ============================================================
//  IStoveTermsAgreementOutcome
// ============================================================

// Outcome of Stove_FetchTermsAgreement(). Delivered to OnFetchTermsAgreementCallback.
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveTermsAgreementOutcome : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_TermsAgreementOutcome);

    // True if the user has already agreed to the current terms;
    // in that case GetUrl() returns an empty string.
    virtual bool IsAgreed() const = 0;

    // URL of the terms page the caller should open (or has already opened
    // via webview). Empty when IsAgreed() is true.
    virtual const wchar_t* GetUrl() const = 0;

    // Whether this call created at least one webview. Valid regardless of
    // callbackResult's success/failure -- always read it. false means onDestroy
    // will never be delivered for this call -- the caller must release any
    // onDestroy-side callback state right here instead of waiting for it.
    // (Declared last on purpose: appending a method keeps the vtable slots of
    //  the members above stable for already-compiled integrations.)
    virtual bool IsPopupCreated() const = 0;
};


// ============================================================
//  IStoveShopCategoryList
// ============================================================

// Container delivered by OnFetchShopCategoriesCallback.
// Wraps the shop-category array returned by Stove_FetchShopCategories().
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveShopCategoryList : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ShopCategoryList);

    // Returns the number of shop categories in the result.
    virtual uint32_t GetCount() const = 0;

    // Returns the shop category at position index (0-based).
    // Returns nullptr if index >= GetCount().
    virtual const IStoveShopCategory* GetAt(uint32_t index) const = 0;
};


// ============================================================
//  IStoveProductList
// ============================================================

// Container delivered by OnFetchProductsCallback.
// Wraps the product array returned by Stove_FetchProducts().
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveProductList : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ProductList);

    // Returns the number of products in the result.
    virtual uint32_t GetCount() const = 0;

    // Returns the product at position index (0-based).
    // Returns nullptr if index >= GetCount().
    virtual const IStoveProduct* GetAt(uint32_t index) const = 0;
};


// ============================================================
//  IStoveInventoryItem
// ============================================================

// One purchase-history entry. Delivered as an array to
// OnFetchInventoryCallback by Stove_FetchInventory().
struct IStoveInventoryItem : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_InventoryItem);

    // Transaction-master number.
    virtual int64_t GetTxnMasterNo() const = 0;

    // Transaction-detail number (per-product TID).
    virtual int64_t GetTxnDetailNo() const = 0;

    // Platform-unique product identifier.
    virtual int64_t GetProductId() const = 0;

    // In-service (game-side) item identifier mapped to this product.
    virtual const wchar_t* GetInserviceItemId() const = 0;

    // Localized product name.
    virtual const wchar_t* GetProductName() const = 0;

    // Purchased quantity.
    virtual int32_t GetQuantity() const = 0;

    // URL of the product's representative thumbnail image.
    virtual const wchar_t* GetThumbnailUrl() const = 0;
};


// ============================================================
//  IStoveInventoryList
// ============================================================

// Container delivered by OnFetchInventoryCallback.
// Wraps the inventory array returned by Stove_FetchInventory().
// The object is SDK-owned and valid only for the duration of the callback;
// do NOT call Destroy() on it.
struct IStoveInventoryList : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_InventoryList);

    // Returns the number of inventory items in the result.
    virtual uint32_t GetCount() const = 0;

    // Returns the inventory item at position index (0-based).
    // Returns nullptr if index >= GetCount().
    virtual const IStoveInventoryItem* GetAt(uint32_t index) const = 0;
};


// ============================================================
//  Parameter interfaces (created by the caller — get/set pairs adjacent)
// ============================================================

// ============================================================
//  IStoveFetchProductsParam
// ============================================================

// Parameters for Stove_FetchProducts().
// Create via Stove_CreateParam(k_EStoveIAPTypeKind_FetchProductsParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveFetchProductsParam : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_FetchProductsParam);

    // Category ID filter. Leave empty to retrieve products from all categories.
    virtual const wchar_t* GetCategoryId() const = 0;
    virtual void SetCategoryId(const wchar_t* categoryId) = 0;

    // Page number for paginated retrieval (1-based; matches the page_no value sent to the server).
    virtual uint32_t GetPageIndex() const = 0;
    virtual void SetPageIndex(uint32_t pageIndex) = 0;

    // Page size (number of products per page).
    virtual uint32_t GetPageSize() const = 0;
    virtual void SetPageSize(uint32_t pageSize) = 0;
};


// ============================================================
//  IStoveOrderProductParam
// ============================================================

// Per-product order entry used by IStoveStartPurchaseParam.
// Create via Stove_CreateParam(k_EStoveIAPTypeKind_OrderProductParam),
// populate the fields via setters, then Destroy() after the purchase call
// returns (or after the parent StartPurchaseParam is destroyed).
struct IStoveOrderProductParam : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_OrderProductParam);

    // Platform-unique product identifier (matches IStoveProduct::GetProductId()).
    virtual int64_t GetProductId() const = 0;
    virtual void SetProductId(int64_t productId) = 0;

    // Unit sale price the client observed for this product
    // (used by the backend to detect client/server price mismatches).
    virtual double GetSalePrice() const = 0;
    virtual void SetSalePrice(double salePrice) = 0;

    // Quantity to purchase.
    virtual int32_t GetQuantity() const = 0;
    virtual void SetQuantity(int32_t quantity) = 0;
};


// ============================================================
//  IStoveWebViewLayoutParam
// ============================================================

// Common webview-placement fields shared by IStovePurchaseParam,
// IStoveFetchTermsAgreementParam, and IStoveWithdrawGameParam.
// Not a standalone Stove_CreateParam() kind -- always used as the base
// of one of those concrete param interfaces.
struct IStoveWebViewLayoutParam : IStoveTypeBase
{
    // Webview display mode (see EStoveWebViewMode: External / Internal).
    virtual int32_t GetWebViewMode() const = 0;
    virtual void SetWebViewMode(int32_t webViewMode) = 0;

    // Webview x position (pixels).
    virtual int32_t GetWebViewPosX() const = 0;
    virtual void SetWebViewPosX(int32_t posX) = 0;

    // Webview y position (pixels).
    virtual int32_t GetWebViewPosY() const = 0;
    virtual void SetWebViewPosY(int32_t posY) = 0;

    // Webview width (pixels).
    virtual int32_t GetWebViewWidth() const = 0;
    virtual void SetWebViewWidth(int32_t width) = 0;

    // Webview height (pixels).
    virtual int32_t GetWebViewHeight() const = 0;
    virtual void SetWebViewHeight(int32_t height) = 0;
};


// ============================================================
//  IStovePurchaseParam
// ============================================================

// Behavior and webview-placement param for Stove_StartPurchase().
// All WebView* fields (inherited from IStoveWebViewLayoutParam) apply
// only when Operation != Default.
struct IStovePurchaseParam : IStoveWebViewLayoutParam
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_PurchaseParam);

    // Behavior selector (see EStovePurchaseOperation).
    virtual int32_t GetOperation() const = 0;
    virtual void SetOperation(int32_t operation) = 0;
};


// ============================================================
//  IStoveStartPurchaseParam
// ============================================================

// Parameters for Stove_StartPurchase().
// Create via Stove_CreateParam(k_EStoveIAPTypeKind_StartPurchaseParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveStartPurchaseParam : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_StartPurchaseParam);

    // Array of per-product order entries to purchase.
    // SetProducts() takes ownership semantics: the caller retains ownership
    // of the passed IStoveOrderProductParam objects and must Destroy() them
    // after the purchase call returns.
    // Ownership note: destroying this parent param (via Destroy()) does NOT
    // destroy the child IStoveOrderProductParam objects passed to SetProducts() --
    // the caller must Destroy() each of them individually. Stove_StartPurchase()
    // reads and copies every field it needs out of these children synchronously,
    // before the call returns, so it is safe for the caller to Destroy() each
    // child (and this parent param) any time after Stove_StartPurchase() returns;
    // no reference to them is retained across the async callback.
    virtual const IStoveOrderProductParam* const* GetProducts() const = 0;
    virtual uint32_t GetProductsCount() const = 0;
    virtual void SetProducts(IStoveOrderProductParam** products, uint32_t count) = 0;

    // Purchase-behavior param. The caller retains ownership of the option
    // object and must Destroy() it after the purchase call returns.
    // Ownership note: destroying this parent param does NOT destroy the child
    // IStovePurchaseParam passed to SetPurchaseParam() -- the caller must
    // Destroy() it separately. As with SetProducts(), Stove_StartPurchase()
    // copies the needed fields out of it synchronously before returning, so
    // Destroy()'ing it any time after Stove_StartPurchase() returns is safe.
    virtual const IStovePurchaseParam* GetPurchaseParam() const = 0;
    virtual void SetPurchaseParam(IStovePurchaseParam* param) = 0;

    // Game-issued service-side transaction number (optional).
    virtual const wchar_t* GetServiceTxnNo() const = 0;
    virtual void SetServiceTxnNo(const wchar_t* serviceTxnNo) = 0;

    // Additional request data, typically a JSON string (optional).
    // Echoed back via IStovePurchaseResult::GetExtraData().
    virtual const wchar_t* GetExtraData() const = 0;
    virtual void SetExtraData(const wchar_t* extraData) = 0;
};


// ============================================================
//  IStoveFetchTermsAgreementParam
// ============================================================

// Behavior and webview-placement params for Stove_FetchTermsAgreement().
// All WebView* fields (inherited from IStoveWebViewLayoutParam) apply
// only when Operation != Default.
struct IStoveFetchTermsAgreementParam : IStoveWebViewLayoutParam
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_FetchTermsAgreementParam);

    // Behavior selector (see EStoveTermsOperation).
    virtual int32_t GetOperation() const = 0;
    virtual void SetOperation(int32_t operation) = 0;
};


// ============================================================
//  IStoveWithdrawGameParam
// ============================================================

// Webview-placement params (inherited from IStoveWebViewLayoutParam) for
// Stove_WithdrawGame() (game-withdrawal / account detach flow).
struct IStoveWithdrawGameParam : IStoveWebViewLayoutParam
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_WithdrawGameParam);
};


// ============================================================
//  IStoveConfirmPurchaseParam
// ============================================================

// Parameters for Stove_ConfirmPurchase().
struct IStoveConfirmPurchaseParam : IStoveTypeBase
{
    STOVE_IAP_TYPE_KIND(k_EStoveIAPTypeKind_ConfirmPurchaseParam);

    // Transaction-master number from IStovePurchaseResult.
    virtual int64_t GetTxnMasterNo() const = 0;
    virtual void SetTxnMasterNo(int64_t txnMasterNo) = 0;
};

#else

typedef struct IStoveShopCategory IStoveShopCategory;
typedef struct IStoveProduct IStoveProduct;
typedef struct IStovePurchasedProduct IStovePurchasedProduct;
typedef struct IStoveChargeInfo IStoveChargeInfo;
typedef struct IStoveStartPurchaseOutcome IStoveStartPurchaseOutcome;
typedef struct IStoveConfirmPurchaseOutcome IStoveConfirmPurchaseOutcome;
typedef struct IStoveWithdrawGameOutcome IStoveWithdrawGameOutcome;
typedef struct IStoveTermsAgreementOutcome IStoveTermsAgreementOutcome;
typedef struct IStoveShopCategoryList IStoveShopCategoryList;
typedef struct IStoveProductList IStoveProductList;
typedef struct IStoveInventoryList IStoveInventoryList;
typedef struct IStoveInventoryItem IStoveInventoryItem;
typedef struct IStoveFetchProductsParam IStoveFetchProductsParam;
typedef struct IStoveOrderProductParam IStoveOrderProductParam;
typedef struct IStoveWebViewLayoutParam IStoveWebViewLayoutParam;
typedef struct IStovePurchaseParam IStovePurchaseParam;
typedef struct IStoveStartPurchaseParam IStoveStartPurchaseParam;
typedef struct IStoveFetchTermsAgreementParam IStoveFetchTermsAgreementParam;
typedef struct IStoveWithdrawGameParam IStoveWithdrawGameParam;
typedef struct IStoveConfirmPurchaseParam IStoveConfirmPurchaseParam;

#endif // __cplusplus


// ============================================================
//  IAPSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and any accompanying object / array pointers
// are owned by the SDK and are valid only for the duration of the callback
// invocation; do NOT call Destroy() on them and do NOT retain pointers
// beyond the callback — copy any values you need to keep.

// Delivered by Stove_FetchShopCategories().
typedef void(__cdecl* OnFetchShopCategoriesCallback)(const IStoveCallbackResult* callbackResult, const IStoveShopCategoryList* list);

// Delivered by Stove_FetchProducts().
typedef void(__cdecl* OnFetchProductsCallback)(const IStoveCallbackResult* callbackResult, const IStoveProductList* list);

// Delivered by Stove_StartPurchase().
typedef void(__cdecl* OnStartPurchaseCallback)(const IStoveCallbackResult* callbackResult, const IStoveStartPurchaseOutcome* outcome);

// Delivered by Stove_ConfirmPurchase().
typedef void(__cdecl* OnConfirmPurchaseCallback)(const IStoveCallbackResult* callbackResult, const IStoveConfirmPurchaseOutcome* outcome);

// Delivered by Stove_FetchInventory().
typedef void(__cdecl* OnFetchInventoryCallback)(const IStoveCallbackResult* callbackResult, const IStoveInventoryList* list);

// Delivered by Stove_FetchTermsAgreement().
typedef void(__cdecl* OnFetchTermsAgreementCallback)(const IStoveCallbackResult* callbackResult, const IStoveTermsAgreementOutcome* outcome);

// Delivered when all IAPSDK-created popups have been destroyed.
// Fires after every popup spawned by the associated API call has closed.
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnIAPPopupDestroyCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);

// Delivered by Stove_WithdrawGame().
typedef void(__cdecl* OnWithdrawGameCallback)(const IStoveCallbackResult* callbackResult, const IStoveWithdrawGameOutcome* outcome);