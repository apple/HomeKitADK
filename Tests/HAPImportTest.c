// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"
#include "HAPPlatform+Init.h"

typedef struct {
    HAPControllerPairingIdentifier pairingIdentifier;
    HAPControllerPublicKey publicKey;
    bool isAdmin;

    bool foundInExport;
} ControllerPairing;

int main() {
    HAPError err;
    HAPPlatformCreate();

    // Import Device ID.
    HAPAccessoryServerDeviceID expectedDeviceID;
    HAPPlatformRandomNumberFill(expectedDeviceID.bytes, sizeof expectedDeviceID.bytes);
    err = HAPLegacyImportDeviceID(platform.keyValueStore, &expectedDeviceID);
    HAPAssert(!err);

    // Import long-term secret key.
    HAPAccessoryServerLongTermSecretKey expectedLongTermSecretKey;
    HAPPlatformRandomNumberFill(expectedLongTermSecretKey.bytes, sizeof expectedLongTermSecretKey.bytes);
    err = HAPLegacyImportLongTermSecretKey(platform.keyValueStore, &expectedLongTermSecretKey);
    HAPAssert(!err);

    // Import pairings.
    ControllerPairing pairings[kHAPPairingStorage_MinElements];
    HAPRawBufferZero(pairings, sizeof pairings);
    for (HAPPlatformKeyValueStoreKey i = 0; i < HAPArrayCount(pairings); i++) {
        pairings[i].pairingIdentifier.numBytes = i % sizeof pairings[i].pairingIdentifier.bytes;
        HAPPlatformRandomNumberFill(pairings[i].pairingIdentifier.bytes, pairings[i].pairingIdentifier.numBytes);
        HAPPlatformRandomNumberFill(pairings[i].publicKey.bytes, sizeof pairings[i].publicKey.bytes);
        pairings[i].isAdmin = i % 2 != 0;
        err = HAPLegacyImportControllerPairing(
                platform.keyValueStore, i, &pairings[i].pairingIdentifier, &pairings[i].publicKey, pairings[i].isAdmin);
        HAPAssert(!err);
    }

    // Remove pairings.
    err = HAPRemoveAllPairings(platform.keyValueStore);
    HAPAssert(!err);

    // Restore factory settings.
    err = HAPRestoreFactorySettings(platform.keyValueStore);
    HAPAssert(!err);

    return 0;
}
