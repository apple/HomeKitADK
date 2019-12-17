// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_SERVICE_DISCOVERY_TEST_H
#define HAP_PLATFORM_SERVICE_DISCOVERY_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Returns whether a service is currently being advertised.
 *
 * @param      serviceDiscovery     Service discovery.
 *
 * @return true                     If a service is currently being advertising.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPPlatformServiceDiscoveryIsAdvertising(HAPPlatformServiceDiscoveryRef serviceDiscovery);

/**
 * Returns the service name of the currently advertised service.
 *
 * - This can only be called if a service is currently being advertised.
 *
 * @param      serviceDiscovery     Service discovery.
 *
 * @return Service name.
 */
HAP_RESULT_USE_CHECK
const char* HAPPlatformServiceDiscoveryGetName(HAPPlatformServiceDiscoveryRef serviceDiscovery);

/**
 * Returns the protocol name of the currently advertised service.
 *
 * - This can only be called if a service is currently being advertised.
 *
 * @param      serviceDiscovery     Service discovery.
 *
 * @return Protocol name.
 */
HAP_RESULT_USE_CHECK
const char* HAPPlatformServiceDiscoveryGetProtocol(HAPPlatformServiceDiscoveryRef serviceDiscovery);

/**
 * Returns the port number of the currently advertised service.
 *
 * - This can only be called if a service is currently being advertised.
 *
 * @param      serviceDiscovery     Service discovery.
 *
 * @return Port number.
 */
HAP_RESULT_USE_CHECK
HAPNetworkPort HAPPlatformServiceDiscoveryGetPort(HAPPlatformServiceDiscoveryRef serviceDiscovery);

/**
 * Callback that should be invoked for each TXT record.
 *
 * @param      context              Context.
 * @param      serviceDiscovery     Service discovery.
 * @param      key                  Key of the TXT record.
 * @param      valueBytes           Buffer containing TXT value. NULL-terminated for convenience.
 * @param      numValueBytes        Length of TXT value.
 * @param[in,out] shouldContinue    True if enumeration shall continue, False otherwise. Is set to true on input.
 */
typedef void (*HAPPlatformServiceDiscoveryEnumerateTXTRecordsCallback)(
        void* _Nullable context,
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        const char* key,
        const void* valueBytes,
        size_t numValueBytes,
        bool* shouldContinue);

/**
 * Enumerates all TXT records of the currently advertised service.
 *
 * - This can only be called if a service is currently being advertised.
 *
 * @param      serviceDiscovery     Service discovery.
 * @param      callback             Function to call on each TXT record.
 * @param      context              Context that is passed to the callback.
 */
void HAPPlatformServiceDiscoveryEnumerateTXTRecords(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPPlatformServiceDiscoveryEnumerateTXTRecordsCallback callback,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
