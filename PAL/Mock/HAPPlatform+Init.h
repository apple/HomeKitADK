// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_INIT_H
#define HAP_PLATFORM_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"
#include "HAPPlatformClock+Test.h"
#include "HAPPlatformTCPStreamManager+Test.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Global platform object.
 */
extern HAPPlatform platform;

/**
 * Initializes the platform.
 */
void HAPPlatformCreate(void);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
