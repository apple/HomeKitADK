// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_ACCESSORY_SERVER_ADVERTISING_H
#define HAP_BLE_ACCESSORY_SERVER_ADVERTISING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * BLE: GSN state.
 */
typedef struct {
    uint16_t gsn;          /**< Global State Number. */
    bool didIncrement : 1; /**< Whether GSN has been incremented in the current connect / disconnect cycle. */
} HAPBLEAccessoryServerGSN;

/**
 * BLE: Fetches GSN state.
 *
 * @param      keyValueStore        Key-value store
 * @param[out] gsn                  GSN.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerGetGSN(HAPPlatformKeyValueStoreRef keyValueStore, HAPBLEAccessoryServerGSN* gsn);

/**
 * BLE: Get advertisement parameters.
 *
 * @param      server               Accessory server.
 * @param[out] isActive             True if advertisement should be active, False otherwise.
 * @param[out] advertisingInterval  Advertising interval in milliseconds, if active.
 * @param[out] advertisingBytes     Advertising data, if active.
 * @param      maxAdvertisingBytes  Capacity of advertising data buffer. Must be at least 31.
 * @param[out] numAdvertisingBytes  Effective length of advertising data buffer, if active.
 * @param[out] scanResponseBytes    Scan response data, if active.
 * @param      maxScanResponseBytes Capacity of scan response data buffer. Should be >= 2.
 * @param      numScanResponseBytes Effective length of scan response data buffer, or 0, if not applicable, if active.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerGetAdvertisingParameters(
        HAPAccessoryServerRef* server,
        bool* isActive,
        uint16_t* advertisingInterval,
        void* advertisingBytes,
        size_t maxAdvertisingBytes,
        size_t* numAdvertisingBytes,
        void* scanResponseBytes,
        size_t maxScanResponseBytes,
        size_t* numScanResponseBytes)
        HAP_DIAGNOSE_ERROR(maxAdvertisingBytes < 31, "maxAdvertisingBytes must be at least 31")
                HAP_DIAGNOSE_WARNING(maxScanResponseBytes < 2, "maxScanResponseBytes should be at least 2");

/**
 * BLE: Informs the accessory server that advertising has started.
 *
 * @param      server               Accessory server.
 */
void HAPBLEAccessoryServerDidStartAdvertising(HAPAccessoryServerRef* server);

/**
 * BLE: Informs the accessory server that a controller has connected.
 *
 * @param      server               Accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a controller is already connected. Only 1 concurrent connection is allowed.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidConnect(HAPAccessoryServerRef* server);

/**
 * BLE: Informs the accessory server that a controller has disconnected.
 *
 * @param      server               Accessory server.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no controller is connected.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidDisconnect(HAPAccessoryServerRef* server);

/**
 * BLE: Informs the accessory server that the value of a characteristic did change.
 *
 * @param      server               Accessory server.
 * @param      characteristic       The characteristic whose value has changed.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      session              The session on which to raise the event.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidRaiseEvent(
        HAPAccessoryServerRef* server,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPSessionRef* _Nullable session);

/**
 * BLE: Informs the accessory server that the value of a characteristic which is registered for Bluetooth LE
 * indications changed.
 *
 * @param      server               Accessory server.
 * @param      characteristic       The characteristic whose value has changed.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerDidSendEventNotification(
        HAPAccessoryServerRef* server,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
