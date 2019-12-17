// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPPlatformTimer+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "Timer" };

#define kTimerStorage_MaxTimers ((size_t) 32)

typedef struct {
    /**
     * ID. 0 if timer has never been used.
     */
    HAPPlatformTimerRef id;

    /**
     * Deadline after which the timer expires.
     */
    HAPTime deadline;

    /**
     * Callback. NULL if timer inactive.
     */
    HAPPlatformTimerCallback _Nullable callback;

    /**
     * The context parameter given to the HAPPlatformTimerRegister function.
     */
    void* _Nullable context;
} HAPPlatformTimer;

static HAPPlatformTimer timers[kTimerStorage_MaxTimers];
static size_t numActiveTimers;
static size_t numExpiredTimers;

void HAPPlatformTimerProcessExpiredTimers(void) {
    // Reentrancy note - Callbacks may lead to reentrant add / remove timer invocations.
    // Do not call any functions that may lead to reentrancy!
    //
    // The idea is that timers 0 ..< numExpiredTimers are managed here.
    // add / remove must only move timers numExpiredTimers ..< numActiveTimers.
    // Timers added through reentrancy are allocated after the expired timers.
    // Timers removed through reentrancy have their callback set to NULL.

    // Get current time, and, by checking, make sure that it is updated.
    HAPTime now = HAPPlatformClockGetCurrent();

    // Find number of expired timers.
    for (numExpiredTimers = 0; numExpiredTimers < numActiveTimers; numExpiredTimers++) {
        if (timers[numExpiredTimers].deadline > now) {
            break;
        }
    }

    // Invoke callbacks.
    for (size_t i = 0; i < numExpiredTimers; i++) {
        if (timers[i].callback) {
            HAPLogDebug(&logObject, "Expired timer: %lu", (unsigned long) timers[i].id);
            timers[i].callback(timers[i].id, timers[i].context);
            timers[i].callback = NULL;
        }
    }

    // Free memory.
    HAPAssert(numExpiredTimers <= numActiveTimers);
    while (numExpiredTimers) {
        HAPPlatformTimerRef id = timers[0].id;
        HAPRawBufferCopyBytes(&timers[0], &timers[1], (numActiveTimers - 1) * sizeof timers[0]);
        numActiveTimers--;
        numExpiredTimers--;
        timers[numActiveTimers].id = id;
    }
    HAPAssert(!numExpiredTimers);
}

HAP_RESULT_USE_CHECK
HAPError HAPPlatformTimerRegister(
        HAPPlatformTimerRef* timer,
        HAPTime deadline,
        HAPPlatformTimerCallback callback,
        void* _Nullable context) {
    HAPPrecondition(timer);
    HAPPrecondition(callback);

    // Do not call any functions that may lead to reentrancy!

    if (numActiveTimers == sizeof timers / sizeof timers[0]) {
        HAPLog(&logObject, "Cannot allocate more timers.");
        return kHAPError_OutOfResources;
    }

    // Find timer slot.
    size_t i;
    for (i = numExpiredTimers; i < numActiveTimers; i++) {
        if (timers[i].deadline > deadline) {
            // Search condition must be '>' and not '>=' to ensure that timers fire in ascending order of their
            // deadlines and that timers registered with the same deadline fire in order of registration.
            break;
        }
    }

    // Move timers.
    HAPPlatformTimerRef id = timers[numActiveTimers].id;
    HAPRawBufferCopyBytes(&timers[i + 1], &timers[i], (numActiveTimers - i) * sizeof timers[0]);
    timers[i].id = id;
    numActiveTimers++;

    // Prepare timer.
    static HAPPlatformTimerRef peakNumTimers;
    if (!timers[i].id) {
        timers[i].id = ++peakNumTimers;
        HAPAssert(timers[i].id <= sizeof timers / sizeof timers[0]);
        HAPLogInfo(
                &logObject,
                "New maximum of concurrent timers: %u (%u%%).",
                (unsigned int) peakNumTimers,
                (unsigned int) (100 * peakNumTimers / (sizeof timers / sizeof timers[0])));
    }

    // Store client data.
    timers[i].deadline = deadline;
    timers[i].callback = callback;
    timers[i].context = context;

    // Store timer ID.
    *timer = timers[i].id;

    HAPLogDebug(
            &logObject,
            "Added timer: %lu (deadline %8llu.%03llu).",
            (unsigned long) timers[i].id,
            (unsigned long long) (timers[i].deadline / HAPSecond),
            (unsigned long long) (timers[i].deadline % HAPSecond));
    return kHAPError_None;
}

void HAPPlatformTimerDeregister(HAPPlatformTimerRef timer) {
    HAPPrecondition(timer);

    // Do not call any functions that may lead to reentrancy!

    HAPLogDebug(&logObject, "Removed timer: %lu", (unsigned long) timer);

    // Find timer.
    for (size_t i = 0; i < numActiveTimers; i++) {
        if (timers[i].id == timer) {
            HAPAssert(timers[i].callback);
            timers[i].callback = NULL;

            if (i >= numExpiredTimers) {
                // Move remaining timers.
                HAPRawBufferCopyBytes(&timers[i], &timers[i + 1], (numActiveTimers - i - 1) * sizeof timers[i]);
                numActiveTimers--;
                timers[numActiveTimers].id = timer;
            }

            return;
        }
    }

    // Timer not found.
    HAPLogError(&logObject, "Timer not found: %lu.", (unsigned long) timer);
    HAPFatalError();
}
