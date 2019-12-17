// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_CHARACTERISTIC_H
#define HAP_IP_CHARACTERISTIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Returns whether a characteristic supports HAP over IP (Ethernet / Wi-Fi).
 *
 * - Certain characteristics are only applicable to HAP over Bluetooth LE.
 *
 * @param      characteristic       Characteristic.
 *
 * @return true                     If the characteristic supports HAP over IP (Ethernet / Wi-Fi).
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPIPCharacteristicIsSupported(const HAPCharacteristic* characteristic);

/**
 * Returns the number of enabled properties of a characteristic.
 *
 * @param      characteristic       Characteristic.
 *
 * @return Number of enabled properties.
 */
HAP_RESULT_USE_CHECK
size_t HAPCharacteristicGetNumEnabledProperties(const HAPCharacteristic* characteristic);

/**
 * Returns the unit of the characteristic value.
 *
 * @param      characteristic       Characteristic.
 *
 * @return Unit of the characteristic value.
 */
HAP_RESULT_USE_CHECK
HAPCharacteristicUnits HAPCharacteristicGetUnit(const HAPCharacteristic* characteristic);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
