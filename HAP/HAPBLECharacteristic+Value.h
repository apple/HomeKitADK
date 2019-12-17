// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_CHARACTERISTIC_VALUE_H
#define HAP_BLE_CHARACTERISTIC_VALUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Reads the value of a characteristic, and serializes the body of a HAP-Characteristic-Read-Response.
 *
 * @param      server               Accessory server.
 * @param      session              Session on which the request came in.
 * @param      characteristic       Characteristic that received the request.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      responseWriter       Writer to serialize Characteristic value into.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request, or writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.7 HAP-Characteristic-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicReadAndSerializeValue(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter);

/**
 * Parses the body of a a HAP-Characteristic-Write-Request, and writes the value to a characteristic.
 *
 * @param      server               Accessory server.
 * @param      session              Session on which the request came in.
 * @param      characteristic       Characteristic that received the request.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 * @param      requestReader        Reader to parse Characteristic value from. Reader content will become invalid.
 * @param      timedWriteStartTime  Time of initial request if a Timed Write procedure is in progress; NULL otherwise.
 * @param[out] hasExpired           True if Timed Write procedure has expired; False otherwise.
 * @param[out] hasReturnResponse    True if a Return-Response TLV item was present and set to 1; False otherwise.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request, or an out-of-range value.
 * @return kHAPError_OutOfResources If out of resources to process request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.4 HAP-Characteristic-Write-Request
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicParseAndWriteValue(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        const HAPTime* _Nullable timedWriteStartTime,
        bool* hasExpired,
        bool* hasReturnResponse);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
