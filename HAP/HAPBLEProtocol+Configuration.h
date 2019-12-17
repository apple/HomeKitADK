// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_PROTOCOL_CONFIGURATION_H
#define HAP_BLE_PROTOCOL_CONFIGURATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Processes a HAP-Protocol-Configuration-Request.
 *
 * @param      server               Accessory server.
 * @param      session              Session on which the request came in.
 * @param      service              The service that received the request.
 * @param      accessory            The accessory that provides the service.
 * @param      requestReader        Reader to parse Protocol Configuration from. Reader content will become invalid.
 * @param[out] didRequestGetAll     true if Get-All-Params has been requested.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.16 HAP-Protocol-Configuration-Request
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEProtocolHandleConfigurationRequest(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        bool* didRequestGetAll,
        HAPPlatformKeyValueStoreRef keyValueStore);

/**
 * Serializes the body of a HAP-Protocol-Configuration-Response.
 *
 * @param      server               Accessory server.
 * @param      session              Session on which the request came in.
 * @param      service              The service that received the request.
 * @param      accessory            The accessory that provides the service.
 * @param      responseWriter       Writer to serialize Protocol Configuration into.
 * @param      keyValueStore        Key-value store.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.17 HAP-Protocol-Configuration-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEProtocolGetConfigurationResponse(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
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
