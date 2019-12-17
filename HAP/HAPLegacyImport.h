// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_LEGACY_IMPORT_H
#define HAP_LEGACY_IMPORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Device ID of an accessory server.
 */
typedef struct HAP_ATTRIBUTE_GCC(packed) {
    /**
     * Device ID.
     */
    uint8_t bytes[6];
} HAPAccessoryServerDeviceID;

/**
 * Imports a device ID into an un-provisioned key-value store.
 * This is useful to import legacy settings from a different HomeKit SDK.
 *
 * - This function must no longer be called after the initial HAPAccessoryServerCreate call.
 *
 * @param      keyValueStore        Un-provisioned key-value store.
 * @param      deviceID             DeviceID of the accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accessing the key-value store.
 */
HAP_ATTRIBUTE_GCC(warn_unused_result)
HAPError HAPLegacyImportDeviceID(HAPPlatformKeyValueStore* keyValueStore, const HAPAccessoryServerDeviceID* deviceID);

/**
 * Imports a configuration number into an un-provisioned key-value store.
 * This is useful to import legacy settings from a different HomeKit SDK.
 *
 * - This function must no longer be called after the initial HAPAccessoryServerCreate call.
 *
 * @param      keyValueStore        Un-provisioned key-value store.
 * @param      configurationNumber  Configuration number.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accessing the key-value store.
 */
HAP_ATTRIBUTE_GCC(warn_unused_result)
HAPError HAPLegacyImportConfigurationNumber(HAPPlatformKeyValueStore* keyValueStore, uint32_t configurationNumber);

/**
 * Ed25519 long-term secret key of an accessory server.
 */
typedef struct HAP_ATTRIBUTE_GCC(packed) {
    /**
     * Ed25519 long-term secret key.
     */
    uint8_t bytes[32];
} HAPAccessoryServerLongTermSecretKey;

/**
 * Imports a Ed25519 long-term secret key into an un-provisioned key-value store.
 * This is useful to import legacy settings from a different HomeKit SDK.
 *
 * - This function must no longer be called after the initial HAPAccessoryServerCreate call.
 *
 * @param      keyValueStore        Un-provisioned key-value store.
 * @param      longTermSecretKey    Long-term secret key of the accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accessing the key-value store.
 */
HAP_ATTRIBUTE_GCC(warn_unused_result)
HAPError HAPLegacyImportLongTermSecretKey(
        HAPPlatformKeyValueStore* keyValueStore,
        const HAPAccessoryServerLongTermSecretKey* longTermSecretKey);

/**
 * Imports an unsuccessful authentication attempts counter into an un-provisioned key-value store.
 * This is useful to import legacy settings from a different HomeKit SDK.
 *
 * - This function must no longer be called after the initial HAPAccessoryServerCreate call.
 *
 * @param      keyValueStore        Un-provisioned key-value store.
 * @param      numAuthAttempts      Unsuccessful authentication attempts counter.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accessing the key-value store.
 */
HAP_ATTRIBUTE_GCC(warn_unused_result)
HAPError HAPLegacyImportUnsuccessfulAuthenticationAttemptsCounter(
        HAPPlatformKeyValueStore* keyValueStore,
        uint8_t numAuthAttempts);

/**
 * Pairing identifier of a paired controller.
 */
typedef struct HAP_ATTRIBUTE_GCC(packed) {
    /**
     * Buffer containing pairing identifier.
     */
    uint8_t bytes[36];

    /**
     * Number of used bytes in buffer.
     */
    size_t numBytes;
} HAPControllerPairingIdentifier;

/**
 * Public key of a paired controller.
 */
typedef struct HAP_ATTRIBUTE_GCC(packed) {
    /**
     * Public key.
     */
    uint8_t bytes[32];
} HAPControllerPublicKey;

/**
 * Imports a controller pairing into an un-provisioned key-value store.
 * This is useful to import legacy settings from a different HomeKit SDK.
 *
 * - This function must no longer be called after the initial HAPAccessoryServerCreate call.
 *
 * @param      keyValueStore        Un-provisioned key-value store.
 * @param      pairingIndex         Key-value store pairing index. 0 ..< Max number of pairings that will be supported.
 * @param      pairingIdentifier    HomeKit pairing identifier.
 * @param      publicKey            Ed25519 long-term public key of the paired controller.
 * @param      isAdmin              Whether or not the added controller has admin permissions.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while accessing the key-value store.
 */
HAP_ATTRIBUTE_GCC(warn_unused_result)
HAPError HAPLegacyImportControllerPairing(
        HAPPlatformKeyValueStore* keyValueStore,
        HAPPlatformKeyValueStoreKey pairingIndex,
        const HAPControllerPairingIdentifier* pairingIdentifier,
        const HAPControllerPublicKey* publicKey,
        bool isAdmin);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
