// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairSetupRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairSetupRead(server, HAPNonnull(request->session), responseWriter);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairSetupWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairSetupWrite(server, request->session, requestReader);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairVerifyRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairVerifyRead(server, HAPNonnull(request->session), responseWriter);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairVerifyWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairVerifyWrite(server, request->session, requestReader);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingFeaturesRead(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicReadRequest* request HAP_UNUSED,
        uint8_t* value,
        void* _Nullable context HAP_UNUSED) {
    *value = HAPAccessoryServerGetPairingFeatureFlags(server);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingPairingsRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairingsRead(server, HAPNonnull(request->session), responseWriter);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandlePairingPairingPairingsWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request);
    HAPPrecondition(request->session);

    return HAPSessionHandlePairingsWrite(server, request->session, requestReader);
}
