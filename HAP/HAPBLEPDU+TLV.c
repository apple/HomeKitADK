// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEPDU" };

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeCharacteristicType(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_CharacteristicType,
                              .value = { .bytes = characteristic->characteristicType->bytes,
                                         .numBytes = sizeof characteristic->characteristicType->bytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeServiceType(const HAPService* service, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(service);
    HAPPrecondition(responseWriter);

    HAPError err;

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_ServiceType,
                              .value = { .bytes = service->serviceType->bytes,
                                         .numBytes = sizeof service->serviceType->bytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeServiceInstanceID(const HAPService* service, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(service);
    HAPPrecondition(responseWriter);

    HAPError err;

    HAPAssert(service->iid <= UINT16_MAX);
    uint8_t sidBytes[] = { HAPExpandLittleUInt16(service->iid) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_ServiceInstanceID,
                              .value = { .bytes = sidBytes, .numBytes = sizeof sidBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPCharacteristicPropertiesDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    uint8_t propertiesBytes[] = { HAPExpandLittleUInt16(
            (characteristic->properties.ble.readableWithoutSecurity ? 0x0001 : 0) |
            (characteristic->properties.ble.writableWithoutSecurity ? 0x0002 : 0) |
            (characteristic->properties.supportsAuthorizationData ? 0x0004 : 0) |
            (characteristic->properties.requiresTimedWrite ? 0x0008 : 0) |
            (characteristic->properties.readable ? 0x0010 : 0) | (characteristic->properties.writable ? 0x0020 : 0) |
            (characteristic->properties.hidden ? 0x0040 : 0) |
            (characteristic->properties.supportsEventNotification ? 0x0080 : 0) |
            (characteristic->properties.ble.supportsDisconnectedNotification ? 0x0100 : 0) |
            (characteristic->properties.ble.supportsBroadcastNotification ? 0x0200 : 0)) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPCharacteristicPropertiesDescriptor,
                              .value = { .bytes = propertiesBytes, .numBytes = sizeof propertiesBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeGATTUserDescriptionDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    if (!characteristic->manufacturerDescription) {
        return kHAPError_None;
    }

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTUserDescriptionDescriptor,
                              .value = { .bytes = characteristic->manufacturerDescription,
                                         .numBytes = HAPStringGetNumBytes(
                                                 HAPNonnull(characteristic->manufacturerDescription)) } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * Converts a HAP format to the corresponding BT SIG format.
 *
 * @param      hapFormat            HAP format to convert.
 *
 * @return BT SIG format.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-51 HAP Format to BT SIG Format mapping
 */
HAP_RESULT_USE_CHECK
static uint8_t ConvertHAPFormatToBTSIGFormat(HAPCharacteristicFormat hapFormat) {
    switch (hapFormat) {
        case kHAPCharacteristicFormat_Data:
            return 0x1B;
        case kHAPCharacteristicFormat_Bool:
            return 0x01;
        case kHAPCharacteristicFormat_UInt8:
            return 0x04;
        case kHAPCharacteristicFormat_UInt16:
            return 0x06;
        case kHAPCharacteristicFormat_UInt32:
            return 0x08;
        case kHAPCharacteristicFormat_UInt64:
            return 0x0A;
        case kHAPCharacteristicFormat_Int:
            return 0x10;
        case kHAPCharacteristicFormat_Float:
            return 0x14;
        case kHAPCharacteristicFormat_String:
            return 0x19;
        case kHAPCharacteristicFormat_TLV8:
            return 0x1B;
    }
    HAPFatalError();
}

/**
 * Converts a HAP unit to the corresponding BT SIG unit.
 *
 * @param      hapUnit              HAP unit to convert.
 *
 * @return BT SIG unit.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-52 HAP Unit to BT SIG Unit mapping
 */
HAP_RESULT_USE_CHECK
static uint16_t ConvertHAPUnitToBTSIGUnit(HAPCharacteristicUnits hapUnit) {
    switch (hapUnit) {
        case kHAPCharacteristicUnits_Celsius:
            return 0x272F;
        case kHAPCharacteristicUnits_ArcDegrees:
            return 0x2763;
        case kHAPCharacteristicUnits_Percentage:
            return 0x27AD;
        case kHAPCharacteristicUnits_None:
            return 0x2700;
        case kHAPCharacteristicUnits_Lux:
            return 0x2731;
        case kHAPCharacteristicUnits_Seconds:
            return 0x2703;
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeGATTPresentationFormatDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    uint8_t btSIGFormat = ConvertHAPFormatToBTSIGFormat(characteristic->format);

    uint16_t btSIGUnit = 0;
    switch (characteristic->format) {
        case kHAPCharacteristicFormat_UInt8: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPUInt8Characteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_UInt16: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPUInt16Characteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_UInt32: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPUInt32Characteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_UInt64: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPUInt64Characteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_Int: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPIntCharacteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_Float: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(((const HAPFloatCharacteristic*) characteristic)->units);
        }
            goto unit_converted;
        case kHAPCharacteristicFormat_Bool:
        case kHAPCharacteristicFormat_String:
        case kHAPCharacteristicFormat_TLV8:
        case kHAPCharacteristicFormat_Data: {
            btSIGUnit = ConvertHAPUnitToBTSIGUnit(kHAPCharacteristicUnits_None);
        }
            goto unit_converted;
    }
    HAPFatalError();
unit_converted:;

    uint8_t formatBytes[] = { /* Format (8bit): */ btSIGFormat,
                              /* Exponent (sint8): */ 0,
                              /* Unit (uint16): */ HAPExpandLittleUInt16(btSIGUnit),
                              /* Namespace (8bit): */ 1,
                              /* Description (16bit): */ HAPExpandLittleUInt16(0) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTPresentationFormatDescriptor,
                              .value = { .bytes = formatBytes, .numBytes = sizeof formatBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError
        HAPBLEPDUTLVSerializeGATTValidRange(const HAPCharacteristic* characteristic_, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    HAPPrecondition(responseWriter);

    HAPError err;

    switch (*((const HAPCharacteristicFormat*) characteristic_)) {
        case kHAPCharacteristicFormat_Data: {
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Bool: {
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt8: {
            const HAPUInt8Characteristic* characteristic = characteristic_;
            uint8_t minimumValue = characteristic->constraints.minimumValue;
            uint8_t maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(minimumValue <= maximumValue);

            if (!minimumValue && maximumValue == UINT8_MAX) {
                return kHAPError_None;
            }
            uint8_t rangeBytes[] = { minimumValue, maximumValue };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt16: {
            const HAPUInt16Characteristic* characteristic = characteristic_;
            uint16_t minimumValue = characteristic->constraints.minimumValue;
            uint16_t maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(minimumValue <= maximumValue);

            if (!minimumValue && maximumValue == UINT16_MAX) {
                return kHAPError_None;
            }

            uint8_t rangeBytes[] = { HAPExpandLittleUInt16(minimumValue), HAPExpandLittleUInt16(maximumValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt32: {
            const HAPUInt32Characteristic* characteristic = characteristic_;
            uint32_t minimumValue = characteristic->constraints.minimumValue;
            uint32_t maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(minimumValue <= maximumValue);

            if (!minimumValue && maximumValue == UINT32_MAX) {
                return kHAPError_None;
            }

            uint8_t rangeBytes[] = { HAPExpandLittleUInt32(minimumValue), HAPExpandLittleUInt32(maximumValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt64: {
            const HAPUInt64Characteristic* characteristic = characteristic_;
            uint64_t minimumValue = characteristic->constraints.minimumValue;
            uint64_t maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(minimumValue <= maximumValue);

            if (!minimumValue && maximumValue == UINT64_MAX) {
                return kHAPError_None;
            }

            uint8_t rangeBytes[] = { HAPExpandLittleUInt64(minimumValue), HAPExpandLittleUInt64(maximumValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Int: {
            const HAPIntCharacteristic* characteristic = characteristic_;
            int32_t minimumValue = characteristic->constraints.minimumValue;
            int32_t maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(minimumValue <= maximumValue);

            if (minimumValue == INT32_MIN && maximumValue == INT32_MAX) {
                return kHAPError_None;
            }

            uint8_t rangeBytes[] = { HAPExpandLittleInt32(minimumValue), HAPExpandLittleInt32(maximumValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Float: {
            const HAPFloatCharacteristic* characteristic = characteristic_;
            float minimumValue = characteristic->constraints.minimumValue;
            float maximumValue = characteristic->constraints.maximumValue;
            HAPPrecondition(HAPFloatIsFinite(minimumValue) || HAPFloatIsInfinite(minimumValue));
            HAPPrecondition(HAPFloatIsFinite(maximumValue) || HAPFloatIsInfinite(maximumValue));
            HAPPrecondition(minimumValue <= maximumValue);

            if (HAPFloatIsInfinite(minimumValue) && minimumValue < 0 && HAPFloatIsInfinite(maximumValue) &&
                maximumValue > 0) {
                return kHAPError_None;
            }

            uint32_t minimumBitPattern = HAPFloatGetBitPattern(minimumValue);
            uint32_t maximumBitPattern = HAPFloatGetBitPattern(maximumValue);
            uint8_t rangeBytes[] = { HAPExpandLittleUInt32(minimumBitPattern),
                                     HAPExpandLittleUInt32(maximumBitPattern) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_String: {
            const HAPStringCharacteristic* characteristic = characteristic_;
            uint16_t maxLength = characteristic->constraints.maxLength;

            // See HomeKit Accessory Protocol Specification R14
            // Table 6-3 Properties of Characteristic Objects in JSON
            if (maxLength == 64) {
                return kHAPError_None;
            }

            // See HomeKit Accessory Protocol Specification - iOS 9 Developer Preview R3
            // Section 5.12.10 Minimum and Maximum Length Descriptor
            uint8_t rangeBytes[] = { HAPExpandLittleUInt16(0), HAPExpandLittleUInt16(maxLength) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_GATTValidRange,
                                      .value = { .bytes = rangeBytes, .numBytes = sizeof rangeBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_TLV8: {
        }
            return kHAPError_None;
    }
    HAPFatalError();
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPStepValueDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    HAPPrecondition(responseWriter);

    HAPError err;

    switch (*((const HAPCharacteristicFormat*) characteristic_)) {
        case kHAPCharacteristicFormat_Data:
        case kHAPCharacteristicFormat_Bool:
        case kHAPCharacteristicFormat_String:
        case kHAPCharacteristicFormat_TLV8: {
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt8: {
            const HAPUInt8Characteristic* characteristic = characteristic_;
            uint8_t stepValue = characteristic->constraints.stepValue;

            if (stepValue <= 1) {
                return kHAPError_None;
            }

            uint8_t stepBytes[] = { stepValue };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt16: {
            const HAPUInt16Characteristic* characteristic = characteristic_;
            uint16_t stepValue = characteristic->constraints.stepValue;

            if (stepValue <= 1) {
                return kHAPError_None;
            }

            uint8_t stepBytes[] = { HAPExpandLittleUInt16(stepValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt32: {
            const HAPUInt32Characteristic* characteristic = characteristic_;
            uint32_t stepValue = characteristic->constraints.stepValue;

            if (stepValue <= 1) {
                return kHAPError_None;
            }

            uint8_t stepBytes[] = { HAPExpandLittleUInt32(stepValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_UInt64: {
            const HAPUInt64Characteristic* characteristic = characteristic_;
            uint64_t stepValue = characteristic->constraints.stepValue;

            if (stepValue <= 1) {
                return kHAPError_None;
            }

            uint8_t stepBytes[] = { HAPExpandLittleUInt64(stepValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Int: {
            const HAPIntCharacteristic* characteristic = characteristic_;
            int32_t stepValue = characteristic->constraints.stepValue;
            HAPPrecondition(stepValue >= 0);

            if (stepValue <= 1) {
                return kHAPError_None;
            }

            uint8_t stepBytes[] = { HAPExpandLittleInt32(stepValue) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
        case kHAPCharacteristicFormat_Float: {
            const HAPFloatCharacteristic* characteristic = characteristic_;
            float stepValue = characteristic->constraints.stepValue;
            HAPPrecondition(HAPFloatIsFinite(stepValue));
            HAPPrecondition(stepValue >= 0);

            if (HAPFloatIsZero(stepValue)) {
                return kHAPError_None;
            }

            uint32_t bitPattern = HAPFloatGetBitPattern(stepValue);
            uint8_t stepBytes[] = { HAPExpandLittleUInt32(bitPattern) };
            err = HAPTLVWriterAppend(
                    responseWriter,
                    &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPStepValueDescriptor,
                                      .value = { .bytes = stepBytes, .numBytes = sizeof stepBytes } });
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                return err;
            }
        }
            return kHAPError_None;
    }
    HAPFatalError();
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPServiceProperties(
        const HAPService* _Nullable service,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Table 7-49 HAP Service Properties
    uint16_t properties = 0;
    if (service) {
        properties = (uint16_t)(
                (service->properties.primaryService ? 0x0001 : 0) | (service->properties.hidden ? 0x0002 : 0) |
                (service->properties.ble.supportsConfiguration ? 0x0004 : 0));
    }

    // Accessories must include the "HAP Service Properties" characteristic only if it supports non-default properties
    // or has linked services. Other services must not include this characteristic.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.4.4 HAP Service Properties
    if (!properties && (!service || !service->linkedServices || !service->linkedServices[0])) {
        return kHAPError_None;
    }

    uint8_t propertiesBytes[] = { HAPExpandLittleUInt16(properties) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPServiceProperties,
                              .value = { .bytes = propertiesBytes, .numBytes = sizeof propertiesBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPLinkedServices(const HAPService* _Nullable service, HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.4.4.1 HAP Linked Services
    size_t linkedServicesCount = 0;
    if (service && service->linkedServices) {
        while (service->linkedServices[linkedServicesCount]) {
            linkedServicesCount++;
        }
    }

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

    uint8_t* linkedServicesBytes = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, 2 * linkedServicesCount);
    if (!linkedServicesBytes) {
        HAPLog(&logObject, "Not enough memory to allocate HAP-Param-HAP-Linked-Services.");
        return kHAPError_OutOfResources;
    }

    for (size_t i = 0; i < linkedServicesCount; i++) {
        HAPWriteLittleUInt16(&linkedServicesBytes[2 * i], service->linkedServices[i]);
    }

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPLinkedServices,
                              .value = { .bytes = linkedServicesBytes, .numBytes = 2 * linkedServicesCount } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPValidValuesDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.5.3 Valid Values Descriptor
    if (characteristic->format != kHAPCharacteristicFormat_UInt8) {
        return kHAPError_None;
    }

    const uint8_t* const* validValues = ((const HAPUInt8Characteristic*) characteristic)->constraints.validValues;
    size_t validValuesCount = 0;
    if (validValues) {
        while (validValues[validValuesCount]) {
            validValuesCount++;
        }
    }
    if (!validValuesCount) {
        return kHAPError_None;
    }

    // See HomeKit Accessory Protocol Specification R14
    // Section 2.3.3.1 Valid Characteristic Values
    HAPPrecondition(HAPUUIDIsAppleDefined(characteristic->characteristicType));

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

    uint8_t* validValuesBytes = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, validValuesCount);
    if (!validValuesBytes) {
        HAPLog(&logObject, "Not enough memory to allocate HAP-Param-HAP-Valid-Values-Descriptor.");
        return kHAPError_OutOfResources;
    }

    for (size_t i = 0; i < validValuesCount; i++) {
        validValuesBytes[i] = *validValues[i];

        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.5.3 Valid Values Descriptor
        if (i) {
            HAPPrecondition(validValuesBytes[i] > validValuesBytes[i - 1]);
        }
    }

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPValidValuesDescriptor,
                              .value = { .bytes = validValuesBytes, .numBytes = validValuesCount } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPValidValuesRangeDescriptor(
        const HAPCharacteristic* characteristic_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.5.4 Valid Values Range Descriptor
    if (characteristic->format != kHAPCharacteristicFormat_UInt8) {
        return kHAPError_None;
    }

    const HAPUInt8CharacteristicValidValuesRange* const* validValuesRanges =
            ((const HAPUInt8Characteristic*) characteristic)->constraints.validValuesRanges;
    size_t validValuesRangesCount = 0;
    if (validValuesRanges) {
        while (validValuesRanges[validValuesRangesCount]) {
            validValuesRangesCount++;
        }
    }
    if (!validValuesRangesCount) {
        return kHAPError_None;
    }

    // See HomeKit Accessory Protocol Specification R14
    // Section 2.3.3.1 Valid Characteristic Values
    HAPPrecondition(HAPUUIDIsAppleDefined(characteristic->characteristicType));

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

    uint8_t* validValuesRangesBytes = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, 2 * validValuesRangesCount);
    if (!validValuesRangesBytes) {
        HAPLog(&logObject, "Not enough memory to allocate HAP-Param-HAP-Valid-Values-Range-Descriptor.");
        return kHAPError_OutOfResources;
    }

    for (size_t i = 0; i < validValuesRangesCount; i++) {
        HAPPrecondition(validValuesRanges[i]->start <= validValuesRanges[i]->end);

        validValuesRangesBytes[2 * i + 0] = validValuesRanges[i]->start;
        validValuesRangesBytes[2 * i + 1] = validValuesRanges[i]->end;

        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.5.4 Valid Values Range Descriptor
        if (i) {
            HAPPrecondition(validValuesRangesBytes[2 * i] > validValuesRangesBytes[2 * i - 1]);
        }
    }

    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPBLEPDUTLVType_HAPValidValuesRangeDescriptor,
                              .value = { .bytes = validValuesRangesBytes, .numBytes = 2 * validValuesRangesCount } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}
