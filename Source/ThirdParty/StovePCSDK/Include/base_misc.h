/*
 * file: base_misc.h
 * Enum definitions used in BaseSDK.
 */
#pragma once
#include <stdint.h> 
#include <stddef.h>
#include <wchar.h>

 // ============================================================
 //  Common defines
 // ============================================================

#if defined(_MSC_VER) && _MSC_VER < 1900
#define STOVE_CONSTEXPR const
#else
#define STOVE_CONSTEXPR constexpr
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
#define STOVE_NOEXCEPT
#else
#define STOVE_NOEXCEPT noexcept
#endif

// Calling convention used by every Stove_* / Stove_IStove*_* C function
// declared in the Public/C headers. Pinning this to __cdecl prevents
// stack-corruption when a 32-bit client builds with /Gz (stdcall default).
// (On x64 there is only one calling convention; the macro is a no-op then.)
#if defined(_MSC_VER)
#define STOVE_API __cdecl
#else
#define STOVE_API
#endif


// ============================================================
//  BaseSDK enums
// ============================================================

// Identifies the concrete type of an IStoveTypeBase instance.
// Used by Stove_CreateParam() to request a specific parameter object,
// and by IStoveTypeBase::GetTypeKind() for runtime type identification.
// Range (0 ~ 999)
typedef enum EStoveBaseTypeKind
{
    k_EStoveBaseTypeKind_Invalid = -1,

    // Result / Data types (returned or delivered by the SDK)
    k_EStoveBaseTypeKind_Base = 0,
    k_EStoveBaseTypeKind_StoveResult = 1,
    k_EStoveBaseTypeKind_StoveCallbackResult = 2,
    k_EStoveBaseTypeKind_StoveUser = 3,
    k_EStoveBaseTypeKind_StoveAccessToken = 4,
    k_EStoveBaseTypeKind_StoveGds = 5,
    k_EStoveBaseTypeKind_StoveSignin = 6,
    k_EStoveBaseTypeKind_StoveOverImmersionInfo = 7,
    k_EStoveBaseTypeKind_StoveVietnamAgeRatingInfo = 8,
    k_EStoveBaseTypeKind_StoveVietnamOverimmersionInfo = 9,
    k_EStoveBaseTypeKind_StoveShutdownInfo = 10,
    k_EStoveBaseTypeKind_StoveRestartAppIfNecessaryOutcome = 11,
    k_EStoveBaseTypeKind_PopupCreationInfo = 12,

    // Parameter types (created by the caller via Stove_CreateParam)
    k_EStoveBaseTypeKind_RestartAppIfNecessaryParam = 500,
    k_EStoveBaseTypeKind_InitializeParam = 501,
    k_EStoveBaseTypeKind_SetGameProfileParam = 502,

    k_EStoveBaseTypeKind_Max = 0x7fffffff,
} EStoveBaseTypeKind;


