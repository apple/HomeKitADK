// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPPlatformClock+Test.h"
#include "HAPPlatformTimer+Init.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "Clock" };

static HAPTime now;

HAPTime HAPPlatformClockGetCurrent(void) {
    // Check for overflow.
    if (now & (1ull << 63)) {
        HAPLog(&logObject, "Time overflowed (capped at 2^63 - 1).");
        HAPFatalError();
    }
    return now;
}

void HAPPlatformClockAdvance(HAPTime delta) {
    now += delta;
    HAPLogInfo(
            &logObject,
            "Clock advanced to %8llu.%03llu",
            (unsigned long long) (now / HAPSecond),
            (unsigned long long) (now % HAPSecond));

    HAPPlatformTimerProcessExpiredTimers();
}
