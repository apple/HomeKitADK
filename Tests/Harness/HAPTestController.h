// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_CONTROLLER_H
#define HAP_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

typedef struct {
    /** Name. */
    char name[65];

    /** Configuration number. */
    uint32_t configurationNumber;

    /** Pairing Feature flags. Only set for IP accessories. */
    struct {
        /** Whether or not Apple Authentication Coprocessor is supported. */
        bool supportsMFiHWAuth : 1;

        /** Whether or not Software Authentication is supported. */
        bool supportsMFiTokenAuth : 1;
    } pairingFeatureFlags;

    /** Device ID. */
    HAPAccessoryServerDeviceID deviceID;

    /** Model. Only set for IP accessories. */
    char model[65];

    /** Protocol version. Only set for IP accessories. */
    struct {
        uint8_t major; /**< Major version number. */
        uint8_t minor; /**< Minor version number. */
    } protocolVersion;

    /** Current state number. */
    uint16_t stateNumber;

    /** Status flags. */
    struct {
        /** Whether or not the accessory has not been paired with any controllers. */
        bool isNotPaired : 1;

        /** Whether or not the accessory has not been configured to join a Wi-Fi network. */
        bool isWiFiNotConfigured : 1;

        /** Whether or not a problem has been detected on the accessory. */
        bool hasProblem : 1;
    } statusFlags;

    /** Category. */
    HAPAccessoryCategory category;

    /** Setup hash. */
    struct {
        uint8_t bytes[4]; /**< Value. */
        bool isSet : 1;   /**< Whether a setup hash is set. */
    } setupHash;
} HAPAccessoryServerInfo;

/**
 * Discovers an IP accessory server.
 *
 * @param      serviceDiscovery     Service discovery.
 * @param[out] serverInfo           Accessory server info.
 * @param[out] serverPort           Port number under which the accessory server is listening.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no IP accessory server is currently being advertised.
 * @return kHAPError_InvalidData    If the advertised data is malformed.
 */
HAPError HAPDiscoverIPAccessoryServer(
        HAPPlatformServiceDiscoveryRef serviceDiscovery,
        HAPAccessoryServerInfo* serverInfo,
        HAPNetworkPort* serverPort);

/**
 * Discovers a BLE accessory server.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param[out] serverInfo           Accessory server info.
 * @param[out] deviceAddress        Bluetooth device address under which the accessory server is listening.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no BLE accessory server is currently being advertised.
 * @return kHAPError_InvalidData    If the advertised data is malformed.
 */
HAPError HAPDiscoverBLEAccessoryServer(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPAccessoryServerInfo* serverInfo,
        HAPPlatformBLEPeripheralManagerDeviceAddress* deviceAddress);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