// Identifies which BaseSDK method produced a given IStoveResult.
// Retrieved via IStoveResult::GetMethodCode(); useful for logging and error routing.
typedef enum EStoveBaseMethodCode
{
    k_EStoveBaseMethodCode_Invalid = -1,

    // Lifecycle / Versioning
    k_EStoveBaseMethodCode_BaseInitialize = 1,
    k_EStoveBaseMethodCode_BaseUninitialize = 2,
    k_EStoveBaseMethodCode_InternalSend81Plug = 3,
    k_EStoveBaseMethodCode_InternalUpdate81Plug = 4,
    k_EStoveBaseMethodCode_BaseGetVersion = 5,

    // Public APIs
    k_EStoveBaseMethodCode_GetAccessToken = 64,
    k_EStoveBaseMethodCode_AccessTokenRenewed = 65,
    k_EStoveBaseMethodCode_GetUser = 66,
    k_EStoveBaseMethodCode_SetLanguage = 67,
    k_EStoveBaseMethodCode_OverImmersionNotification = 68,
    k_EStoveBaseMethodCode_ShutdownNotification = 69,
    // 70 : Deprecated (Internal method)
    // 71 : Deprecated (Internal method)
    k_EStoveBaseMethodCode_SetGameProfile = 72,
    k_EStoveBaseMethodCode_GetGds = 73,
    k_EStoveBaseMethodCode_GetSignin = 74,
    k_EStoveBaseMethodCode_RestartAppIfNecessary = 75,
    k_EStoveBaseMethodCode_RestartAppIfNecessaryAsync = 76,
    k_EStoveBaseMethodCode_OpenExternalUrl = 77,
    k_EStoveBaseMethodCode_GetCloudSavingPath = 78,
    k_EStoveBaseMethodCode_VietnamAgeRatingNotification = 79,
    k_EStoveBaseMethodCode_VietnamOverimmersionNotification = 80,

    // Closes all SDK popups (IAP + View).
    k_EStoveBaseMethodCode_CloseAllPopups = 81,

    // Internal methods (playtime reports, server config, token handling, etc.)
    // These codes are for SDK-internal / private surface use only and never
    // appear as the method code delivered to a game's callback via
    // IStoveResult::GetMethodCode().
    k_EStoveBaseMethodCode_InternalSetPlaytimeReport = 96,
    k_EStoveBaseMethodCode_InternalOperatorReportPlaytime = 97,
    k_EStoveBaseMethodCode_InternalGameExitReportPlaytime = 98,
    k_EStoveBaseMethodCode_InternalFileSaveUnsetPlaytime = 99,
    k_EStoveBaseMethodCode_InternalFailedPlaytimeTransfers = 100,
    k_EStoveBaseMethodCode_InternalOperatorReportConcurrentUser = 101,
    k_EStoveBaseMethodCode_InternalServerConfig = 102,
    k_EStoveBaseMethodCode_InternalGdsInfo = 103,
    k_EStoveBaseMethodCode_InternalGameCheckerLogin = 104,
    k_EStoveBaseMethodCode_InternalOnstoveLogin = 105,
    k_EStoveBaseMethodCode_InternalFunction = 106,
    k_EStoveBaseMethodCode_InternalRenewGuidToken = 107,
    k_EStoveBaseMethodCode_InternalRenewToken = 108,
    // 109 : Deprecated
    k_EStoveBaseMethodCode_InternalTranslateLanguage = 110,
    k_EStoveBaseMethodCode_InternalGetGamemeta = 111,
    k_EStoveBaseMethodCode_InternalForceRefreshToken = 112,
    k_EStoveBaseMethodCode_InternalAccessTokenRenewedToPrivate = 113,
    k_EStoveBaseMethodCode_InternalGetTokenToPrivate = 114,

    // Private getters (exposed to other SDK modules, not to games)
    // Same restriction as the Internal* block above: these are SDK-internal /
    // Private surface only and are never delivered to a game's callback via
    // IStoveResult::GetMethodCode().
    k_EStoveBaseMethodCode_GetEnvPrivate = 115,
    k_EStoveBaseMethodCode_GetGameIdPrivate = 116,
    k_EStoveBaseMethodCode_GetMemberNoPrivate = 117,
    k_EStoveBaseMethodCode_GetPublicIp = 118U,
    k_EStoveBaseMethodCode_GetTranslateLanguage = 119U,

    k_EStoveBaseMethodCode_Max = 0x7fffffff
} EStoveBaseMethodCode;


