// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_SERVICE_SIGNATURE_H
#define HAP_BLE_SERVICE_SIGNATURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Serializes the body of a HAP-Service-Signature-Read-Response.
 *
 * @param      service              Service. NULL if the request had an invalid IID.
 * @param      responseWriter       Writer to serialize Service Signature into.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.13 HAP-Service-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEServiceGetSignatureReadResponse(const HAPService* _Nullable service, HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
