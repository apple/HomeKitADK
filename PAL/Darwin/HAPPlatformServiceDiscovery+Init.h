// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_SERVICE_DISCOVERY_INIT_H
#define HAP_PLATFORM_SERVICE_DISCOVERY_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <dns_sd.h>
#include <net/if.h>

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * Bonjour service discovery for Darwin based on NSNetService.
 *
 * **Example**

   @code{.c}

   // Allocate service discovery object.
   static HAPPlatformServiceDiscovery serviceDiscovery;

   // Initialize service discovery object. Depends on a TCP stream manager instance.
   HAPPlatformServiceDiscoveryCreate(&platform.serviceDiscovery,
       &(const HAPPlatformServiceDiscoveryOptions) {
           // Register services on all available network interfaces.
           .interfaceName = NULL
       });

   @endcode
 */

/**
 * Service discovery initialization options.
 */
typedef struct {
    /**
     * Local network interface name on which to register services.
     * A value of NULL will use all available network interfaces.
     */
    const char* _Nullable interfaceName;
} HAPPlatformServiceDiscoveryOptions;

/**
 * Service discovery memory configuration value.
 */
#define kHAPPlatformServiceDiscovery_MaxTXTRecordBufferBytes (512)

/**
 * Service discovery.
 */
struct HAPPlatformServiceDiscovery {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char _;
    /**@endcond */
};

/**
 * Initializes service discovery.
 *
 * @param      serviceDiscovery     Pointer to an allocated but uninitialized HAPPlatformServiceDiscovery structure.
 * @param      options              Initialization options.
 */
void HAPPlatformServiceDiscoveryCreate(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const HAPPlatformServiceDiscoveryOptions* options);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
