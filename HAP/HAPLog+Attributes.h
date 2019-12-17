// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_LOG_ATTRIBUTES_H
#define HAP_LOG_ATTRIBUTES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

// ISO C99 requires at least one argument for the "..." in a variadic macro.
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC system_header
#endif

/**
 * Logs the contents of a buffer and a message related to an accessory at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogAccessoryBufferWithType(logObject, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogAccessoryBufferDebug(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogAccessoryBufferInfo(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogAccessoryBuffer(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogAccessoryBufferError(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogAccessoryBufferFault(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryBuffer(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryBufferInfo(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryBufferDebug(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryBufferError(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryBufferFault(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a message related to an accessory at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogAccessoryWithType(logObject, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogAccessoryDebug(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogAccessoryInfo(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogAccessory(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogAccessoryError(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogAccessoryFault(logObject, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessory(logObject, accessory, format, ...) \
    HAPLog(logObject, "[%016llX %s] " format, (unsigned long long) (accessory)->aid, (accessory)->name, ##__VA_ARGS__)

/**
 * Logs an info-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryInfo(logObject, accessory, format, ...) \
    HAPLogInfo( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryDebug(logObject, accessory, format, ...) \
    HAPLogDebug( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryError(logObject, accessory, format, ...) \
    HAPLogError( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to an accessory.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogAccessoryFault(logObject, accessory, format, ...) \
    HAPLogFault( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a message related to an accessory
 * at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveAccessoryBufferWithType(logObject, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveAccessoryBufferDebug(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveAccessoryBufferInfo(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveAccessoryBuffer(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveAccessoryBufferError(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveAccessoryBufferFault(logObject, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to an accessory
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryBuffer(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to an accessory
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryBufferInfo(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to an accessory
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryBufferDebug(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to an accessory
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryBufferError(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to an accessory
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryBufferFault(logObject, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a message related to an accessory at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveAccessoryWithType(logObject, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveAccessoryDebug(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveAccessoryInfo(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveAccessory(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveAccessoryError(logObject, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveAccessoryFault(logObject, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to an accessory that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessory(logObject, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs an info-level message related to an accessory that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryInfo(logObject, accessory, format, ...) \
    HAPLogSensitiveInfo( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to an accessory that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryDebug(logObject, accessory, format, ...) \
    HAPLogSensitiveDebug( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to an accessory that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryError(logObject, accessory, format, ...) \
    HAPLogSensitiveError( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to an accessory that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      accessory            Accessory.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveAccessoryFault(logObject, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Logs the contents of a buffer and a message related to a service at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogServiceBufferWithType(logObject, service, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogServiceBufferDebug(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogServiceBufferInfo(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogServiceBuffer(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogServiceBufferError(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogServiceBufferFault(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceBuffer(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceBufferInfo(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceBufferDebug(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceBufferError(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceBufferFault(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a message related to a service at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogServiceWithType(logObject, service, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogServiceDebug(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogServiceInfo(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogService(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogServiceError(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogServiceFault(logObject, service, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogService(logObject, service, accessory, format, ...) \
    HAPLog(logObject, \
           "[%016llX %s] [%016llX %s] " format, \
           (unsigned long long) (accessory)->aid, \
           (accessory)->name, \
           (unsigned long long) (service)->iid, \
           (service)->debugDescription, \
           ##__VA_ARGS__)

/**
 * Logs an info-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceInfo(logObject, service, accessory, format, ...) \
    HAPLogInfo( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceDebug(logObject, service, accessory, format, ...) \
    HAPLogDebug( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceError(logObject, service, accessory, format, ...) \
    HAPLogError( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to a service.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogServiceFault(logObject, service, accessory, format, ...) \
    HAPLogFault( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a message related to a service
 * at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveServiceBufferWithType(logObject, service, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveServiceBufferDebug(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveServiceBufferInfo(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveServiceBuffer(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveServiceBufferError(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveServiceBufferFault(logObject, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to a service
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceBuffer(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to a service
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceBufferInfo(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to a service
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceBufferDebug(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to a service
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceBufferError(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to a service
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceBufferFault(logObject, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a message related to a service at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveServiceWithType(logObject, service, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveServiceDebug(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveServiceInfo(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveService(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveServiceError(logObject, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveServiceFault(logObject, service, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to a service that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveService(logObject, service, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an info-level message related to a service that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceInfo(logObject, service, accessory, format, ...) \
    HAPLogSensitiveInfo( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to a service that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceDebug(logObject, service, accessory, format, ...) \
    HAPLogSensitiveDebug( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to a service that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceError(logObject, service, accessory, format, ...) \
    HAPLogSensitiveError( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to a service that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      service              Service.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveServiceFault(logObject, service, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) (service)->iid, \
            (service)->debugDescription, \
            ##__VA_ARGS__)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Logs the contents of a buffer and a message related to a characteristic at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogCharacteristicBufferWithType(logObject, characteristic, service, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogCharacteristicBufferDebug( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogCharacteristicBufferInfo( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogCharacteristicBuffer( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogCharacteristicBufferError( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogCharacteristicBufferFault( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicBuffer(logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicBufferInfo(logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicBufferDebug(logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicBufferError(logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicBufferFault(logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a message related to a characteristic at a specific logging level.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogCharacteristicWithType(logObject, characteristic, service, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogCharacteristicDebug(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogCharacteristicInfo(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogCharacteristic(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogCharacteristicError(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogCharacteristicFault(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristic(logObject, characteristic, service, accessory, format, ...) \
    HAPLog(logObject, \
           "[%016llX %s] [%016llX %s] " format, \
           (unsigned long long) (accessory)->aid, \
           (accessory)->name, \
           (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
           ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
           ##__VA_ARGS__)

/**
 * Logs an info-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicInfo(logObject, characteristic, service, accessory, format, ...) \
    HAPLogInfo( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicDebug(logObject, characteristic, service, accessory, format, ...) \
    HAPLogDebug( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicError(logObject, characteristic, service, accessory, format, ...) \
    HAPLogError( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to a characteristic.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogCharacteristicFault(logObject, characteristic, service, accessory, format, ...) \
    HAPLogFault( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a message related to a characteristic
 * at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveCharacteristicBufferWithType( \
        logObject, characteristic, service, accessory, bytes, numBytes, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveCharacteristicBufferDebug( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveCharacteristicBufferInfo( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveCharacteristicBuffer( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveCharacteristicBufferError( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveCharacteristicBufferFault( \
                        logObject, characteristic, service, accessory, bytes, numBytes, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs the contents of a buffer and a default-level message related to a characteristic
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicBuffer( \
        logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBuffer( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an info-level message related to a characteristic
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicBufferInfo( \
        logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferInfo( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a debug-level message related to a characteristic
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicBufferDebug( \
        logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferDebug( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and an error-level message related to a characteristic
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicBufferError( \
        logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferError( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs the contents of a buffer and a fault-level message related to a characteristic
 * that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      bytes                Buffer containing data to log.
 * @param      numBytes             Length of buffer.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicBufferFault( \
        logObject, characteristic, service, accessory, bytes, numBytes, format, ...) \
    HAPLogSensitiveBufferFault( \
            logObject, \
            bytes, \
            numBytes, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a message related to a characteristic at a specific logging level that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      type                 A log type constant, indicating the level of logging to perform.
 */
