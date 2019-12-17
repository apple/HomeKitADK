// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLECharacteristic" };

/**
 * Structure representing the Char Value TLV item.
 */
typedef struct {
    void* bytes;     /**< Start of value. */
    size_t numBytes; /**< Length of value. */
    size_t maxBytes; /**< Capacity of value, including free memory after the value. */
} HAPCharacteristicValueTLV;

/**
 * Parses the body of a a HAP-Characteristic-Write-Request.
 *
 * @param      characteristic_      Characteristic that received the request.
 * @param      requestReader        Reader to parse Characteristic value from. Reader content will become invalid.
 * @param[out] value                Char Value TLV item.
 * @param[out] remote               Whether the request appears to be sent remotely.
 * @param[out] authDataBytes        Additional authorization data.
 * @param[out] numAuthDataBytes     Length of additional authorization data.
 * @param[out] ttl                  TTL If a TTL TLV item was present; 0 Otherwise.
 * @param[out] hasReturnResponse    True If a Return-Response TLV item was present and set to 1; False Otherwise.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError ParseRequest(
        const HAPCharacteristic* characteristic_,
        HAPTLVReaderRef* requestReader,
        HAPCharacteristicValueTLV* value,
        bool* remote,
        const void** authDataBytes,
        size_t* numAuthDataBytes,
        uint8_t* ttl,
        bool* hasReturnResponse) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(requestReader);
    HAPPrecondition(value);
    HAPPrecondition(remote);
    HAPPrecondition(authDataBytes);
    HAPPrecondition(numAuthDataBytes);
    HAPPrecondition(ttl);
    HAPPrecondition(hasReturnResponse);

    HAPError err;

    HAPTLV valueTLV, authDataTLV, originTLV;
    valueTLV.type = kHAPBLEPDUTLVType_Value;
    authDataTLV.type = kHAPBLEPDUTLVType_AdditionalAuthorizationData;
    originTLV.type = kHAPBLEPDUTLVType_Origin;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.5.4 HAP Characteristic Timed Write Procedure
    HAPTLV ttlTLV;
    ttlTLV.type = kHAPBLEPDUTLVType_TTL;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.5.5 HAP Characteristic Write-With-Response Procedure
    HAPTLV returnResponseTLV;
    returnResponseTLV.type = kHAPBLEPDUTLVType_ReturnResponse;

    err = HAPTLVReaderGetAll(
            requestReader,
            (HAPTLV* const[]) { &valueTLV, &authDataTLV, &originTLV, &ttlTLV, &returnResponseTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    // HAP-Param-Value.
    if (!valueTLV.value.bytes) {
        HAPLog(&logObject, "HAP-Param-Value missing.");
        return kHAPError_InvalidData;
    }

    // HAP-Param-Origin.
    if (originTLV.value.bytes) {
        if (originTLV.value.numBytes != 1) {
            HAPLog(&logObject, "HAP-Param-Origin has invalid length (%lu).", (unsigned long) originTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        uint8_t origin = ((const uint8_t*) originTLV.value.bytes)[0];

        switch (origin) {
            case 0: {
                *remote = false;
            } break;
            case 1: {
                *remote = true;
            } break;
            default: {
                HAPLog(&logObject, "HAP-Param-Origin invalid: %u.", origin);
                return kHAPError_InvalidData;
            }
        }
    } else {
        *remote = false;
    }

    // HAP-Param-Additional-Authorization-Data, HAP-Param-Origin.
    *authDataBytes = NULL;
    *numAuthDataBytes = 0;
    if (characteristic->properties.supportsAuthorizationData) {
        if (authDataTLV.value.bytes) {
            if (!originTLV.value.bytes) {
                // When additional authorization data is present it is included
                // as an additional type to the TLV8 format along with the Value and Remote TLV types.
                // See HomeKit Accessory Protocol Specification R14
                // Section 7.4.5.2 Characteristic with Additional Authorization Data
                HAPLog(&logObject, "HAP-Param-Origin missing but HAP-Param-Additional-Authorization-Data is present.");
                return kHAPError_InvalidData;
            }

            *authDataBytes = authDataTLV.value.bytes;
            *numAuthDataBytes = authDataTLV.value.numBytes;
        }
    } else if (authDataTLV.value.bytes) {
        HAPLog(&logObject,
               "HAP-Param-Additional-Authorization-Data present but Additional Authorization is not supported.");
    }

    // HAP-Param-TTL.
    if (ttlTLV.value.bytes) {
        if (ttlTLV.value.numBytes != 1) {
            HAPLog(&logObject, "HAP-Param-TTL has invalid length (%lu).", (unsigned long) ttlTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        *ttl = ((const uint8_t*) ttlTLV.value.bytes)[0];
    } else {
        *ttl = 0;
    }

    // HAP-Param-Return-Response.
    if (returnResponseTLV.value.bytes) {
        if (returnResponseTLV.value.numBytes != 1) {
            HAPLog(&logObject,
                   "HAP-Param-Return-Response has invalid length (%lu).",
                   (unsigned long) returnResponseTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
        uint8_t returnResponse = ((const uint8_t*) returnResponseTLV.value.bytes)[0];
        if (returnResponse != 1) {
            HAPLog(&logObject, "HAP-Param-Return-Response invalid: %u.", returnResponse);
            return kHAPError_InvalidData;
        }
        *hasReturnResponse = true;
    } else {
        *hasReturnResponse = false;
    }

    // Optimize memory. We want as much free space as possible after the value.
    uint8_t* bytes = ((HAPTLVReader*) requestReader)->bytes;
    size_t maxBytes = ((HAPTLVReader*) requestReader)->maxBytes;

    // TLV values are always NULL terminated to simplify string handling. This property should be retained.
    // The NULL terminator is not counted in the TLV value's numBytes.
    // Case 1: [  AAD  |  VAL  | empty ]
    // Case 2: [  VAL  | empty |  AAD  ]
    // Case 3: [  VAL  |     empty     ]
    //         AAD and VAL fields contain an additional NULL byte.
    HAPRawBufferZero(value, sizeof *value);
    if (*authDataBytes) {
        size_t numValueBytes = valueTLV.value.numBytes;
        size_t numValueBytesWithNull = numValueBytes + 1;
        size_t numAuthDataBytesWithNull = *numAuthDataBytes + 1;

        void* valueStart;
        void* authDataStart;
        if (*authDataBytes < valueTLV.value.bytes) {
            // Case 1.
            authDataStart = &bytes[0];
            valueStart = &bytes[numAuthDataBytesWithNull];
        } else {
            // Case 2.
            authDataStart = &bytes[maxBytes - numAuthDataBytesWithNull];
            valueStart = &bytes[0];
        }

        // Move AAD.
        HAPRawBufferCopyBytes(authDataStart, *authDataBytes, numAuthDataBytesWithNull);
        *authDataBytes = authDataStart;

        // Move VAL.
        HAPRawBufferCopyBytes(valueStart, HAPNonnullVoid(valueTLV.value.bytes), numValueBytesWithNull);
        value->bytes = valueStart;
        value->numBytes = numValueBytes;
        value->maxBytes = maxBytes - numAuthDataBytesWithNull;
    } else {
        size_t numValueBytes = valueTLV.value.numBytes;
        size_t numValueBytesWithNULL = numValueBytes + 1;

        // Case 3.
        void* valueStart = &bytes[0];

        // Move VAL.
        HAPRawBufferCopyBytes(valueStart, HAPNonnullVoid(valueTLV.value.bytes), numValueBytesWithNULL);
        value->bytes = valueStart;
        value->numBytes = numValueBytes;
        value->maxBytes = maxBytes;
    }

    HAPAssert(((const uint8_t*) value->bytes)[value->numBytes] == '\0');
    if (*authDataBytes) {
        HAPAssert(((const uint8_t*) *authDataBytes)[*numAuthDataBytes] == '\0');
    }
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicParseAndWriteValue(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader,
        const HAPTime* _Nullable timedWriteStartTime,
        bool* hasExpired,
        bool* hasReturnResponse) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(requestReader);
    HAPPrecondition(hasExpired);
    HAPPrecondition(hasReturnResponse);

    HAPError err;

    *hasExpired = false;
    *hasReturnResponse = false;

    HAPCharacteristicValueTLV value = { .bytes = NULL, .numBytes = 0, .maxBytes = 0 };
    bool remote = false;
    const void* authDataBytes = NULL;
    size_t numAuthDataBytes = 0;
    uint8_t ttl = 0;
    err = ParseRequest(
            characteristic, requestReader, &value, &remote, &authDataBytes, &numAuthDataBytes, &ttl, hasReturnResponse);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    // Handle Timed Write.
    if (timedWriteStartTime) {
        if (!ttl) {
            HAPLog(&logObject, "Timed Write Request did not include valid TTL.");
            return kHAPError_InvalidData;
        }

        HAPTime now = HAPPlatformClockGetCurrent();
        *hasExpired = now >= ttl * 100 * HAPMillisecond && now - ttl * 100 * HAPMillisecond > *timedWriteStartTime;
        if (*hasExpired) {
            return kHAPError_None;
        }
    }

    // The maximum length of an HAP characteristic value shall be 64000 bytes.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.1.7 Maximum Payload Size
    if (value.numBytes > 64000) {
        HAPLog(&logObject, "Value exceeds maximum allowed length of 64000 bytes.");
        return kHAPError_InvalidData;
    }

    // Parse value and handle write.
    uint8_t* bytes = value.bytes;
    size_t numBytes = value.numBytes;
    switch (*((const HAPCharacteristicFormat*) characteristic)) {
        case kHAPCharacteristicFormat_Data: {
            err = HAPDataCharacteristicHandleWrite(
                    server_,
                    &(const HAPDataCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    bytes,
                    numBytes,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Bool: {
            if (numBytes != sizeof(bool)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            if (bytes[0] != 0 && bytes[0] != 1) {
                HAPLogCharacteristic(
                        &logObject, characteristic, service, accessory, "Unexpected bool value: %u.", bytes[0]);
                return kHAPError_InvalidData;
            }
            err = HAPBoolCharacteristicHandleWrite(
                    server_,
                    &(const HAPBoolCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    bytes[0] != 0,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt8: {
            if (numBytes != sizeof(uint8_t)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            err = HAPUInt8CharacteristicHandleWrite(
                    server_,
                    &(const HAPUInt8CharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    bytes[0],
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt16: {
            if (numBytes != sizeof(uint16_t)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            err = HAPUInt16CharacteristicHandleWrite(
                    server_,
                    &(const HAPUInt16CharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    HAPReadLittleUInt16(bytes),
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt32: {
            if (numBytes != sizeof(uint32_t)) {
                HAPLogCharacteristic(&logObject, characteristic, service, accessory, "Unexpected value length.");
                return kHAPError_InvalidData;
            }
            err = HAPUInt32CharacteristicHandleWrite(
                    server_,
                    &(const HAPUInt32CharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    HAPReadLittleUInt32(bytes),
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt64: {
            if (numBytes != sizeof(uint64_t)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            err = HAPUInt64CharacteristicHandleWrite(
                    server_,
                    &(const HAPUInt64CharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    HAPReadLittleUInt64(bytes),
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Int: {
            if (numBytes != sizeof(int32_t)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            err = HAPIntCharacteristicHandleWrite(
                    server_,
                    &(const HAPIntCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    HAPReadLittleInt32(bytes),
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Float: {
            if (numBytes != sizeof(float)) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Unexpected value length: %lu.",
                        (unsigned long) numBytes);
                return kHAPError_InvalidData;
            }
            uint32_t bitPattern = HAPReadLittleUInt32(bytes);
            err = HAPFloatCharacteristicHandleWrite(
                    server_,
                    &(const HAPFloatCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    HAPFloatFromBitPattern(bitPattern),
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_String: {
            if (numBytes != HAPStringGetNumBytes(value.bytes)) {
                HAPLogSensitiveCharacteristicBuffer(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        bytes,
                        numBytes,
                        "Unexpected string value (contains NULL bytes).");
                return kHAPError_InvalidData;
            }
            if (!HAPUTF8IsValidData(bytes, numBytes)) {
                HAPLogSensitiveCharacteristicBuffer(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        bytes,
                        numBytes,
                        "Unexpected string value (invalid UTF-8 encoding).");
                return kHAPError_InvalidData;
            }
            err = HAPStringCharacteristicHandleWrite(
                    server_,
                    &(const HAPStringCharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    (const char*) bytes,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_TLV8: {
            HAPTLVReaderRef reader;
            HAPTLVReaderCreateWithOptions(
                    &reader,
                    &(const HAPTLVReaderOptions) { .bytes = bytes, .numBytes = numBytes, .maxBytes = value.maxBytes });

            err = HAPTLV8CharacteristicHandleWrite(
                    server_,
                    &(const HAPTLV8CharacteristicWriteRequest) {
                            .transportType = kHAPTransportType_BLE,
                            .session = session,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                            .remote = remote,
                            .authorizationData = { .bytes = authDataBytes, .numBytes = numAuthDataBytes } },
                    &reader,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                        err == kHAPError_OutOfResources || err == kHAPError_NotAuthorized || err == kHAPError_Busy);
                return err;
            }
        }
            return kHAPError_None;
    }
    HAPFatalError();
}
