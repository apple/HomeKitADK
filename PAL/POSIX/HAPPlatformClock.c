// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <errno.h>
#include <sys/time.h>
#include <time.h>

#include "HAPPlatform.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "Clock" };

HAPTime HAPPlatformClockGetCurrent(void) {
    int e;

    static bool isInitialized;
    static HAPTime previousNow;

    // Get current time.
    HAPTime now;
#if defined(CLOCK_MONOTONIC_RAW)
    // This clock should be unaffected by frequency or time adjustments.

    if (!isInitialized) {
        HAPLog(&logObject, "Using 'clock_gettime' with 'CLOCK_MONOTONIC_RAW'.");
        isInitialized = true;
    }

    struct timespec t;
    e = clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    if (e) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPLogError(&logObject, "clock_gettime failed: %d.", _errno);
        HAPFatalError();
    }
    now = (HAPTime) t.tv_sec * 1000 + (HAPTime) t.tv_nsec / 1000000;

    if (now < previousNow) {
        HAPLog(&logObject, "Time jumped backwards by %lu ms.", (unsigned long) (previousNow - now));
        HAPFatalError();
    }
#else
    // Portable fallback clock.
    // Note: `gettimeofday` is susceptible to significant jumps as it can be changed remotely (e.g. through NTP).
    // We try to mitigate against the case of turning back the clock by keeping track of the time difference.
    // When the time jumps forward timers may complete early and operations may fail.
    // This may happen for example when the system time is re-synchronized after joining a different network.

    if (!isInitialized) {
        HAPLog(&logObject, "Using 'gettimeofday'.");
        isInitialized = true;
    }

    struct timeval t;
    e = gettimeofday(&t, NULL);
    if (e) {
        int _errno = errno;
        HAPAssert(e == -1);
        HAPLogError(&logObject, "gettimeofday failed: %d.", _errno);
        HAPFatalError();
    }
    now = (HAPTime) t.tv_sec * 1000 + (HAPTime) t.tv_usec / 1000;

    static HAPTime offset;
    if (now < previousNow) {
        HAPLog(&logObject, "Time jumped backwards by %lu ms. Adjusting offset.", (unsigned long) (previousNow - now));
        offset += previousNow - now;
    }
    now += offset;
#endif

    // Check for overflow.
    if (now & (1ull << 63)) {
        HAPLog(&logObject, "Time overflowed (capped at 2^63 - 1).");
        HAPFatalError();
    }

    previousNow = now;
    return now;
}
