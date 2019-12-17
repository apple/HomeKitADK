// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_CHARACTERISTIC_H
#define HAP_BLE_CHARACTERISTIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Returns whether a characteristic supports HAP-BLE service procedures.
 *
 * @param      characteristic       Characteristic.
 *
 * @return true                     If the characteristic supports HAP-BLE service procedures.
 * @return false                    Otherwise.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.12 HAP-Service-Signature-Read-Request
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.13 HAP-Service-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
bool HAPBLECharacteristicSupportsServiceProcedures(const HAPCharacteristic* characteristic);

/**
 * Returns whether accessing a characteristic over BLE should drop the security session.
 *
 * @param      characteristic       Characteristic.
 *
 * @return true                     If the security session should be dropped if the characteristic is accessed.
 * @return false                    Otherwise.
 */
bool HAPBLECharacteristicDropsSecuritySession(const HAPCharacteristic* characteristic);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
