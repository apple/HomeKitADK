// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_CHARACTERISTIC_CONFIGURATION_H
#define HAP_BLE_CHARACTERISTIC_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Processes a HAP-Characteristic-Configuration-Request.
 *
 * @param      characteristic       Characteristic that received the request.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      requestReader        Reader to parse Characteristic Configuration from. Reader content becomes invalid.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.14 HAP-Characteristic-Configuration-Request
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicHandleConfigurationRequest(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Serializes the body of a HAP-Characteristic-Configuration-Response.
 *
 * @param      characteristic       Characteristic that received the request.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      responseWriter       Writer to serialize Characteristic Configuration into.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.15 HAP-Characteristic-Configuration-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetConfigurationResponse(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter,
        HAPPlatformKeyValueStoreRef keyValueStore);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