// Module-specific result codes — global dictionary, banded by category (hundreds digit = category).
//   3xx: Base    4xx: webview / popup UI    5xx: IAP purchase / payload
// Single binary makes per-module separation meaningless, so codes that meant the
// same thing across modules are unified into one number here. Which module a result
// came from is still recoverable from GetMethodCode() (module block = code / 1000).
// Common, cross-module result codes (0~299) live in EStoveCommonResultCode.
typedef enum EStoveResultCode
{
    // 3xx — Base language / GDS / launcher / token / IPC
    k_EStoveResultCode_LanguageNotSet = 300,               // Display language has not been set
    k_EStoveResultCode_EmptyTranslatedString = 301,        // Translated string is empty
    k_EStoveResultCode_NotFoundRequiredInformation = 302,  // Required information was not found
    k_EStoveResultCode_InvalidGdsInfo = 303,               // Invalid GDS (country/regulation) information
    k_EStoveResultCode_NeedStoveLauncher = 304,            // Stove launcher is required but not running
    k_EStoveResultCode_LauncherFailedCreateRequired = 305, // Failed to create a required launcher resource
    k_EStoveResultCode_RenewTokenMaxRetryCountExceeded = 306, // Token renewal exceeded the max retry count
    k_EStoveResultCode_IpcConnectFailed = 307,             // Failed to connect to the launcher via IPC
    k_EStoveResultCode_IpcAesKeyNotReceived = 308,         // Did not receive the AES key over IPC
    k_EStoveResultCode_IpcTimeout = 309,                   // Timed out communicating with the launcher (IPC)

    // 4xx — (reserved) Webview/popup UI codes moved to EStoveCommonResultCode (60~68)

    // 5xx — IAP purchase / payload (501, 502 moved to EStoveCommonResultCode 80~81; 503 stays here)
    k_EStoveResultCode_InvalidOrderProductInformation = 503, // Invalid order/product information

    k_EStoveResultCode_Max = 0x7fffffff
} EStoveResultCode;


