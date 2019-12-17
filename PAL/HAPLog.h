// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_LOG_H
#define HAP_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPBase.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

// Validate log level.
// 0 - No logs are emitted. Default.
// 1 - Logs with type Default, Error and Fault are emitted.
// 2 - Logs with type Info, Default, Error and Fault are emitted.
// 3 - Logs with type Debug, Info, Default, Error and Fault are emitted. All logs.
#ifndef HAP_LOG_LEVEL
#define HAP_LOG_LEVEL (0)
#endif
#if HAP_LOG_LEVEL < 0 || HAP_LOG_LEVEL > 3
#error "Invalid HAP_LOG_LEVEL."
#endif

// Validate flag for including sensitive information in logs.
#ifndef HAP_LOG_SENSITIVE
#define HAP_LOG_SENSITIVE (0)
#endif
#if HAP_LOG_SENSITIVE < 0 || HAP_LOG_SENSITIVE > 1
#error "Invalid HAP_LOG_SENSITIVE."
#endif

/**
 * Log object.
 */
typedef struct {
    /**
     * Subsystem that's performing logging.
     */
    const char* _Nullable subsystem;

    /**
     * A category within the specified subsystem.
     *
     * - If a category is defined, a subsystem must be specified as well.
     */
    const char* _Nullable category;
} HAPLogObject;

/**
 * Default log object.
 *
 * - Log messages are logged with NULL subsystem and category.
 */
extern const HAPLogObject kHAPLog_Default;

/**
 * Logging levels.
 */
HAP_ENUM_BEGIN(uint8_t, HAPLogType) {
    /**
     * Messages logged at this level contain information that may be useful during development or while troubleshooting
     * a specific problem.
     */
    kHAPLogType_Debug,

    /**
     * Use this level to capture information that may be helpful, but isn't essential, for troubleshooting errors.
     */
    kHAPLogType_Info,

    /**
     * Use this level to capture information about things that might result a failure.
     */
    kHAPLogType_Default,

    /**
     * Error-level messages are intended for reporting component-level errors.
     */
    kHAPLogType_Error,

    /**
     * Fault-level messages are intended for capturing system-level or multi-component errors only.
     */
    kHAPLogType_Fault
} HAP_ENUM_END(uint8_t, HAPLogType);

/**
 * Logs the contents of a buffer and a message at a specific logging level.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogBufferWithType(log, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogBufferDebug(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogBufferInfo(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogBuffer(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogBufferError(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogBufferFault(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogBuffer(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogBufferInternal(log, bytes, numBytes, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and an info-level message.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogBufferInfo(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 2) { \
            HAPLogBufferInfoInternal(log, bytes, numBytes, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a debug-level message.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogBufferDebug(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 3) { \
            HAPLogBufferDebugInternal(log, bytes, numBytes, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and an error-level message.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogBufferError(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogBufferErrorInternal(log, bytes, numBytes, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a fault-level message.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogBufferFault(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogBufferFaultInternal(log, bytes, numBytes, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs a message at a specific logging level.
 *
 * @param      log                  Log object.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogWithType(log, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogDebug(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogInfo(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLog(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogError(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogFault(log, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message.
 *
 * @param      log                  Log object.
 */
#define HAPLog(log, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogInternal(log, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs an info-level message.
 *
 * @param      log                  Log object.
 */
