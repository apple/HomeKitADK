// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_ACCESSORY_SERVER_BROADCAST_H
#define HAP_BLE_ACCESSORY_SERVER_BROADCAST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * BLE: Broadcast encryption key.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.7.3 Broadcast Encryption Key Generation
 */
typedef struct {
    uint8_t value[32]; /**< Value. */
} HAPBLEAccessoryServerBroadcastEncryptionKey;
HAP_STATIC_ASSERT(
        sizeof(HAPBLEAccessoryServerBroadcastEncryptionKey) == 32,
        HAPBLEAccessoryServerBroadcastEncryptionKey);
HAP_NONNULL_SUPPORT(HAPBLEAccessoryServerBroadcastEncryptionKey)

/**
 * BLE: Fetches broadcast encryption key parameters.
 *
 * @param      keyValueStore        Key-value store.
 * @param[out] keyExpirationGSN     GSN after which the broadcast encryption key expires. 0 if key is expired.
 * @param[out] broadcastKey         Broadcast encryption key, if available.
 * @param[out] advertisingID        Accessory advertising identifier.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.7.3 Broadcast Encryption Key Generation
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastGetParameters(
        HAPPlatformKeyValueStoreRef keyValueStore,
        uint16_t* keyExpirationGSN,
        HAPBLEAccessoryServerBroadcastEncryptionKey* _Nullable broadcastKey,
        HAPDeviceID* _Nullable advertisingID);

/**
 * BLE: Generate a new broadcast encryption key.
 *
 * @param      session              Session that requested generation of the broadcast encryption key. Must be secured.
 * @param      advertisingID        New accessory advertising identifier. If null, the identifier is not changed.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.7.3 Broadcast Encryption Key Generation
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastGenerateKey(HAPSessionRef* session, const HAPDeviceID* _Nullable advertisingID);

/**
 * BLE: Set accessory advertising identifier.
 *
 * @param      keyValueStore        Key-value store.
 * @param      advertisingID        New accessory advertising identifier.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.7.3 Broadcast Encryption Key Generation
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastSetAdvertisingID(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPDeviceID* advertisingID);

/**
 * BLE: Invalidate broadcast encryption key.
 *
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.7.4 Broadcast Encryption Key expiration and refresh
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastExpireKey(HAPPlatformKeyValueStoreRef keyValueStore);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
