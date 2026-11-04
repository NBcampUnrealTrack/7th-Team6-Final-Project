/*
 * file: base_types.h
 * Pure virtual interface and callback definitions used in BaseSDK.
 */

#pragma once
#include "base_misc.h"


 // ============================================================
 //  BaseSDK Types
 // ============================================================

#ifdef __cplusplus

// Attaches a compile-time EStoveBaseTypeKind to an interface struct.
// Exposes k_eStaticTypeKind so templates and casts can resolve a type's kind
// without an instance (e.g., for static_assert or type dispatch).

#define STOVE_BASE_TYPE_KIND(EnumValue) \
    static STOVE_CONSTEXPR EStoveBaseTypeKind k_eStaticTypeKind = EnumValue


// ============================================================
//  IStoveTypeBase
// ============================================================

// Root interface of every SDK-owned object crossing the DLL boundary.
// Provides runtime type identification and an explicit Destroy() lifecycle
// (the caller must call Destroy() on any object it received ownership of).
// QueryExt(extId) is reserved for future extension interfaces.
struct IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_Base);

    // Returns the concrete type kind (one of EStoveBaseTypeKind).
    virtual int GetTypeKind() const = 0;

    // Returns true if the caller is responsible for calling Destroy().
    // Useful for objects whose lifetime is sometimes managed by the SDK.
    // Check this before calling Destroy():
    //   true  : the caller owns this instance (e.g. an object returned via an
    //           out-parameter or a Get*() API) -- call Destroy() exactly once
    //           when done with it.
    //   false : the SDK owns and manages this instance's lifetime (e.g. an
    //           object delivered as a callback argument). It is valid only
    //           for the duration of that call -- the caller must NOT call
    //           Destroy() on it.
    virtual bool ShouldDestroy() const = 0;

    // Releases the object. Must be called exactly once per owned instance.
    virtual void Destroy() = 0;

    // Returns an optional extension pointer; may be nullptr.
    // 'extId' identifies which extension is being requested:
    //   0           : reserved (returns nullptr in the current SDK)
    //   1..0xFFFF   : reserved for future BaseSDK-defined extensions
    //   0x10000+    : SDK-module-defined extension ids
    // Unknown ids must return nullptr.
    virtual void* QueryExt(uint32_t extId) const = 0;
};


// ============================================================
//  IStoveResult
// ============================================================

// Result of a synchronous BaseSDK call.
// Carries the originating method code and a result code
// (see EStoveCommonResultCode / EStoveResultCode).
// Returned objects must be released via Destroy().
struct IStoveResult : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveResult);

    // Globally-unique method code identifying which API produced this result.
    // Encodes the originating module as a 1000-block (see EStove<Module>MethodCode).
    virtual uint32_t GetMethodCode() const = 0;

    // Result code; 0 (Success) on success, non-zero on failure.
    // Common codes: EStoveCommonResultCode (0~299). Module-specific: EStoveResultCode (300~).
    virtual uint32_t GetResultCode() const = 0;

    // 0 (Success) on success, non-zero on failure.
    virtual bool IsSuccessful() const = 0;
};


// ============================================================
//  IStoveCallbackResult
// ============================================================

// Result delivered to an asynchronous callback.
// Wraps an IStoveResult plus optional error context and the userData
// pointer originally passed to the async API.
// The lifetime of the contained pointers is scoped to the callback invocation.
struct IStoveCallbackResult : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveCallbackResult);

    // Underlying result object (valid only for the duration of the callback).
    virtual IStoveResult* GetResult() const = 0;

    // Detailed message describing why the error occurred.
    virtual const wchar_t* GetErrorMessage() const = 0;

    // External error value (HTTP error code or API result code).
    virtual int32_t GetExternalError() const = 0;

    // The userData pointer originally passed by the caller.
    virtual void* GetUserData() const = 0;
};


// ============================================================
//  IStovePopupCreationInfo
// ============================================================

