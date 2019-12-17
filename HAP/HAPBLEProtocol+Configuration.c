// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEProtocol" };

/**
 * Protocol configuration request types.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-32 Protocol Configuration Request Types
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEProtocolConfigurationRequestTLVType) {
    /** Generate-Broadcast-Encryption-Key. */
    kHAPBLEProtocolConfigurationRequestTLVType_GenerateBroadcastEncryptionKey = 0x01,

    /** Get-All-Params. */
    kHAPBLEProtocolConfigurationRequestTLVType_GetAllParams = 0x02,

    /** Set-Accessory-Advertising-Identifier. */
    kHAPBLEProtocolConfigurationRequestTLVType_SetAccessoryAdvertisingIdentifier = 0x03
} HAP_ENUM_END(uint8_t, HAPBLEProtocolConfigurationRequestTLVType);

HAP_RESULT_USE_CHECK
HAPError HAPBLEProtocolHandleConfigurationRequest(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        bool* didRequestGetAll,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(requestReader);
    HAPPrecondition(didRequestGetAll);
    HAPPrecondition(keyValueStore);

    HAPError err;

    HAPTLV generateKeyTLV, getAllTLV, setAdvertisingIDTLV;
    generateKeyTLV.type = kHAPBLEProtocolConfigurationRequestTLVType_GenerateBroadcastEncryptionKey;
    getAllTLV.type = kHAPBLEProtocolConfigurationRequestTLVType_GetAllParams;
    setAdvertisingIDTLV.type = kHAPBLEProtocolConfigurationRequestTLVType_SetAccessoryAdvertisingIdentifier;
    err = HAPTLVReaderGetAll(
            requestReader, (HAPTLV* const[]) { &generateKeyTLV, &getAllTLV, &setAdvertisingIDTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    // Generate-Broadcast-Encryption-Key.
    bool generateKey = false;
    if (generateKeyTLV.value.bytes) {
        if (generateKeyTLV.value.numBytes) {
            HAPLog(&logObject,
                   "Generate-Broadcast-Encryption-Key has invalid length (%lu).",
                   (unsigned long) generateKeyTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        generateKey = true;
    }

    // Get-All-Params.
    *didRequestGetAll = false;
    if (getAllTLV.value.bytes) {
        if (getAllTLV.value.numBytes) {
            HAPLog(&logObject, "Get-All-Params has invalid length (%lu).", (unsigned long) getAllTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        *didRequestGetAll = true;
    }

    // Set-Accessory-Advertising-Identifier.
    const HAPDeviceID* advertisingID = NULL;
    if (setAdvertisingIDTLV.value.bytes) {
        if (setAdvertisingIDTLV.value.numBytes != sizeof advertisingID->bytes) {
            HAPLog(&logObject,
                   "Set-Accessory-Advertising-Identifier has invalid length (%lu).",
                   (unsigned long) setAdvertisingIDTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        advertisingID = setAdvertisingIDTLV.value.bytes;
    }

    // Handle request.
    if (generateKey) {
        err = HAPBLEAccessoryServerBroadcastGenerateKey(session, advertisingID);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    } else if (advertisingID) {
        err = HAPBLEAccessoryServerBroadcastSetAdvertisingID(server->platform.keyValueStore, advertisingID);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    return kHAPError_None;
}

/**
 * Protocol configuration parameter types.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-34 Protocol Configuration Parameter Types
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEProtocolConfigurationResponseTLVType) {
    /** HAP-Param-Current-State-Number. */
    kHAPBLEProtocolConfigurationResponseTLVType_CurrentStateNumber = 0x01,

    /** HAP-Param-Current-Config-Number. */
    kHAPBLEProtocolConfigurationResponseTLVType_CurrentConfigNumber = 0x02,

    /** HAP-Param-Accessory-Advertising-Identifier. */
    kHAPBLEProtocolConfigurationResponseTLVType_AccessoryAdvertisingIdentifier = 0x03,

    /** HAP-Param-Broadcast-Encryption-Key. */
    kHAPBLEProtocolConfigurationResponseTLVType_BroadcastEncryptionKey = 0x04
} HAP_ENUM_END(uint8_t, HAPBLEProtocolConfigurationResponseTLVType);

HAP_RESULT_USE_CHECK
HAPError HAPBLEProtocolGetConfigurationResponse(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter,
        HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(responseWriter);
    HAPPrecondition(keyValueStore);

    HAPError err;

    // HAP-Param-Current-State-Number.
    HAPBLEAccessoryServerGSN gsn;
    err = HAPBLEAccessoryServerGetGSN(server->platform.keyValueStore, &gsn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    uint8_t gsnBytes[] = { HAPExpandLittleUInt16(gsn.gsn) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEProtocolConfigurationResponseTLVType_CurrentStateNumber,
                              .value = { .bytes = gsnBytes, .numBytes = sizeof gsnBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Current-Config-Number.
    uint16_t cn;
    err = HAPAccessoryServerGetCN(server->platform.keyValueStore, &cn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    uint8_t cnBytes[] = { (uint8_t)((cn - 1) % UINT8_MAX + 1) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEProtocolConfigurationResponseTLVType_CurrentConfigNumber,
                              .value = { .bytes = cnBytes, .numBytes = sizeof cnBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Accessory-Advertising-Identifier, HAP-Param-Broadcast-Encryption-Key.
    uint16_t keyExpirationGSN;
    HAPBLEAccessoryServerBroadcastEncryptionKey broadcastKey;
    HAPDeviceID advertisingID;
    err = HAPBLEAccessoryServerBroadcastGetParameters(
            server->platform.keyValueStore, &keyExpirationGSN, &broadcastKey, &advertisingID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEProtocolConfigurationResponseTLVType_AccessoryAdvertisingIdentifier,
                              .value = { .bytes = advertisingID.bytes, .numBytes = sizeof advertisingID.bytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }
    if (keyExpirationGSN) {
        err = HAPTLVWriterAppend(
                responseWriter,
                &(const HAPTLV) { .type = kHAPBLEProtocolConfigurationResponseTLVType_BroadcastEncryptionKey,
                                  .value = { .bytes = broadcastKey.value, .numBytes = sizeof broadcastKey.value } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    }

    return kHAPError_None;
}
