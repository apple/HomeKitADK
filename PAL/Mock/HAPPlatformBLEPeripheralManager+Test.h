// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_TEST_H
#define HAP_PLATFORM_BLE_PERIPHERAL_MANAGER_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Returns whether a BLE peripheral manager is currently advertising.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 *
 * @return true                     If the BLE peripheral manager is currently advertising.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPPlatformBLEPeripheralManagerIsAdvertising(HAPPlatformBLEPeripheralManagerRef blePeripheralManager);

/**
 * Returns the Bluetooth device address (BD_ADDR) that is currently being advertised.
 *
 * - This can only be called if the BLE peripheral manager is currently advertising.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param[out] deviceAddress        Bluetooth device address.
 */
void HAPPlatformBLEPeripheralManagerGetDeviceAddress(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerDeviceAddress* deviceAddress);

/**
 * Fetches the currently advertised advertising data.
 *
 * - This can only be called if the BLE peripheral manager is currently advertising.
 *
 * @param      blePeripheralManager BLE peripheral manager.
 * @param[out] advertisingBytes     Advertising data buffer.
 * @param      maxAdvertisingBytes  Capacity of advertising data buffer.
 * @param[out] numAdvertisingBytes  Length of advertising data buffer.
 * @param[out] scanResponseBytes    Scan response data buffer.
 * @param      maxScanResponseBytes Capacity of scan response data buffer.
 * @param[out] numScanResponseBytes Length of scan response data buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffers are not large enough.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformBLEPeripheralManagerGetAdvertisingData(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        void* advertisingBytes,
        size_t maxAdvertisingBytes,
        size_t* numAdvertisingBytes,
        void* scanResponseBytes,
        size_t maxScanResponseBytes,
        size_t* numScanResponseBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