// Webview-creation signal delivered as the onFinished payload of the View popup
// APIs (OnViewPopupCallback). Not a standalone Stove_CreateParam() kind -- the
// SDK owns the object and it is valid only for the duration of the callback.
//
// IAP's IStove*Outcome types expose the same information through their own
// IsPopupCreated(), declared at the end of each interface rather than inherited
// from here: appending a method preserves the vtable slots of the existing
// members, whereas inserting a base class would shift them and break
// already-compiled integrations.
struct IStovePopupCreationInfo : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_PopupCreationInfo);

    // Whether this call created at least one webview. Valid regardless of
    // callbackResult's success/failure -- always read it. false means onDestroy
    // will never be delivered for this call -- the caller must release any
    // onDestroy-side callback state right here instead of waiting for it.
    virtual bool IsPopupCreated() const = 0;
};


// ============================================================
//  IStoveRestartAppIfNecessaryParam
// ============================================================

// Parameters for Stove_RestartAppIfNecessary().
// Create via Stove_CreateParam(k_EStoveBaseTypeKind_RestartAppIfNecessaryParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveRestartAppIfNecessaryParam : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_RestartAppIfNecessaryParam);

    // Stove environment value.
    virtual const wchar_t* GetEnvironment() const = 0;
    virtual void SetEnvironment(const wchar_t* environment) = 0;

    // Stove game ID value.
    virtual const wchar_t* GetGameId() const = 0;
    virtual void SetGameId(const wchar_t* gameId) = 0;

    // Stove application key value.
    virtual const wchar_t* GetAppKey() const = 0;
    virtual void SetAppKey(const wchar_t* appKey) = 0;

    // Wait time to check whether the app was launched from the launcher (milliseconds).
    virtual uint32_t GetWaitTimeMilliSec() const = 0;
    virtual void SetWaitTimeMilliSec(uint32_t waitTimeMillisec) = 0;

    // Whether to launch the Stove launcher if it is not already running.
    virtual bool GetLaunchStoveLauncher() const = 0;
    virtual void SetLaunchStoveLauncher(bool bLaunchStoveLauncher) = 0;

    // Platform name (e.g., L"Stove", L"Steam"...).
    virtual const wchar_t* GetPlatformName() const = 0;
    virtual void SetPlatformName(const wchar_t* platformName) = 0;
};


// ============================================================
//  IStoveInitializeParam
// ============================================================

// Parameters for Stove_Initialize().
// Create via Stove_CreateParam(k_EStoveBaseTypeKind_InitializeParam),
// populate the fields via setters, then Destroy() after the call returns.
struct IStoveInitializeParam : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_InitializeParam);

    // IAP shop key. Set the shop key here; when non-empty,
    // Stove_Initialize() initializes the IAP module with it. Leave empty to skip IAP.
    virtual const wchar_t* GetShopKey() const = 0;
    virtual void SetShopKey(const wchar_t* shopKey) = 0;

    // Game main window handle (HWND as const void*). Set the main window
    // handle here (valid only after the window exists); when non-null, Stove_Initialize()
    // initializes the View module (and, with a shop key, the IAP module) using it.
    // Leave null to skip View/IAP setup.
    virtual const void* GetMainWndHandle() const = 0;
    virtual void SetMainWndHandle(const void* mainWndHandle) = 0;
};


// ============================================================
//  IStoveAccessToken
// ============================================================

// Token structure of the callback received when calling the AccessTokenRenewed API.
struct IStoveAccessToken : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveAccessToken);

    // Stove AccessToken value.
    virtual const wchar_t* GetAccessToken() const = 0;

    // Remaining valid time of the Stove AccessToken (seconds).
    virtual int32_t GetExpireIn() const = 0;
};


// ============================================================
//  IStoveUser
// ============================================================

// Structure of User information received when calling the GetUser API.
struct IStoveUser : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveUser);

    // Stove nickname of the user logged in from the launcher.
    virtual const wchar_t* GetNickName() const = 0;

    // GameUserId of the user logged in from the launcher.
    virtual uint64_t GetUserId() const = 0;
};


// ============================================================
//  IStoveGds
// ============================================================

