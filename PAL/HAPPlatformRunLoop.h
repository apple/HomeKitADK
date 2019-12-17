// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_RUN_LOOP_H
#define HAP_PLATFORM_RUN_LOOP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Runs the loop which processes data from all attached input sources.
 *
 * - This function may only be called while the run loop is stopped.
 */
void HAPPlatformRunLoopRun(void);

/**
 * Schedules a request to exit the run loop.
 *
 * - This function must be called from the same execution context (e.g., thread) as the run loop,
 *   i.e. it should be scheduled on the run loop.
 *
 * - When this function returns, the run loop is not yet fully stopped.
 *   Wait for HAPPlatformRunLoopRun to return.
 *
 * - The execution of pending scheduled events may be delayed until the run loop is started again.
 */
void HAPPlatformRunLoopStop(void);

/**
 * Callback that is invoked from the run loop.
 *
 * @param      context              Client context.
 * @param      contextSize          Size of the context.
 */
typedef void (*HAPPlatformRunLoopCallback)(void* _Nullable context, size_t contextSize);

/**
 * Schedule a callback that will be called from the run loop.
 *
 * - It is safe to call this function from execution contexts (e.g., threads) other than the run loop,
 *   e.g., from another thread or from a signal handler.
 *
 * @param      callback             Function to call on the run loop.
 * @param      context              Context that is passed to the callback.
 * @param      contextSize          Size of context data that is passed to the callback.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If there are not enough resources to schedule the callback.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformRunLoopScheduleCallback(
        HAPPlatformRunLoopCallback callback,
        void* _Nullable context,
        size_t contextSize);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
