// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_ACCESSORY_SERVER_H
#define HAP_BLE_ACCESSORY_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

struct HAPBLEAccessoryServerTransport {
    void (*create)(HAPAccessoryServerRef* server, const HAPAccessoryServerOptions* options);

    void (*validateAccessory)(const HAPAccessory* accessory);

    void (*prepareStart)(HAPAccessoryServerRef* server);

    void (*start)(HAPAccessoryServerRef* server);

    void (*tryStop)(HAPAccessoryServerRef* server, bool* didStop);

    HAP_RESULT_USE_CHECK
    HAPError (*didRaiseEvent)(
            HAPAccessoryServerRef* server_,
            const HAPCharacteristic* characteristic_,
            const HAPService* service,
            const HAPAccessory* accessory,
            HAPSessionRef* _Nullable session);

    void (*updateAdvertisingData)(HAPAccessoryServerRef* server);

    HAP_RESULT_USE_CHECK
    HAPError (*getGSN)(HAPPlatformKeyValueStoreRef keyValueStore, HAPBLEAccessoryServerGSN* gsn);

    struct {
        HAP_RESULT_USE_CHECK
        HAPError (*expireKey)(HAPPlatformKeyValueStoreRef keyValueStore);
    } broadcast;

    struct {
        void (*release)(HAPAccessoryServerRef* server_);

        void (*handleSessionAccept)(HAPAccessoryServerRef* server_, HAPSessionRef* session);

        void (*handleSessionInvalidate)(HAPAccessoryServerRef* server, HAPSessionRef* session);
    } peripheralManager;

    struct {
        void (*fetch)(
                HAPAccessoryServerRef* server,
                const HAPPairingBLESessionID* sessionID,
                uint8_t sharedSecret[_Nonnull X25519_SCALAR_BYTES],
                int* pairingID);

        void (*save)(
                HAPAccessoryServerRef* server,
                const HAPPairingBLESessionID* sessionID,
                uint8_t sharedSecret[_Nonnull X25519_SCALAR_BYTES],
                int pairingID);

        void (*invalidateEntriesForPairing)(HAPAccessoryServerRef* server_, int pairingID);
    } sessionCache;

    struct {
        void (*create)(HAPAccessoryServerRef* server, HAPSessionRef* session);

        void (*release)(HAPBLESession* bleSession);

        void (*invalidate)(HAPAccessoryServerRef* server, HAPBLESession* bleSession, bool terminateLink);

        void (*didStartPairingProcedure)(
                HAPAccessoryServerRef* server,
                HAPSessionRef* session,
                HAPPairingProcedureType pairingProcedureType);

        void (*didCompletePairingProcedure)(
                HAPAccessoryServerRef* server,
                HAPSessionRef* session,
                HAPPairingProcedureType pairingProcedureType);
    } session;
};

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
