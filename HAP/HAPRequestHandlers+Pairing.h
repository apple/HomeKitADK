// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_REQUEST_HANDLERS_PAIRING_H
#define HAP_REQUEST_HANDLERS_PAIRING_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Handle read request to the 'Pair Setup' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairSetupRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle write request to the 'Pair Setup' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairSetupWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

/**
 * Handle read request to the 'Pair Verify' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairVerifyRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle write request to the 'Pair Verify' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairVerifyWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

/**
 * Handle read request to the 'Pairing Features' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingFeaturesRead(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicReadRequest* request,
        uint8_t* value,
        void* _Nullable context);

/**
 * Handle read request to the 'Pairing Pairings' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingPairingsRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Handle write request to the 'Pairing Pairings' characteristic of the Pairing service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingPairingsWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
