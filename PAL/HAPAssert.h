// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ASSERT_H
#define HAP_ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPBase.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

// Validate flag for disabling assertions.
#ifndef HAP_DISABLE_ASSERTS
#define HAP_DISABLE_ASSERTS (0)
#endif
#if HAP_DISABLE_ASSERTS < 0 || HAP_DISABLE_ASSERTS > 1
#error "Invalid HAP_DISABLE_ASSERTS."
#endif

// Validate flag for disabling preconditions.
#ifndef HAP_DISABLE_PRECONDITIONS
#define HAP_DISABLE_PRECONDITIONS (0)
#endif
#if HAP_DISABLE_PRECONDITIONS < 0 || HAP_DISABLE_PRECONDITIONS > 1
#error "Invalid HAP_DISABLE_PRECONDITIONS."
#endif

/**
 * Performs an assert if assertions are enabled.
 *
 * @param      condition            The condition to test. Only evaluated when assertions are enabled.
 */
#define HAPAssert(condition) \
    do { \
        if (!HAP_DISABLE_ASSERTS && !(condition)) { \
            if (HAP_LOG_LEVEL >= 1) { \
                HAPAssertInternal(__func__, HAP_FILE, __LINE__); \
            } else { \
                HAPAssertAbortInternal(); \
            } \
        } \
    } while (0)

/**
 * Indicates that an internal sanity check failed if assertions are enabled.
 */
#define HAPAssertionFailure() \
    do { \
        if (!HAP_DISABLE_ASSERTS) { \
            if (HAP_LOG_LEVEL >= 1) { \
                HAPAssertionFailureInternal(__func__, HAP_FILE, __LINE__); \
            } else { \
                HAPAssertAbortInternal(); \
            } \
        } \
    } while (0)

/**
 * Checks a necessary condition for making forward progress if precondition checks are enabled.
 *
 * @param      condition            The condition to test. Only evaluated when precondition checks are enabled.
 */
#define HAPPrecondition(condition) \
    do { \
        if (!HAP_DISABLE_PRECONDITIONS && !(condition)) { \
            if (HAP_LOG_LEVEL >= 1) { \
                HAPPreconditionInternal(#condition, __func__); \
            } else { \
                HAPAssertAbortInternal(); \
            } \
        } \
    } while (0)

/**
 * Indicates that a precondition was violated if precondition checks are enabled.
 */
#define HAPPreconditionFailure() \
    do { \
        if (!HAP_DISABLE_PRECONDITIONS) { \
            if (HAP_LOG_LEVEL >= 1) { \
                HAPPreconditionFailureInternal(__func__); \
            } else { \
                HAPAssertAbortInternal(); \
            } \
        } \
    } while (0)

/**
 * Unconditionally indicates a fatal error.
 */
#define HAPFatalError() \
    do { \
        if (HAP_LOG_LEVEL >= 1) { \
            HAPFatalErrorInternal(__func__, HAP_FILE, __LINE__); \
        } else { \
            HAPAssertAbortInternal(); \
        } \
    } while (0)

//----------------------------------------------------------------------------------------------------------------------
// Internal functions. Do not use directly.

/**@cond */
HAP_NORETURN
void HAPAssertAbortInternal(void);
HAP_DISALLOW_USE(HAPAssertAbortInternal)

HAP_NORETURN
void HAPAssertInternal(const char* callerFunction, const char* callerFile, int callerLine);
HAP_DISALLOW_USE(HAPAssertInternal)

HAP_NORETURN
void HAPAssertionFailureInternal(const char* callerFunction, const char* callerFile, int callerLine);
HAP_DISALLOW_USE(HAPAssertionFailureInternal)

HAP_NORETURN
void HAPPreconditionInternal(const char* condition, const char* callerFunction);
HAP_DISALLOW_USE(HAPPreconditionInternal)

HAP_NORETURN
void HAPPreconditionFailureInternal(const char* callerFunction);
HAP_DISALLOW_USE(HAPPreconditionFailureInternal)

HAP_NORETURN
void HAPFatalErrorInternal(const char* callerFunction, const char* callerFile, int callerLine);
HAP_DISALLOW_USE(HAPFatalErrorInternal)
/**@endcond */

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
