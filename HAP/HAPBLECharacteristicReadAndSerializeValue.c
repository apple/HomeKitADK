// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLECharacteristic" };

/**
 * Serializes Char Value field of HAP-Characteristic-Read-Response.
 *
 * @param      valueBytes           Characteristic value.
 * @param      numValueBytes        Length of characteristic value.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.7 HAP-Characteristic-Read-Response
 */
HAP_RESULT_USE_CHECK
static HAPError SerializeCharValue(const void* valueBytes, size_t numValueBytes, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(valueBytes);
    HAPPrecondition(responseWriter);

    // The maximum length of an HAP characteristic value shall be 64000 bytes.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.1.7 Maximum Payload Size
    HAPPrecondition(numValueBytes <= 64000);

    HAPError err;

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_Value,
                              .value = { .bytes = valueBytes, .numBytes = numValueBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPBLECharacteristicReadAndSerializeValue(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(responseWriter);

    HAPError err;

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

    // Fetch characteristic value.
    size_t numBytes;
    switch (*((const HAPCharacteristicFormat*) characteristic)) {
        case kHAPCharacteristicFormat_Data: {
            // The maximum length of an HAP characteristic value shall be 64000 bytes.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.1.7 Maximum Payload Size
            err = HAPDataCharacteristicHandleRead(
                    server_,
                    &(const HAPDataCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                .session = session,
                                                                .characteristic = characteristic,
                                                                .service = service,
                                                                .accessory = accessory },
                    bytes,
                    maxBytes <= 64000 ? maxBytes : 64000,
                    &numBytes,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
        }
            goto serialized;
        case kHAPCharacteristicFormat_Bool: {
            if (maxBytes < sizeof(bool)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "Bool");
                return kHAPError_OutOfResources;
            }
            bool characteristicValue;
            err = HAPBoolCharacteristicHandleRead(
                    server_,
                    &(const HAPBoolCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                .session = session,
                                                                .characteristic = characteristic,
                                                                .service = service,
                                                                .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            b[0] = (uint8_t)(characteristicValue ? 1 : 0);
            numBytes = sizeof(bool);
        }
            goto serialized;
        case kHAPCharacteristicFormat_UInt8: {
            if (maxBytes < sizeof(uint8_t)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "UInt8");
                return kHAPError_OutOfResources;
            }
            uint8_t characteristicValue;
            err = HAPUInt8CharacteristicHandleRead(
                    server_,
                    &(const HAPUInt8CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                 .session = session,
                                                                 .characteristic = characteristic,
                                                                 .service = service,
                                                                 .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            b[0] = characteristicValue;
            numBytes = sizeof(uint8_t);
        }
            goto serialized;
        case kHAPCharacteristicFormat_UInt16: {
            if (maxBytes < sizeof(uint16_t)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "UInt16");
                return kHAPError_OutOfResources;
            }
            uint16_t characteristicValue;
            err = HAPUInt16CharacteristicHandleRead(
                    server_,
                    &(const HAPUInt16CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                  .session = session,
                                                                  .characteristic = characteristic,
                                                                  .service = service,
                                                                  .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            HAPWriteLittleUInt16(b, characteristicValue);
            numBytes = sizeof(uint16_t);
        }
            goto serialized;
        case kHAPCharacteristicFormat_UInt32: {
            if (maxBytes < sizeof(uint32_t)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "UInt32");
                return kHAPError_OutOfResources;
            }
            uint32_t characteristicValue;
            err = HAPUInt32CharacteristicHandleRead(
                    server_,
                    &(const HAPUInt32CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                  .session = session,
                                                                  .characteristic = characteristic,
                                                                  .service = service,
                                                                  .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            HAPWriteLittleUInt32(b, characteristicValue);
            numBytes = sizeof(uint32_t);
        }
            goto serialized;
        case kHAPCharacteristicFormat_UInt64: {
            if (maxBytes < sizeof(uint64_t)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "UInt64");
                return kHAPError_OutOfResources;
            }
            uint64_t characteristicValue;
            err = HAPUInt64CharacteristicHandleRead(
                    server_,
                    &(const HAPUInt64CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                  .session = session,
                                                                  .characteristic = characteristic,
                                                                  .service = service,
                                                                  .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            HAPWriteLittleUInt64(b, characteristicValue);
            numBytes = sizeof(uint64_t);
        }
            goto serialized;
        case kHAPCharacteristicFormat_Int: {
            if (maxBytes < sizeof(int32_t)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "Int32");
                return kHAPError_OutOfResources;
            }
            int32_t characteristicValue;
            err = HAPIntCharacteristicHandleRead(
                    server_,
                    &(const HAPIntCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                               .session = session,
                                                               .characteristic = characteristic,
                                                               .service = service,
                                                               .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint8_t* b = bytes;
            HAPWriteLittleInt32(b, characteristicValue);
            numBytes = sizeof(int32_t);
        }
            goto serialized;
        case kHAPCharacteristicFormat_Float: {
            if (maxBytes < sizeof(float)) {
                HAPLog(&logObject, "Not enough space to read %s value.", "Float");
                return kHAPError_OutOfResources;
            }
            float characteristicValue;
            err = HAPFloatCharacteristicHandleRead(
                    server_,
                    &(const HAPFloatCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                 .session = session,
                                                                 .characteristic = characteristic,
                                                                 .service = service,
                                                                 .accessory = accessory },
                    &characteristicValue,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            uint32_t bitPattern = HAPFloatGetBitPattern(characteristicValue);
            uint8_t* b = bytes;
            HAPWriteLittleUInt32(b, bitPattern);
            numBytes = sizeof(float);
        }
            goto serialized;
        case kHAPCharacteristicFormat_String: {
            // The maximum length of an HAP characteristic value shall be 64000 bytes.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.1.7 Maximum Payload Size
            err = HAPStringCharacteristicHandleRead(
                    server_,
                    &(const HAPStringCharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                  .session = session,
                                                                  .characteristic = characteristic,
                                                                  .service = service,
                                                                  .accessory = accessory },
                    bytes,
                    maxBytes <= 64000 ? maxBytes : 64000,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }
            numBytes = HAPStringGetNumBytes(bytes);
        }
            goto serialized;
        case kHAPCharacteristicFormat_TLV8: {
            // The maximum length of an HAP characteristic value shall be 64000 bytes.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.4.1.7 Maximum Payload Size
            HAPTLVWriterRef writer;
            HAPTLVWriterCreate(&writer, bytes, maxBytes <= 64000 ? maxBytes : 64000);

            err = HAPTLV8CharacteristicHandleRead(
                    server_,
                    &(const HAPTLV8CharacteristicReadRequest) { .transportType = kHAPTransportType_BLE,
                                                                .session = session,
                                                                .characteristic =
                                                                        (const HAPTLV8Characteristic*) characteristic,
                                                                .service = service,
                                                                .accessory = accessory },
                    &writer,
                    server->context);
            if (err) {
                HAPAssert(
                        err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources ||
                        err == kHAPError_Busy);
                return err;
            }

            void* tlvBytes;
            HAPTLVWriterGetBuffer(&writer, &tlvBytes, &numBytes);
            HAPAssert(tlvBytes == bytes);
        }
            goto serialized;
    }
    HAPFatalError();
serialized:

    HAPAssert(numBytes <= maxBytes);
    err = SerializeCharValue(bytes, numBytes, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}
