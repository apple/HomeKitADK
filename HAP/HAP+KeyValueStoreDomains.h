// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_KEY_VALUE_STORE_DOMAINS_H
#define HAP_KEY_VALUE_STORE_DOMAINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Statically provisioned data.
 *
 * Purged: Never.
 */
#define kHAPKeyValueStoreDomain_Provisioning ((HAPPlatformKeyValueStoreDomain) 0x80)

/**
 * Accessory configuration.
 *
 * Purged: On factory reset.
 */
#define kHAPKeyValueStoreDomain_Configuration ((HAPPlatformKeyValueStoreDomain) 0x90)

/**
 * HomeKit characteristic configuration.
 *
 * Purged: On factory reset.
 */
#define kHAPKeyValueStoreDomain_CharacteristicConfiguration ((HAPPlatformKeyValueStoreDomain) 0x92)

/**
 * HomeKit pairing data.
 *
 * Purged: On factory reset and on pairing reset.
 */
#define kHAPKeyValueStoreDomain_Pairings ((HAPPlatformKeyValueStoreDomain) 0xA0)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Device ID.
 *
 * Format: uint8_t[kHAPDeviceID_NumBytes].
 */
#define kHAPKeyValueStoreKey_Configuration_DeviceID ((HAPPlatformKeyValueStoreKey) 0x00)

/**
 * Firmware Version.
 *
 * Format: <major : uint32_t> <minor : uint32_t> <revision : uint32_t>, little endian.
 */
#define kHAPKeyValueStoreKey_Configuration_FirmwareVersion ((HAPPlatformKeyValueStoreKey) 0x10)

/**
 * Configuration number.
 *
 * Format: uint32_t, little endian.
 */
#define kHAPKeyValueStoreKey_Configuration_ConfigurationNumber ((HAPPlatformKeyValueStoreKey) 0x20)

/**
 * Ed25519 long-term secret key.
 *
 * Format: uint8_t[ED25519_SECRET_KEY_BYTES].
 */
#define kHAPKeyValueStoreKey_Configuration_LTSK ((HAPPlatformKeyValueStoreKey) 0x21)

/**
 * Unsuccessful authentication attempts counter.
 *
 * Format: uint8_t.
 */
#define kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts ((HAPPlatformKeyValueStoreKey) 0x22)

/**
 * BLE Global State Number.
 *
 * Format: <gsn : uint16_t> <didIncrement (0x01) : uint8_t>, little endian.
 */
#define kHAPKeyValueStoreKey_Configuration_BLEGSN ((HAPPlatformKeyValueStoreKey) 0x40)

/**
 * BLE Broadcast Encryption Key and Advertising Identifier.
 *
 * Format: little endian
 *     <keyExpirationGSN : uint16_t>
 *     <key : uint8_t[32]>
 *     <hasAdvertisingID (0x01) : uint8_t>
 *     <advertisingID : uint8_t[kHAPDeviceID_NumBytes]>
 */
#define kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters ((HAPPlatformKeyValueStoreKey) 0x41)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// kHAPKeyValueStoreDomain_CharacteristicConfiguration.
// Format: 2 bytes aid, n times <2 bytes cid + 1 byte broadcast interval>, little endian.
// Current implementation restriction: 42 cid's supported (2 + 42 * 3 = 128 bytes).
// Future format:
// - Add one more triple, cid == 0000 + 1 byte KVS key of continuation.
// - Replace aid with 0000 in continuations.
// Restricted to 16-bit aid / cid as Bluetooth LE does not support larger IDs.
// Could be worked around by re-using aid as version (currently aid is always 1).

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// kHAPKeyValueStoreDomain_Pairings.
// Format: little endian.
//     <identifier : uint8_t[36]>
//     <numIdentifierBytes : uint8_t>
//     <publicKey : uint8_t[ED25519_PUBLIC_KEY_BYTES]>
//     <permissions : uint8_t>

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