// Structure of User information received when calling the GetGds API.
struct IStoveGds : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveGds);

    // True if the country code could not be determined from the user's IP and
    // the default country code is used instead; false if the country code was
    // resolved from the IP.
    virtual bool IsDefault() const = 0;

    // Country code resolved from the logged-in user's IP address (ISO 3166-1 ALPHA-2).
    virtual const wchar_t* GetNation() const = 0;

    // Regulation name applied by country code (e.g., GDPR).
    virtual const wchar_t* GetRegulation() const = 0;

    // Timezone of the logged-in user, as an IANA Time Zone Database (TZDB) ID
    // (e.g., L"Asia/Seoul").
    virtual const wchar_t* GetTimezone() const = 0;

    // UTC offset of the user's timezone, in minutes.
    virtual int32_t GetUtcOffset() const = 0;

    // Language of the logged-in user (ISO 639-1 ALPHA-2).
    // Exceptions: Chinese uses zh / zh-tw, Indonesian uses "in".
    virtual const wchar_t* GetLang() const = 0;
};


// ============================================================
//  IStoveSignin
// ============================================================

// Structure of Signin information received when calling the GetSignin API.
struct IStoveSignin : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveSignin);

    // Whether the user has completed personal identity verification.
    virtual bool IsPersonVerified() const = 0;

    // Whether the user's email address has been verified.
    virtual bool IsEmailVerified() const = 0;

    // Stove platform registration country code (ISO 3166-1 ALPHA-2).
    virtual const wchar_t* GetRegisteredCountryCode() const = 0;

    // IDP (Identity Provider) classifier code — the authentication method used
    // at Stove login (string form).
    // Examples: SO (Stove email), FB (Facebook), GP (Google), STEAM, VTCO .. etc
    virtual const wchar_t* GetProviderCode() const = 0;

    // Account type code (numeric form, server-driven).
    // Examples: 2=Facebook, 3=Twitter, 6=Naver, 9=Google+, 11=Stove PC signup,
    // 12=Apple, 13=LINE, 14=LINE Games, 15=Steam, 16=VTCO .. etc
    // Note: the published spec does not enumerate a fixed value->meaning
    // mapping for this code. To identify the IDP / account kind (e.g., STEAM
    // vs STEAM_SHADOW), refer to GetProviderCode() instead.
    virtual int32_t GetAccountType() const = 0;
};


// ============================================================
//  IStoveOverImmersionInfo
// ============================================================

// Structure for preventing over-immersion received when calling the
// OverImmersionNotification API.
struct IStoveOverImmersionInfo : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveOverImmersionInfo);

    // Over-immersion warning message.
    virtual const wchar_t* GetMsg() const = 0;

    // Game elapsed time, in hours.
    virtual int32_t GetElapsedHours() const = 0;

    // Message exposure time, in seconds.
    virtual int32_t GetExposureTime() const = 0;
};


// ============================================================
//  IStoveVietnamAgeRatingInfo
// ============================================================

// Structure for age rating notification received when calling the
// VietnamAgeRatingNotification API.
struct IStoveVietnamAgeRatingInfo : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveVietnamAgeRatingInfo);

    // Overlay display state (SHOW: show overlay, HIDE: hide overlay).
    virtual int32_t GetOverlayMode() const = 0;

    // Overlay type (0 = Black: dark background RGB(33,33,33) with white text,
    // 1 = White: white background RGB(255,255,255) with black text; White is the default).
    virtual int32_t GetOverlayType() const = 0;

    // Overlay scale (0.0 ~ 1.0).
    virtual float GetOverlayScale() const = 0;

    // Overlay opacity (0.0 ~ 1.0).
    virtual float GetOverlayOpacity() const = 0;

    // Game age rating (0 = All ages, 12 = 12+, 16 = 16+, 18 = 18+).
    virtual int32_t GetAgeRating() const = 0;

    // Age rating notification message.
    virtual const wchar_t* GetMsg() const = 0;

    // Message display position, x-coordinate.
    // Based on the left of the screen (0.0 ~ 1.0).
    virtual float GetDisplayPositionX() const = 0;

    // Message display position, y-coordinate.
    // Based on the top of the screen (0.0 ~ 1.0).
    virtual float GetDisplayPositionY() const = 0;

    // Language code for font selection
    // (e.g., L"ko", L"en", L"ja", L"vi", L"zh-cn", L"zh-tw", L"th").
    virtual const wchar_t* GetLanguage() const = 0;
};


