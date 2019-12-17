// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_RUN_LOOP_INIT_H
#define HAP_PLATFORM_RUN_LOOP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Run loop initialization options.
 */
typedef struct {
    /**
     * Key-value store.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;
} HAPPlatformRunLoopOptions;

/**
 * Initializes the run loop.
 *
 * @param      options              Initialization options.
 */
void HAPPlatformRunLoopCreate(const HAPPlatformRunLoopOptions* options);

/**
 * Release run loop.
 */
void HAPPlatformRunLoopRelease(void);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
