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

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * Accessory setup manager.
 *
 * - The linked key-value store needs to be provisioned with accessory setup information
 *   before this implementation may be used. Please refer to the Provision tool.
 *
 * **Example**

   @code{.c}

   // Get dependencies.
   HAPPlatformKeyValueStoreRef keyValueStore;

   // Allocate accessory setup manager.
   static HAPPlatformAccessorySetup accessorySetup;

   // Determine accessory capabilities.
   bool HAVE_DISPLAY;
   bool HAVE_NFC;
   const char *LIBNFC_CONNECTION_STRING; // Only applicable if programmable NFC is used.

   // Initialize accessory setup manager.
   HAPPlatformAccessorySetupCreate(&accessorySetup,
       &(const HAPPlatformAccessorySetupOptions) {
           .keyValueStore = keyValueStore,
           .useDisplay = HAVE_DISPLAY,
           .useProgrammableNFC = HAVE_NFC,
           .libnfcConnectionString = HAVE_NFC ? LIBNFC_CONNECTION_STRING : NULL
       });

   @endcode
 */

/**
 * Accessory setup manager initialization options.
 */
typedef struct {
    /**
     * Key-value store.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;
} HAPPlatformAccessorySetupOptions;

/**
 * Accessory setup manager.
 */
struct HAPPlatformAccessorySetup {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    HAPPlatformKeyValueStoreRef keyValueStore;
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

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