// ============================================================
//  IStoveVietnamOverimmersionInfo
// ============================================================

// Structure for over-immersion notification received when calling the
// VietnamOverimmersionNotification API.
struct IStoveVietnamOverimmersionInfo : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveVietnamOverimmersionInfo);

    // Overlay display state (SHOW: show overlay, HIDE: hide overlay).
    virtual int32_t GetOverlayMode() const = 0;

    // Overlay type (0 = Black: dark background RGB(33,33,33) with white text,
    // 1 = White: white background RGB(255,255,255) with black text; White is the default).
    virtual int32_t GetOverlayType() const = 0;

    // Overlay scale (0.0 ~ 1.0).
    virtual float GetOverlayScale() const = 0;

    // Overlay opacity (0.0 ~ 1.0).
    virtual float GetOverlayOpacity() const = 0;

    // Game age rating (0 = All ages, 12 = 12+, 16 = 16+, 18 = 18+).
    virtual int32_t GetAgeRating() const = 0;

    // Over-immersion warning message.
    virtual const wchar_t* GetMsg() const = 0;

    // Styled (translated) warning message with markup tags such as
    // <b>, <color=#RRGGBBAA>, for rich-text rendering.
    virtual const wchar_t* GetStyledMsg() const = 0;

    // Game elapsed time, in minutes.
    virtual int32_t GetElapsedMinutes() const = 0;

    // Message exposure time, in seconds.
    virtual int32_t GetExposureTime() const = 0;

    // Expand animation time, in seconds.
    // Overlay expansion animation duration for the Show -> Expand transition.
    virtual float GetExpandAnimationTime() const = 0;

    // Message display position, x-coordinate.
    // Based on the left of the screen (0.0 ~ 1.0).
    virtual float GetDisplayPositionX() const = 0;

    // Message display position, y-coordinate.
    // Based on the top of the screen (0.0 ~ 1.0).
    virtual float GetDisplayPositionY() const = 0;

    // Language code for font selection
    // (e.g., L"ko", L"en", L"ja", L"vi", L"zh-cn", L"zh-tw", L"th").
    virtual const wchar_t* GetLanguage() const = 0;
};


// ============================================================
//  IStoveShutdownInfo
// ============================================================

// Structure for shutdown received when calling the ShutdownNotification API.
struct IStoveShutdownInfo : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveShutdownInfo);

    // Shutdown notification message.
    virtual const wchar_t* GetMsg() const = 0;

    // Shutdown message exposure time, in seconds.
    virtual int32_t GetExposureTime() const = 0;

    // Time remaining until the user is shut down, in minutes.
    virtual int32_t GetInadvanceMinutes() const = 0;
};


// ============================================================
//  IStoveRestartAppIfNecessaryOutcome
// ============================================================

// Outcome of the Stove_RestartAppIfNecessary() async call.
// Wraps the single 'requiresRestart' flag as a domain entity so that
// future fields can be added without changing the callback signature.
struct IStoveRestartAppIfNecessaryOutcome : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_StoveRestartAppIfNecessaryOutcome);

    // True if the app must be relaunched through the Stove launcher
    // (the current process should exit); false if it may continue.
    virtual bool IsRestartRequired() const = 0;
};


// ============================================================
//  IStoveSetGameProfileParam
// ============================================================

// GameProfile structure passed when calling the SetGameProfile API.
struct IStoveSetGameProfileParam : IStoveTypeBase
{
    STOVE_BASE_TYPE_KIND(k_EStoveBaseTypeKind_SetGameProfileParam);

