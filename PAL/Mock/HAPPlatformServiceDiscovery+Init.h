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

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Service discovery.
 */
struct HAPPlatformServiceDiscovery {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    char name[65];
    char protocol[31];
    struct {
        char key[9];
        struct {
            uint8_t bytes[22];
            uint8_t numBytes;
        } value;
    } txtRecords[16];
    HAPNetworkPort port;
    /**@endcond */
};

/**
 * Initializes a service discovery.
 *
 * @param[out] serviceDiscovery     Pointer to an allocated but uninitialized HAPPlatformServiceDiscovery structure.
 */
void HAPPlatformServiceDiscoveryCreate(HAPPlatformServiceDiscoveryRef serviceDiscovery);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
