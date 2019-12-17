// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_MFI_TOKEN_AUTH_INIT_H
#define HAP_PLATFORM_MFI_TOKEN_AUTH_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * Software Token provider.
 *
 * The linked key-value store needs to be provisioned with software token information
 * before this implementation may be used. Please refer to the Provision tool.
 *
 * **Example**

   @code{.c}

   // Get dependencies.
   HAPPlatformKeyValueStoreRef keyValueStore;

   // Allocate Software Token provider.
   static HAPPlatformMFiTokenAuth mfiTokenAuth;

   // Initialize Software Token provider.
   HAPPlatformMFiTokenAuthCreate(&mfiTokenAuth,
       &(const HAPPlatformMFiTokenAuthOptions) {
           .keyValueStore = keyValueStore
       });

   @endcode
 */

/**
 * Software token provider initialization options.
 */
typedef struct {
    /**
     * Key-value store.
     */
    HAPPlatformKeyValueStoreRef keyValueStore;
} HAPPlatformMFiTokenAuthOptions;

/**
 * Software Token provider.
 */
struct HAPPlatformMFiTokenAuth {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    HAPPlatformKeyValueStoreRef keyValueStore;
    /**@endcond */
};

/**
 * Initializes Software Token provider.
 *
 * @param[out] mfiTokenAuth         Software Token provider.
 * @param      options              Initialization options.
 */
void HAPPlatformMFiTokenAuthCreate(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        const HAPPlatformMFiTokenAuthOptions* options);

/**
 * Returns whether a Software Token is provisioned.
 *
 * @param      mfiTokenAuth         Software Token provider.
 *
 * @return true                     If a Software Token is provisioned.
 * @return false                    Otherwise.
 */
bool HAPPlatformMFiTokenAuthIsProvisioned(HAPPlatformMFiTokenAuthRef mfiTokenAuth);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
