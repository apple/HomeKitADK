// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "LegacyImport" };

HAP_RESULT_USE_CHECK
HAPError
        HAPLegacyImportDeviceID(HAPPlatformKeyValueStoreRef keyValueStore, const HAPAccessoryServerDeviceID* deviceID) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(deviceID);

    HAPError err;

    // Check if key-value store has already been configured.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_DeviceID,
            NULL,
            0,
            NULL,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPPrecondition(!found);

    // Store Device ID.
    HAPLogBufferInfo(&logObject, deviceID->bytes, sizeof deviceID->bytes, "Importing legacy Device ID.");
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_DeviceID,
            deviceID->bytes,
            sizeof deviceID->bytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPLegacyImportConfigurationNumber(HAPPlatformKeyValueStoreRef keyValueStore, uint32_t configurationNumber) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(configurationNumber);

    HAPError err;

    // Check if key-value store has already been configured.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
            NULL,
            0,
            NULL,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPPrecondition(!found);

    // Store configuration number.
    HAPLogInfo(&logObject, "Importing legacy configuration number: %lu.", (unsigned long) configurationNumber);
    uint8_t cnBytes[] = { HAPExpandLittleUInt32(configurationNumber) };
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
            cnBytes,
            sizeof cnBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_STATIC_ASSERT(
        sizeof(HAPAccessoryServerLongTermSecretKey) == ED25519_SECRET_KEY_BYTES,
        HAPAccessoryServerLongTermSecretKey_MatchesCrypto);

HAP_RESULT_USE_CHECK
HAPError HAPLegacyImportLongTermSecretKey(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPAccessoryServerLongTermSecretKey* longTermSecretKey) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(longTermSecretKey);

    HAPError err;

    // Check if key-value store has already been configured.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_LTSK,
            NULL,
            0,
            NULL,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPPrecondition(!found);

    // Store LTSK.
    HAPLogSensitiveBufferInfo(
            &logObject,
            longTermSecretKey->bytes,
            sizeof longTermSecretKey->bytes,
            "Importing legacy long-term secret key.");
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_LTSK,
            longTermSecretKey->bytes,
            sizeof longTermSecretKey->bytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPLegacyImportUnsuccessfulAuthenticationAttemptsCounter(
        HAPPlatformKeyValueStoreRef keyValueStore,
        uint8_t numAuthAttempts) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(numAuthAttempts <= 100);

    HAPError err;

    // Check if key-value store has already been configured.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts,
            NULL,
            0,
            NULL,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPPrecondition(!found);

    // Store unsuccessful authentication attempts counter.
    HAPLogSensitiveInfo(
            &logObject, "Importing legacy unsuccessful authentication attempts counter: %u.", numAuthAttempts);
    uint8_t numAuthAttemptsBytes[] = { numAuthAttempts };
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts,
            numAuthAttemptsBytes,
            sizeof numAuthAttemptsBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_STATIC_ASSERT(sizeof(HAPControllerPublicKey) == ED25519_PUBLIC_KEY_BYTES, HAPControllerPublicKey_MatchesCrypto);

HAP_RESULT_USE_CHECK
HAPError HAPLegacyImportControllerPairing(
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreKey pairingIndex,
        const HAPControllerPairingIdentifier* pairingIdentifier,
        const HAPControllerPublicKey* publicKey,
        bool isAdmin) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(pairingIdentifier);
    HAPPrecondition(pairingIdentifier->numBytes <= sizeof pairingIdentifier->bytes);
    HAPPrecondition(pairingIdentifier->numBytes <= sizeof(HAPPairingID));
    HAPPrecondition(publicKey);

    HAPError err;

    // Check if key-value store has already been configured.
    bool found;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore, kHAPKeyValueStoreDomain_Pairings, pairingIndex, NULL, 0, NULL, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPPrecondition(!found);

    // Store pairing.
    HAPLogInfo(&logObject, "Importing legacy pairing (%s).", isAdmin ? "admin" : "regular");
    HAPLogSensitiveBufferInfo(&logObject, pairingIdentifier->bytes, pairingIdentifier->numBytes, "Pairing identifier.");
    HAPLogSensitiveBufferInfo(&logObject, publicKey->bytes, sizeof publicKey->bytes, "Public key.");
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    HAPRawBufferZero(pairingBytes, sizeof pairingBytes);
    HAPAssert(sizeof pairingIdentifier->bytes == 36);
    HAPRawBufferCopyBytes(&pairingBytes[0], pairingIdentifier->bytes, pairingIdentifier->numBytes);
    pairingBytes[36] = (uint8_t) pairingIdentifier->numBytes;
    HAPAssert(sizeof publicKey->bytes == 32);
    HAPRawBufferCopyBytes(&pairingBytes[37], publicKey->bytes, 32);
    pairingBytes[69] = isAdmin ? (uint8_t) 0x01 : (uint8_t) 0x00;

    err = HAPPlatformKeyValueStoreSet(
            keyValueStore, kHAPKeyValueStoreDomain_Pairings, pairingIndex, pairingBytes, sizeof pairingBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}
