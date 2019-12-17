// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PAIRING_PAIRINGS_H
#define HAP_PAIRING_PAIRINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Initializes Pairings state for a given session.
 *
 * @param      session              Session.
 */
void HAPPairingPairingsReset(HAPSessionRef* session);

/**
 * Processes a write request on the Pairings endpoint.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the request has been received.
 * @param      requestReader        TLV reader for parsing the value.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPairingPairingsHandleWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVReaderRef* requestReader);

/**
 * Processes a read request on the Pairings endpoint.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPairingPairingsHandleRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
