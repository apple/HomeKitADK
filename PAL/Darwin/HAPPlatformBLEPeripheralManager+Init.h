// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_INIT_H
#define HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * BLE peripheral manager initialization options.
 */
typedef struct {
    /**
     * Key-value store.
     * If critical data is pending, BLE reads will be delayed until the data is persisted.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;
} HAPPlatformBLEPeripheralManagerOptions;

/**
 * BLE peripheral manager.
 */
struct HAPPlatformBLEPeripheralManager {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
};

/**
 * Initializes the BLE peripheral manager.
 *
 * @param[out] blePeripheralManager Pointer to an allocated but uninitialized HAPPlatformBLEPeripheralManager structure.
 * @param      options              Initialization options.
 */
void HAPPlatformBLEPeripheralManagerCreate(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        const HAPPlatformBLEPeripheralManagerOptions* options);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
