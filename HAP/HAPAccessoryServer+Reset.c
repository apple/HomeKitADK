// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
HAPError HAPRestoreFactorySettings(HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(keyValueStore);

    HAPError err;

    // Erase persistent store.
    static const HAPPlatformKeyValueStoreDomain domainsToPurge[] = {
        kHAPKeyValueStoreDomain_Configuration,
        kHAPKeyValueStoreDomain_CharacteristicConfiguration,
        kHAPKeyValueStoreDomain_Pairings
    };
    for (size_t i = 0; i < HAPArrayCount(domainsToPurge); i++) {
        err = HAPPlatformKeyValueStorePurgeDomain(keyValueStore, domainsToPurge[i]);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPRemoveAllPairings(HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(keyValueStore);

    HAPError err;

    // Erase persistent store.
    static const HAPPlatformKeyValueStoreDomain domainsToPurge[] = { kHAPKeyValueStoreDomain_Pairings };
    for (size_t i = 0; i < HAPArrayCount(domainsToPurge); i++) {
        err = HAPPlatformKeyValueStorePurgeDomain(keyValueStore, domainsToPurge[i]);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleFirmwareUpdate(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPlatformKeyValueStoreRef keyValueStore = server->platform.keyValueStore;

    HAPError err;

    // Increment CN.
    // See HomeKit Accessory Protocol Specification R14
    // Table 6-7 _hap._tcp Bonjour TXT Record Keys
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.2.1.2 Manufacturer Data
    err = HAPAccessoryServerIncrementCN(keyValueStore);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // BLE: Reset GSN.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.1.8 Global State Number (GSN)
    if (server->transports.ble) {
        err = HAPPlatformKeyValueStoreRemove(
                keyValueStore, kHAPKeyValueStoreDomain_Configuration, kHAPKeyValueStoreKey_Configuration_BLEGSN);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    // BLE: Reset Broadcast Encryption Key.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.7.4 Broadcast Encryption Key expiration and refresh
    if (server->transports.ble) {
        err = HAPNonnull(server->transports.ble)->broadcast.expireKey(keyValueStore);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    return kHAPError_None;
}
