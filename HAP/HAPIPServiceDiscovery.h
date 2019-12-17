// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_SERVICE_DISCOVERY_H
#define HAP_IP_SERVICE_DISCOVERY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * The currently active service.
 */
HAP_ENUM_BEGIN(uint8_t, HAPIPServiceDiscoveryType) { /**
                                                      * No Bonjour service discovery active.
                                                      */
                                                     kHAPIPServiceDiscoveryType_None,

                                                     /**
                                                      * _hap service is currently being advertised.
                                                      */
                                                     kHAPIPServiceDiscoveryType_HAP,

                                                     /**
                                                      * _mfi-config service is currently being advertised.
                                                      */
                                                     kHAPIPServiceDiscoveryType_MFiConfig
} HAP_ENUM_END(uint8_t, HAPIPServiceDiscoveryType);

/**
 * Registers or updates the Bonjour records for the _hap service.
 *
 * - Only one service may be active at a time. To switch services, first stop Bonjour service discovery.
 *
 * @param      server               Accessory server.
 */
void HAPIPServiceDiscoverySetHAPService(HAPAccessoryServerRef* server);

/**
 * Registers or updates the Bonjour records for the _mfi-config service.
 *
 * - Only one service may be active at a time. To switch services, first stop Bonjour service discovery.
 *
 * @param      server               Accessory server.
 */
void HAPIPServiceDiscoverySetMFiConfigService(HAPAccessoryServerRef* server);

/**
 * Stops Bonjour service discovery.
 *
 * @param      server               Accessory server.
 */
void HAPIPServiceDiscoveryStop(HAPAccessoryServerRef* server);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
