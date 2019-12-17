// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_INIT_H
#define HAP_PLATFORM_ACCESSORY_SETUP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_NFC
#define HAVE_NFC 0
#endif

#if HAVE_NFC
#include <pthread.h>
#include <nfc/nfc.h>
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Accessory setup manager initialization options.
 */
typedef struct {
    HAPPlatformKeyValueStore* keyValueStore;
} HAPPlatformAccessorySetupOptions;

/**
 * Accessory setup manager.
 */
struct HAPPlatformAccessorySetup {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
};

/**
 * Initializes the accessory setup manager.
 *
 * @param[out] accessorySetup       Pointer to an allocated but uninitialized HAPPlatformAccessorySetup structure.
 * @param      options              Initialization options.
 */
void HAPPlatformAccessorySetupCreate(
        HAPPlatformAccessorySetupRef accessorySetup,
        const HAPPlatformAccessorySetupOptions* options);

/**
 * Releases resources associated with an initialized accessory setup manager.
 *
 * @param      accessorySetup       Accessory setup manager.
 */
void HAPPlatformAccessorySetupRelease(HAPPlatformAccessorySetupRef accessorySetup);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
