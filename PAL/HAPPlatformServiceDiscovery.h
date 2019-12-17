// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_SERVICE_DISCOVERY_H
#define HAP_PLATFORM_SERVICE_DISCOVERY_H

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
typedef struct HAPPlatformServiceDiscovery HAPPlatformServiceDiscovery;
typedef struct HAPPlatformServiceDiscovery* HAPPlatformServiceDiscoveryRef;
HAP_NONNULL_SUPPORT(HAPPlatformServiceDiscovery)

/**
 * TXT record.
 */
typedef struct {
    /** Key of the TXT record. */
    const char* key;

    /** Value of the TXT record. */
    struct {
        /** Buffer containing TXT value. */
        const void* _Nullable bytes;

        /** Length of TXT value. */
        size_t numBytes;
    } value;
} HAPPlatformServiceDiscoveryTXTRecord;

/**
 * Starts Bonjour service discovery.
 *
 * @param      serviceDiscovery     Service discovery.
 * @param      name                 Service name.
 * @param      protocol             Protocol name.
 * @param      port                 Port number.
 * @param      txtRecords           Array of TXT records.
 * @param      numTXTRecords        Number of TXT records.
 */
void HAPPlatformServiceDiscoveryRegister(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const char* name,
        const char* protocol,
        HAPNetworkPort port,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords);

/**
 * Updates the TXT records of a running Bonjour service discovery.
 *
 * @param      serviceDiscovery     Service discovery.
 * @param      txtRecords           Array of TXT records.
 * @param      numTXTRecords        Number of TXT records.
 */
void HAPPlatformServiceDiscoveryUpdateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryTXTRecord* txtRecords,
        size_t numTXTRecords);

/**
 * Stops Bonjour service discovery.
 *
 * @param      serviceDiscovery     Service discovery.
 */
void HAPPlatformServiceDiscoveryStop(HAPPlatformServiceDiscoveryRef serviceDiscovery);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
