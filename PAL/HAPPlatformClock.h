// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_CLOCK_H
#define HAP_PLATFORM_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Gets the current system time expressed as milliseconds relative to an implementation-defined time in the past.
 *
 * - The clock must not jump backwards.
 * - The clock must have a minimum resolution of 100ms.
 * - The clock does not have to be persisted across system restarts.
 * - The clock may be suspended to conserve power when no timers are scheduled and no connection is active.
 *
 * @return Clock in milliseconds. Cannot exceed 2^63 - 1 to allow implementations to add offsets without overflow.
 */
HAPTime HAPPlatformClockGetCurrent(void);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
