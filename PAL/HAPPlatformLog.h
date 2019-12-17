// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_LOG_H
#define HAP_PLATFORM_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Enabled log types.
 */
HAP_ENUM_BEGIN(uint8_t, HAPPlatformLogEnabledTypes) {
    /**
     * No messages are captured.
     */
    kHAPPlatformLogEnabledTypes_None,

    /**
     * Only default-level messages are captured.
     *
     * - Default-level messages contain information about things that might result a failure.
     *
     * - Error- and fault-level messages are also included.
     *   Error-level messages are intended for reporting component-level errors.
     *   Fault-level messages are intended for capturing system-level or multi-component errors only.
     */
    kHAPPlatformLogEnabledTypes_Default,

    /**
     * Default-level and info-level messages are captured.
     *
     * - Info-level messages contain information that may be helpful, but isn't essential, for troubleshooting errors.
     */
    kHAPPlatformLogEnabledTypes_Info,

    /**
     * Default-level, info-level, and debug-level messages are captured.
     *
     * - Messages logged at debug level contain information that may be useful during development or while
     *   troubleshooting a specific problem.
     */
    kHAPPlatformLogEnabledTypes_Debug
} HAP_ENUM_END(uint8_t, HAPPlatformLogEnabledTypes);

/**
 * Indicates whether a specific type of logging, such as default, info, debug, error, or fault, is enabled
 * for a specific log object. Different log objects may have different configurations.
 *
 * - Log levels are described in the documentation of HAPLogType in file HAPLog.h.
 *
 * @param      log                  Log object.
 *
 * @return Logging levels that shall be enabled for the given subsystem / category.
 */
HAP_RESULT_USE_CHECK
HAPPlatformLogEnabledTypes HAPPlatformLogGetEnabledTypes(const HAPLogObject* log);

/**
 * Logs a message.
 *
 * @param      log                  Log object.
 * @param      type                 Logging level.
 * @param      message              A log message. NULL-terminated.
 * @param      bufferBytes          Optional buffer containing related data to log.
 * @param      numBufferBytes       Length of buffer.
 */
void HAPPlatformLogCapture(
        const HAPLogObject* log,
        HAPLogType type,
        const char* message,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes) HAP_DIAGNOSE_ERROR(!bufferBytes && numBufferBytes, "empty buffer cannot have a length");

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
