/*
 * file: log_api.h
 * Flat C API function declarations provided by LogSDK.
 */

#pragma once
#include "log_types.h"


#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================
    //  LogSDK API
    // ============================================================


    // Sends a single log entry to the Stove log backend.
    void STOVE_API Stove_SendLog(const IStoveSendLogParam* logSendParam, OnSendLogCallback onFinished, void* userData);


    // ============================================================
    //  Function Pointer Typedefs
    // ============================================================

    typedef void(__cdecl* Stove_SendLog_t)(const IStoveSendLogParam* logSendParam, OnSendLogCallback onFinished, void* userData);

#ifdef __cplusplus
}
#endif