    // World identifier of the game.
    virtual const wchar_t* GetWorldId() const = 0;
    virtual void SetWorldId(const wchar_t* worldId) = 0;

    // Character number within the world.
    // (Legacy StoveGameProfileParams labeled this field as "worldId Length",
    //  which was a documentation bug — the field is the character number.)
    virtual int64_t GetCharacterNo() const = 0;
    virtual void SetCharacterNo(int64_t characterNo) = 0;
};

#else

typedef struct IStoveTypeBase IStoveTypeBase;
typedef struct IStoveResult IStoveResult;
typedef struct IStoveCallbackResult IStoveCallbackResult;
typedef struct IStovePopupCreationInfo IStovePopupCreationInfo;
typedef struct IStoveRestartAppIfNecessaryParam IStoveRestartAppIfNecessaryParam;
typedef struct IStoveInitializeParam IStoveInitializeParam;
typedef struct IStoveAccessToken IStoveAccessToken;
typedef struct IStoveUser IStoveUser;
typedef struct IStoveGds IStoveGds;
typedef struct IStoveSignin IStoveSignin;
typedef struct IStoveOverImmersionInfo IStoveOverImmersionInfo;
typedef struct IStoveVietnamAgeRatingInfo IStoveVietnamAgeRatingInfo;
typedef struct IStoveVietnamOverimmersionInfo IStoveVietnamOverimmersionInfo;
typedef struct IStoveShutdownInfo IStoveShutdownInfo;
typedef struct IStoveRestartAppIfNecessaryOutcome IStoveRestartAppIfNecessaryOutcome;
typedef struct IStoveSetGameProfileParam IStoveSetGameProfileParam;

#endif // __cplusplus


// ============================================================
//  BaseSDK Callbacks
// ============================================================

// All callbacks follow the __cdecl calling convention.
// The IStoveCallbackResult* and any accompanying object pointers
// (token, user, gds, ...) are owned by the SDK and are valid only for
// the duration of the callback invocation; do NOT call Destroy() on them.

// Delivered by Stove_RestartAppIfNecessary().
typedef void(__cdecl* OnRestartAppIfNecessaryCallback)(const IStoveCallbackResult* callbackResult, const IStoveRestartAppIfNecessaryOutcome* outcome);

// Delivered by Stove_AccessTokenRenewed() when the access token is refreshed.
typedef void(__cdecl* OnAccessTokenRenewedCallback)(const IStoveCallbackResult* callbackResult, const IStoveAccessToken* token);

// Delivered by Stove_OverImmersionNotification() (Korea).
typedef void(__cdecl* OnOverImmersionNotificationCallback)(const IStoveCallbackResult* callbackResult, const IStoveOverImmersionInfo* overImmersion);

// Delivered by Stove_ShutdownNotification().
typedef void(__cdecl* OnShutdownNotificationCallback)(const IStoveCallbackResult* callbackResult, const IStoveShutdownInfo* shutdown);

// Delivered by Stove_VietnamAgeRatingNotification() (Vietnam).
typedef void(__cdecl* OnVietnamAgeRatingNotificationCallback)(const IStoveCallbackResult* callbackResult, const IStoveVietnamAgeRatingInfo* ageRatingInfo);

// Delivered by Stove_VietnamOverimmersionNotification() (Vietnam).
typedef void(__cdecl* OnVietnamOverimmersionNotificationCallback)(const IStoveCallbackResult* callbackResult, const IStoveVietnamOverimmersionInfo* overimmersionInfo);

// Delivered by Stove_OpenExternalUrl().
// 'reserved' is a placeholder kept to fix the callback's argument count ahead of
// any future payload: it is ALWAYS nullptr in the current SDK. When a concrete
// payload object is introduced here later, it will follow the same lifetime/ownership
// policy as the other callback payloads (SDK-owned, valid only for the callback
// duration, do NOT call Destroy()); integrators can use `if (reserved)` to detect it.
typedef void(__cdecl* OnOpenExternalUrlCallback)(const IStoveCallbackResult* callbackResult, const IStoveTypeBase* reserved);