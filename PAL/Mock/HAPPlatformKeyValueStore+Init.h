// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_KEY_VALUE_STORE_INIT_H
#define HAP_PLATFORM_KEY_VALUE_STORE_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * RAM-based ephemeral key-value store implementation.
 *
 * **Example**

   @code{.c}

   // Allocate memory for key-value store items.
   // Necessary amount may differ depending on usage.
   static HAPPlatformKeyValueStoreItem keyValueStoreItems[32];

   // Allocate key-value store.
   static HAPPlatformKeyValueStore keyValueStore;

   // Initialize key-value store.
   HAPPlatformKeyValueStoreCreate(&keyValueStore,
       &(const HAPPlatformKeyValueStoreOptions) {
           .items = keyValueStoreItems,
           .numItems = HAPArrayCount(keyValueStoreItems)
       });

   @endcode
 */

/**
 * Key-value store item.
 *
 * - Each item stores the value of one key in RAM. Values are not stored persistently.
 */
typedef struct {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    bool active;
    HAPPlatformKeyValueStoreDomain domain;
    HAPPlatformKeyValueStoreKey key;
    size_t numBytes;
    uint8_t bytes[128];
    /**@endcond */
} HAPPlatformKeyValueStoreItem;

/**
 * Key-value store initialization options.
 */
typedef struct {
    /**
     * Buffer to store key-value store items.
     */
    HAPPlatformKeyValueStoreItem* items;

    /**
     * Number of items.
     */
    size_t numItems;
} HAPPlatformKeyValueStoreOptions;

/**
 * Key-value store.
 */
struct HAPPlatformKeyValueStore {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    HAPPlatformKeyValueStoreItem* bytes;
    size_t maxBytes;
    /**@endcond */
};

/**
 * Initializes the key-value store.
 *
 * @param[out] keyValueStore        Pointer to an allocated but uninitialized HAPPlatformKeyValueStore structure.
 * @param      options              Initialization options.
 */
void HAPPlatformKeyValueStoreCreate(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPPlatformKeyValueStoreOptions* options);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