#define HAPLogSensitiveCharacteristicWithType(logObject, characteristic, service, accessory, type, ...) \
    do { \
        switch (type) { \
            case kHAPLogType_Debug: { \
                HAPLogSensitiveCharacteristicDebug(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Info: { \
                HAPLogSensitiveCharacteristicInfo(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Default: { \
                HAPLogSensitiveCharacteristic(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Error: { \
                HAPLogSensitiveCharacteristicError(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
            case kHAPLogType_Fault: { \
                HAPLogSensitiveCharacteristicFault(logObject, characteristic, service, accessory, __VA_ARGS__); \
            } break; \
        } \
    } while (0)

/**
 * Logs a default-level message related to a characteristic that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristic(logObject, characteristic, service, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an info-level message related to a characteristic that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicInfo(logObject, characteristic, service, accessory, format, ...) \
    HAPLogSensitiveInfo( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a debug-level message related to a characteristic that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicDebug(logObject, characteristic, service, accessory, format, ...) \
    HAPLogSensitiveDebug( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs an error-level message related to a characteristic that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicError(logObject, characteristic, service, accessory, format, ...) \
    HAPLogSensitiveError( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

/**
 * Logs a fault-level message related to a characteristic that may contain sensitive information.
 *
 * @param      logObject            Log object.
 * @param      characteristic       Characteristic.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      format               Format string that produces a human-readable log message.
 */
#define HAPLogSensitiveCharacteristicFault(logObject, characteristic, service, accessory, format, ...) \
    HAPLogSensitive( \
            logObject, \
            "[%016llX %s] [%016llX %s] " format, \
            (unsigned long long) (accessory)->aid, \
            (accessory)->name, \
            (unsigned long long) ((const HAPBaseCharacteristic*) (characteristic))->iid, \
            ((const HAPBaseCharacteristic*) (characteristic))->debugDescription, \
            ##__VA_ARGS__)

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
