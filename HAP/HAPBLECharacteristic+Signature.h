// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_CHARACTERISTIC_SIGNATURE_H
#define HAP_BLE_CHARACTERISTIC_SIGNATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Serializes the body of a HAP-Characteristic-Signature-Read-Response.
 *
 * @param      characteristic       Characteristic that received the request.
 * @param      service              The service that contains the characteristic.
 * @param      responseWriter       Writer to serialize Characteristic Signature into.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicGetSignatureReadResponse(
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
