// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ACCESSORY_INFO_H
#define HAP_ACCESSORY_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Serializes the body of a HAP-Info-Read-Response.
 *
 * @param      server               Accessory server.
 * @param      session              Session on which the request came in.
 * @param      accessory            The accessory on which the request came in.
 * @param      responseWriter       Writer to serialize Characteristic Signature into.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an I/O error occurred.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
HAPError HAPAccessoryGetInfoResponse(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