// Common result codes shared across all Stove SDK modules.
// Retrieved via IStoveResult::GetResultCode(); Success == 0.
typedef enum EStoveCommonResultCode
{
    // General outcome
    k_EStoveCommonResultCode_Success = 0,
    k_EStoveCommonResultCode_Fail = 1,

    // Configuration / parameter validation failures
    k_EStoveCommonResultCode_InvalidConfig = 2,
    k_EStoveCommonResultCode_InvalidLogLevel = 3,
    k_EStoveCommonResultCode_InvalidLogPath = 4,
    k_EStoveCommonResultCode_InvalidParam = 5,

    // Initialization state errors
    k_EStoveCommonResultCode_BaseNotInitialized = 16,
    k_EStoveCommonResultCode_NotInitialized = 17,
    k_EStoveCommonResultCode_AlreadyInitialized = 18,

    // Token / entity errors
    k_EStoveCommonResultCode_InvalidAccessToken = 19,
    k_EStoveCommonResultCode_NullTokenEntity = 20,
    k_EStoveCommonResultCode_NullEntity = 21,

    // HTTP / response errors
    k_EStoveCommonResultCode_HttpError = 22,
    k_EStoveCommonResultCode_ResponseError = 23,
    k_EStoveCommonResultCode_ResponseInvalidCode = 24,
    k_EStoveCommonResultCode_ResponseValueIsNull = 25,
    k_EStoveCommonResultCode_ResponseInvalidValueFormat = 26,

    // 27 : Deprecated
    // 28 : Deprecated
    k_EStoveCommonResultCode_AsyncOperationInProgress = 29,
    k_EStoveCommonResultCode_BaseUninitialized = 30,
    k_EStoveCommonResultCode_NotSupportedCountry = 31,

    // 33 : Operation ended without creating a popup. Used only by the deprecated internal
    //      Stove_Internal_IAP_StartPayment path, where it is still delivered to onDestroy
    //      (safe to ignore there). Every other popup-showing API instead reports this via
    //      the onFinished payload's IsPopupCreated() and never delivers 33 to onDestroy.
    k_EStoveCommonResultCode_PopupNotCreated = 33,
    k_EStoveCommonResultCode_WebviewClosedBeforeComplete = 34, // Webview was closed before the operation completed

    // Local DB failures shared across all Stove SDK modules; all LocalDb codes now live here.
    k_EStoveCommonResultCode_LocalDbCreateWorkingDirectoryFailed = 40,
    k_EStoveCommonResultCode_LocalDbConnectFailed = 41,
    k_EStoveCommonResultCode_LocalDbCreateTableFailed = 42,
    k_EStoveCommonResultCode_LocalDbDisconnectFailed = 43,
    k_EStoveCommonResultCode_LocalDbWriteFailed = 44,

    // 45-59 reserved for future Local DB / payload / storage-layer codes (keeps DB band contiguous)

    // Webview / popup UI failures (cross-module); see also 33 PopupNotCreated / 34 WebviewClosedBeforeComplete
    k_EStoveCommonResultCode_ViewUiNotInitialized = 60,        // Popup/webview UI subsystem not initialized
    k_EStoveCommonResultCode_ViewUiUninitFailed = 61,          // Failed to shut down the popup/webview UI subsystem
    k_EStoveCommonResultCode_WebviewCreateFail = 62,           // Failed to create the webview
    k_EStoveCommonResultCode_WebviewLoadUrlFail = 63,          // Failed to load the URL in the webview
    k_EStoveCommonResultCode_WebviewCreateCookieFail = 64,     // Failed to set the webview cookie
    k_EStoveCommonResultCode_WebviewCloseAllFail = 65,         // Failed to close all webviews/popups
    k_EStoveCommonResultCode_WebviewCloseFail = 66,            // Failed to close the webview/popup
    k_EStoveCommonResultCode_NoPopupData = 67,                 // No popup data to display
    k_EStoveCommonResultCode_CloseAllPopupsFailed = 68,        // Stove_CloseAllPopups() failed to close one or more popups (IAP + View unified call)

    // 69-79 reserved for future webview/popup UI codes

    // Parameter / payload validation (cross-module)
    k_EStoveCommonResultCode_ParameterLengthExceeded = 80,     // Request parameter exceeds the maximum length
    k_EStoveCommonResultCode_InvalidJsonString = 81,           // Malformed JSON string
    k_EStoveCommonResultCode_PayloadSizeExceeded = 82,         // Payload exceeds the maximum allowed size

    // Network transport failures
    // 249 : Network transport error. externalError carries the HTTP backend's native error code (e.g. WinHTTP 12002/12007/12029).
    k_EStoveCommonResultCode_NetworkTransportError = 249,

    // System / runtime failures
    k_EStoveCommonResultCode_PcsdkDllNotFound = 251,
    k_EStoveCommonResultCode_NotImplemented = 252,
    k_EStoveCommonResultCode_UnmanagedException = 253,
    k_EStoveCommonResultCode_ManagedException = 254,
    k_EStoveCommonResultCode_UnknownError = 255,

    k_EStoveCommonResultCode_Max = 0x7fffffff
} EStoveCommonResultCode;


// How the View/IAP popup window is placed.
typedef enum EStoveWebViewMode
{
    k_EStoveWebViewMode_Invalid = -1,

    k_EStoveWebViewMode_External = 0,    // Standalone window, independent of the game window
    k_EStoveWebViewMode_Internal = 1,    // Child window of the game's main window (External if unavailable)

    k_EStoveWebViewMode_Max = 0x7fffffff
} EStoveWebViewMode;


// Display mode of the regulatory/notification overlay
// (e.g., Vietnam age-rating and over-immersion notices).
typedef enum EStoveOverlayMode
{
    k_EStoveOverlayMode_Invalid = -1,

    k_EStoveOverlayMode_Show = 0,        // Show the overlay
    k_EStoveOverlayMode_Hide = 1,        // Hide the overlay
    k_EStoveOverlayMode_Expanded = 2,    // Show the overlay in expanded form

    k_EStoveOverlayMode_Max = 0x7fffffff
} EStoveOverlayMode;