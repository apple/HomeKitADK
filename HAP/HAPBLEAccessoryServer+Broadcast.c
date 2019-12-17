// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEAccessoryServer" };

typedef struct {
    uint16_t keyExpirationGSN;
    HAPBLEAccessoryServerBroadcastEncryptionKey key;
    bool hasAdvertisingID;
    HAPDeviceID advertisingID;
} HAPBLEAccessoryServerBroadcastParameters;

#define HAP_BLE_ACCESSORY_SERVER_GET_BROADCAST_PARAMETERS_OR_RETURN_ERROR(keyValueStore, parameters) \
    do { \
        bool found; \
        size_t numBytes; \
        uint8_t parametersBytes \
                [sizeof(uint16_t) + sizeof(HAPBLEAccessoryServerBroadcastEncryptionKey) + sizeof(uint8_t) + \
                 sizeof(HAPDeviceID)]; \
        err = HAPPlatformKeyValueStoreGet( \
                (keyValueStore), \
                kHAPKeyValueStoreDomain_Configuration, \
                kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters, \
                parametersBytes, \
                sizeof parametersBytes, \
                &numBytes, \
                &found); \
        if (err) { \
            HAPAssert(err == kHAPError_Unknown); \
            return err; \
        } \
        if (!found) { \
            HAPRawBufferZero(parametersBytes, sizeof parametersBytes); \
        } else if (numBytes != sizeof parametersBytes) { \
            HAPLog(&logObject, "Invalid BLE broadcast state length: %lu.", (unsigned long) numBytes); \
            return kHAPError_Unknown; \
        } \
        HAPRawBufferZero((parameters), sizeof *(parameters)); \
        (parameters)->keyExpirationGSN = HAPReadLittleUInt16(&parametersBytes[0]); \
        HAPAssert(sizeof(parameters)->key.value == 32); \
        HAPRawBufferCopyBytes((parameters)->key.value, &parametersBytes[2], 32); \
        (parameters)->hasAdvertisingID = (uint8_t)(parametersBytes[34] & 0x01U) == 0x01; \
        HAPAssert(sizeof(parameters)->advertisingID.bytes == 6); \
        HAPRawBufferCopyBytes((parameters)->advertisingID.bytes, &parametersBytes[35], 6); \
    } while (0)

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastGetParameters(
        HAPPlatformKeyValueStoreRef keyValueStore,
        uint16_t* keyExpirationGSN,
        HAPBLEAccessoryServerBroadcastEncryptionKey* _Nullable broadcastKey,
        HAPDeviceID* _Nullable advertisingID) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(keyExpirationGSN);

    HAPError err;

    // Get parameters.
    HAPBLEAccessoryServerBroadcastParameters parameters;
    HAP_BLE_ACCESSORY_SERVER_GET_BROADCAST_PARAMETERS_OR_RETURN_ERROR(keyValueStore, &parameters);

    // Copy result.
    *keyExpirationGSN = parameters.keyExpirationGSN;
    if (parameters.keyExpirationGSN) {
        if (broadcastKey) {
            HAPRawBufferCopyBytes(HAPNonnull(broadcastKey), &parameters.key, sizeof *broadcastKey);
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    parameters.key.value,
                    sizeof parameters.key.value,
                    "BLE Broadcast Encryption Key (Expires after GSN %u).",
                    parameters.keyExpirationGSN);
        }
    }
    if (advertisingID) {
        if (parameters.hasAdvertisingID) {
            HAPRawBufferCopyBytes(HAPNonnull(advertisingID), &parameters.advertisingID, sizeof *advertisingID);
        } else {
            // Fallback to Device ID.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.2.2.2 Manufacturer Data
            err = HAPDeviceIDGet(keyValueStore, HAPNonnull(advertisingID));
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError
        HAPBLEAccessoryServerBroadcastGenerateKey(HAPSessionRef* session_, const HAPDeviceID* _Nullable advertisingID) {
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;
    HAPPrecondition(HAPSessionIsSecured(session_));

    HAPError err;

    // Get state.
    HAPBLEAccessoryServerBroadcastParameters parameters;
    HAP_BLE_ACCESSORY_SERVER_GET_BROADCAST_PARAMETERS_OR_RETURN_ERROR(server->platform.keyValueStore, &parameters);

    // Get GSN.
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // The broadcast encryption key shall expire and automatically and must be discarded by the
    // accessory after 32,767 (2^15 - 1) increments in GSN after the current broadcast key was
    // generated. The controller will normally refresh the broadcast key to ensure that they key does
    // not expire automatically on the accessory.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.7.4 Broadcast Encryption Key expiration and refresh
    uint32_t keyExpirationGSN = (uint32_t)(gsn.gsn + 32767 - 1);
    if (keyExpirationGSN > UINT16_MAX) {
        keyExpirationGSN -= UINT16_MAX;
    }
    HAPAssert(keyExpirationGSN != 0 && keyExpirationGSN <= UINT16_MAX);
    parameters.keyExpirationGSN = (uint16_t) keyExpirationGSN;

    // Fetch controller's Ed25519 long term public key.
    HAPAssert(session->hap.pairingID >= 0);
    bool found;
    size_t numBytes;
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Pairings,
            (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
            pairingBytes,
            sizeof pairingBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPAssert(found);
    HAPAssert(numBytes == sizeof pairingBytes);
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPAssert(sizeof pairing.identifier.bytes == 36);
    HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
    pairing.numIdentifierBytes = pairingBytes[36];
    HAPAssert(sizeof pairing.publicKey.value == 32);
    HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
    pairing.permissions = pairingBytes[69];
    {
        // Generate encryption key.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.7.3 Broadcast Encryption Key Generation
        static const uint8_t info[] = "Broadcast-Encryption-Key";
        HAP_hkdf_sha512(
                parameters.key.value,
                sizeof parameters.key.value,
                session->hap.cv_KEY,
                sizeof session->hap.cv_KEY,
                pairing.publicKey.value,
                sizeof pairing.publicKey.value,
                info,
                sizeof info - 1);
        HAPLogSensitiveBufferDebug(
                &logObject, session->hap.cv_KEY, sizeof session->hap.cv_KEY, "Curve25519 shared secret.");
        HAPLogSensitiveBufferDebug(
                &logObject, pairing.publicKey.value, sizeof pairing.publicKey.value, "Controller LTPK.");
        HAPLogSensitiveBufferDebug(
                &logObject, parameters.key.value, sizeof parameters.key.value, "BLE Broadcast Encryption Key.");
    }
    HAPRawBufferZero(&pairing, sizeof pairing);

    // Copy advertising identifier.
    if (advertisingID) {
        parameters.hasAdvertisingID = true;
        HAPAssert(sizeof parameters.advertisingID == sizeof *advertisingID);
        HAPRawBufferCopyBytes(&parameters.advertisingID, HAPNonnull(advertisingID), sizeof parameters.advertisingID);
    }

    // Save.
    uint8_t parametersBytes
            [sizeof(uint16_t) + sizeof(HAPBLEAccessoryServerBroadcastEncryptionKey) + sizeof(uint8_t) +
             sizeof(HAPDeviceID)];
    HAPWriteLittleUInt16(&parametersBytes[0], parameters.keyExpirationGSN);
    HAPAssert(sizeof parameters.key.value == 32);
    HAPRawBufferCopyBytes(&parametersBytes[2], parameters.key.value, 32);
    parametersBytes[34] = parameters.hasAdvertisingID ? (uint8_t) 0x01 : (uint8_t) 0x00;
    HAPAssert(sizeof parameters.advertisingID.bytes == 6);
    HAPRawBufferCopyBytes(&parametersBytes[35], parameters.advertisingID.bytes, 6);
    err = HAPPlatformKeyValueStoreSet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters,
            parametersBytes,
            sizeof parametersBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastSetAdvertisingID(
        HAPPlatformKeyValueStoreRef keyValueStore,
        const HAPDeviceID* advertisingID) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(advertisingID);

    HAPError err;

    // Get state.
    HAPBLEAccessoryServerBroadcastParameters parameters;
    HAP_BLE_ACCESSORY_SERVER_GET_BROADCAST_PARAMETERS_OR_RETURN_ERROR(keyValueStore, &parameters);

    // Copy advertising identifier.
    parameters.hasAdvertisingID = true;
    HAPAssert(sizeof parameters.advertisingID == sizeof *advertisingID);
    HAPRawBufferCopyBytes(&parameters.advertisingID, advertisingID, sizeof parameters.advertisingID);

    // Save.
    uint8_t parametersBytes
            [sizeof(uint16_t) + sizeof(HAPBLEAccessoryServerBroadcastEncryptionKey) + sizeof(uint8_t) +
             sizeof(HAPDeviceID)];
    HAPWriteLittleUInt16(&parametersBytes[0], parameters.keyExpirationGSN);
    HAPAssert(sizeof parameters.key.value == 32);
    HAPRawBufferCopyBytes(&parametersBytes[2], parameters.key.value, 32);
    parametersBytes[34] = parameters.hasAdvertisingID ? (uint8_t) 0x01 : (uint8_t) 0x00;
    HAPAssert(sizeof parameters.advertisingID.bytes == 6);
    HAPRawBufferCopyBytes(&parametersBytes[35], parameters.advertisingID.bytes, 6);
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters,
            parametersBytes,
            sizeof parametersBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEAccessoryServerBroadcastExpireKey(HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPLogInfo(&logObject, "Expiring broadcast encryption key.");

    // Get state.
    HAPBLEAccessoryServerBroadcastParameters parameters;
    HAP_BLE_ACCESSORY_SERVER_GET_BROADCAST_PARAMETERS_OR_RETURN_ERROR(keyValueStore, &parameters);

    // Expire encryption key.
    parameters.keyExpirationGSN = 0;
    HAPRawBufferZero(&parameters.key, sizeof parameters.key);

    // Save.
    uint8_t parametersBytes
            [sizeof(uint16_t) + sizeof(HAPBLEAccessoryServerBroadcastEncryptionKey) + sizeof(uint8_t) +
             sizeof(HAPDeviceID)];
    HAPWriteLittleUInt16(&parametersBytes[0], parameters.keyExpirationGSN);
    HAPAssert(sizeof parameters.key.value == 32);
    HAPRawBufferCopyBytes(&parametersBytes[2], parameters.key.value, 32);
    parametersBytes[34] = parameters.hasAdvertisingID ? (uint8_t) 0x01 : (uint8_t) 0x00;
    HAPAssert(sizeof parameters.advertisingID.bytes == 6);
    HAPRawBufferCopyBytes(&parametersBytes[35], parameters.advertisingID.bytes, 6);
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters,
            parametersBytes,
            sizeof parametersBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}
