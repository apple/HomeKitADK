// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_CHARACTERISTIC_BROADCAST_H
#define HAP_BLE_CHARACTERISTIC_BROADCAST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Broadcast interval.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-30 Broadcast Interval
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLECharacteristicBroadcastInterval) { /** 20 ms (Default). */
                                                                 kHAPBLECharacteristicBroadcastInterval_20Ms = 0x01,

                                                                 /** 1280 ms. */
                                                                 kHAPBLECharacteristicBroadcastInterval_1280Ms = 0x02,

                                                                 /** 2560 ms. */
                                                                 kHAPBLECharacteristicBroadcastInterval_2560Ms = 0x03
} HAP_ENUM_END(uint8_t, HAPBLECharacteristicBroadcastInterval);

/**
 * Checks whether a value represents a valid broadcast interval.
 *
 * @param      value                Value to check.
 *
 * @return true                     If the value is valid.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLECharacteristicIsValidBroadcastInterval(uint8_t value);

/**
 * Gets the broadcast configuration of a characteristic.
 *
 * @param      characteristic       Characteristic. Characteristic must support broadcasts.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param[out] broadcastsEnabled    Whether broadcast notifications are enabled.
 * @param[out] broadcastInterval    Broadcast interval, if broadcast notifications are enabled.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.5.8 HAP Characteristic Configuration Procedure
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetBroadcastConfiguration(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        bool* broadcastsEnabled,
        HAPBLECharacteristicBroadcastInterval* broadcastInterval,
        HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Enables broadcasts for a characteristic.
 *
 * @param      characteristic       Characteristic. Characteristic must support broadcasts.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      broadcastInterval    Broadcast interval.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.5.8 HAP Characteristic Configuration Procedure
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicEnableBroadcastNotifications(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPBLECharacteristicBroadcastInterval broadcastInterval,
        HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Disables broadcasts for a characteristic.
 *
 * @param      characteristic       Characteristic. Characteristic must support broadcasts.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.5.8 HAP Characteristic Configuration Procedure
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicDisableBroadcastNotifications(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPPlatformKeyValueStoreRef keyValueStore);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