#define HAPLogInfo(log, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 2) { \
            HAPLogInfoInternal(log, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs a debug-level message.
 *
 * @param      log                  Log object.
 */
#define HAPLogDebug(log, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 3) { \
            HAPLogDebugInternal(log, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs an error-level message.
 *
 * @param      log                  Log object.
 */
#define HAPLogError(log, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogErrorInternal(log, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs a fault-level message.
 *
 * @param      log                  Log object.
 */
#define HAPLogFault(log, ...) \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPLogFaultInternal(log, __VA_ARGS__); \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a message at a specific logging level that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveBufferWithType(log, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveBufferDebug(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveBufferInfo(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveBuffer(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveBufferError(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveBufferFault(log, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogSensitiveBuffer(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogBuffer(log, bytes, numBytes, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLog(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and an info-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogSensitiveBufferInfo(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogBufferInfo(log, bytes, numBytes, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogInfo(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a debug-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogSensitiveBufferDebug(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogBufferDebug(log, bytes, numBytes, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogDebug(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and an error-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogSensitiveBufferError(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogBufferError(log, bytes, numBytes, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogError(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a fault-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 */
#define HAPLogSensitiveBufferFault(log, bytes, numBytes, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogBufferFault(log, bytes, numBytes, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogFault(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs a message at a specific logging level that may contain sensitive information.
 *
 * @param      log                  Log object.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveWithType(log, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveDebug(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveInfo(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitive(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveError(log, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveFault(log, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 */
#define HAPLogSensitive(log, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLog(log, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLog(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs an info-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 */
#define HAPLogSensitiveInfo(log, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogInfo(log, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogInfo(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs a debug-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 */
#define HAPLogSensitiveDebug(log, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogDebug(log, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogDebug(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs an error-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 */
#define HAPLogSensitiveError(log, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogError(log, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogError(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

/**
 * Logs a fault-level message that may contain sensitive information.
 *
 * @param      log                  Log object.
 */
#define HAPLogSensitiveFault(log, ...) \
    do { \
        if (HAP_LOG_SENSITIVE) { \
            HAPLogFault(log, __VA_ARGS__); \
        } else { \
            HAP_DIAGNOSTIC_PUSH \
            HAP_DIAGNOSTIC_IGNORED_CLANG("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_GCC("-Wformat-extra-args") \
            HAP_DIAGNOSTIC_IGNORED_ARMCC(225) \
            HAPLogFault(log, "<private> %s", __VA_ARGS__); \
            HAP_DIAGNOSTIC_POP \
        } \
    } while (0)

//----------------------------------------------------------------------------------------------------------------------
// Internal functions. Do not use directly.

/**@cond */
HAP_PRINTFLIKE(4, 5)
void HAPLogBufferInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...);
HAP_DISALLOW_USE(HAPLogBufferInternal)

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferInfoInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...);
HAP_DISALLOW_USE(HAPLogBufferInfoInternal)

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferDebugInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...);
HAP_DISALLOW_USE(HAPLogBufferDebugInternal)

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferErrorInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...);
HAP_DISALLOW_USE(HAPLogBufferErrorInternal)

HAP_PRINTFLIKE(4, 5)
void HAPLogBufferFaultInternal(
        const HAPLogObject* _Nullable log,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes,
        const char* format,
        ...);
HAP_DISALLOW_USE(HAPLogBufferFaultInternal)

HAP_PRINTFLIKE(2, 3)
void HAPLogInternal(const HAPLogObject* _Nullable log, const char* format, ...);
HAP_DISALLOW_USE(HAPLogInternal)

HAP_PRINTFLIKE(2, 3)
void HAPLogInfoInternal(const HAPLogObject* _Nullable log, const char* format, ...);
HAP_DISALLOW_USE(HAPLogInfoInternal)

HAP_PRINTFLIKE(2, 3)
void HAPLogDebugInternal(const HAPLogObject* _Nullable log, const char* format, ...);
HAP_DISALLOW_USE(HAPLogDebugInternal)

HAP_PRINTFLIKE(2, 3)
void HAPLogErrorInternal(const HAPLogObject* _Nullable log, const char* format, ...);
HAP_DISALLOW_USE(HAPLogErrorInternal)

HAP_PRINTFLIKE(2, 3)
void HAPLogFaultInternal(const HAPLogObject* _Nullable log, const char* format, ...);
HAP_DISALLOW_USE(HAPLogFaultInternal)
/**@endcond */

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
