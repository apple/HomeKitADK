// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_TIMER_H
#define HAP_PLATFORM_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

typedef uintptr_t HAPPlatformTimerRef;

/**
 * Callback that is invoked when a timer expires.
 *
 * @param      timer                Timer ID.
 * @param      context              Client context.
 */
typedef void (*HAPPlatformTimerCallback)(HAPPlatformTimerRef timer, void* _Nullable context);

/**
 * Registers a timer to fire a callback after a certain time.
 *
 * - The callback is never invoked synchronously, even if the timer already expired on creation.
 *
 * - The deadline is given as an absolute time in milliseconds, relative to an implementation-defined time in the past
 *   (the same one as in HAPPlatformClockGetCurrent).
 *
 * - Timers fire in ascending order of their deadlines. Timers registered with the same deadline fire in order of
 *   registration.
 *
 * @param[out] timer                Non-zero Timer ID, if successful.
 * @param      deadline             Deadline after which the timer expires.
 * @param      callback             Function to call when the timer expires.
 * @param      context              Context that is passed to the callback.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If not more timers can be allocated.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformTimerRegister(
        HAPPlatformTimerRef* timer,
        HAPTime deadline,
        HAPPlatformTimerCallback callback,
        void* _Nullable context);

/**
 * Deregisters a timer that has not yet fired.
 *
 * @param      timer                Timer ID.
 */
void HAPPlatformTimerDeregister(HAPPlatformTimerRef timer);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
