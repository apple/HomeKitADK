// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#undef HAP_DISALLOW_USE_IGNORED
#define HAP_DISALLOW_USE_IGNORED 1

#include "HAPPlatform.h"

const HAPLogObject kHAPLog_Default = { .subsystem = NULL, .category = NULL };

/**
 * Maximum length of a log message.
 */
#define kHAPLogMessage_MaxBytes ((size_t)(2 * 1024))

HAP_PRINTFLIKE(5, 0)
static void
        Capture(const HAPLogObject* _Nullable const log,
                const void* _Nullable bytes,
                size_t numBytes,
                HAPLogType type,
                const char* format,
                va_list args) {
    HAPError err;

    if (!log) {
        return;
    }

    // Check if logs are enabled.
    HAPPlatformLogEnabledTypes enabledTypes = HAPPlatformLogGetEnabledTypes(log);
    switch (enabledTypes) {
        case kHAPPlatformLogEnabledTypes_None: {
            return;
        }
        case kHAPPlatformLogEnabledTypes_Default: {
            if (type == kHAPLogType_Info || type == kHAPLogType_Debug) {
                return;
            }
        } break;
        case kHAPPlatformLogEnabledTypes_Info: {
            if (type == kHAPLogType_Debug) {
                return;
            }
        } break;
        case kHAPPlatformLogEnabledTypes_Debug: {
        } break;
    }

    // Format log message.
    char message[kHAPLogMessage_MaxBytes];
    HAPRawBufferZero(message, sizeof message);

    err = HAPStringWithFormatAndArguments(message, sizeof message, format, args);
    if (err) {
        HAPPlatformLogCapture(log, kHAPLogType_Error, "<Log message too long>", NULL, 0);
        return;
    }

    // Capture log.
    HAPPlatformLogCapture(log, type, message, bytes, numBytes);
}

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...) {
    va_list args;
    va_start(args, format);
    Capture(log, bufferBytes, numBufferBytes, kHAPLogType_Default, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferInfoInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...) {
    va_list args;
    va_start(args, format);
    Capture(log, bufferBytes, numBufferBytes, kHAPLogType_Info, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferDebugInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...) {
    va_list args;
    va_start(args, format);
    Capture(log, bufferBytes, numBufferBytes, kHAPLogType_Debug, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferErrorInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...) {
    va_list args;
    va_start(args, format);
    Capture(log, bufferBytes, numBufferBytes, kHAPLogType_Error, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferFaultInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...) {
    va_list args;
    va_start(args, format);
    Capture(log, bufferBytes, numBufferBytes, kHAPLogType_Fault, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 3)
void HAPLogInternal(const HAPLogObject* _Nullable log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    Capture(log, /* bufferBytes: */ NULL, /* numBufferBytes: */ 0, kHAPLogType_Default, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 3)
void HAPLogInfoInternal(const HAPLogObject* _Nullable log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    Capture(log, /* bufferBytes: */ NULL, /* numBufferBytes: */ 0, kHAPLogType_Info, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 3)
void HAPLogDebugInternal(const HAPLogObject* _Nullable log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    Capture(log, /* bufferBytes: */ NULL, /* numBufferBytes: */ 0, kHAPLogType_Debug, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 3)
void HAPLogErrorInternal(const HAPLogObject* _Nullable log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    Capture(log, /* bufferBytes: */ NULL, /* numBufferBytes: */ 0, kHAPLogType_Error, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 3)
void HAPLogFaultInternal(const HAPLogObject* _Nullable log, const char* format, ...) {
    va_list args;
    va_start(args, format);
    Capture(log, /* bufferBytes: */ NULL, /* numBufferBytes: */ 0, kHAPLogType_Fault, format, args);
    va_end(args);
}
