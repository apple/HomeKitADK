// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_PDU_TLV_H
#define HAP_BLE_PDU_TLV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * HAP-BLE PDU Body Additional Parameter Types.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 7-10 Additional Parameter Types Description
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEPDUTLVType) {
    /** HAP-Param-Value. */
    kHAPBLEPDUTLVType_Value = 0x01,

    /** HAP-Param-Additional-Authorization-Data. */
    kHAPBLEPDUTLVType_AdditionalAuthorizationData = 0x02,

    /** HAP-Param-Origin (local vs remote). */
    kHAPBLEPDUTLVType_Origin = 0x03,

    /** HAP-Param-Characteristic-Type. */
    kHAPBLEPDUTLVType_CharacteristicType = 0x04,

    /** HAP-Param-Characteristic-Instance-ID. */
    kHAPBLEPDUTLVType_CharacteristicInstanceID = 0x05,

    /** HAP-Param-Service-Type. */
    kHAPBLEPDUTLVType_ServiceType = 0x06,

    /** HAP-Param-Service-Instance-ID. */
    kHAPBLEPDUTLVType_ServiceInstanceID = 0x07,

    /** HAP-Param-TTL. */
    kHAPBLEPDUTLVType_TTL = 0x08,

    /** HAP-Param-Return-Response. */
    kHAPBLEPDUTLVType_ReturnResponse = 0x09,

    /** HAP-Param-HAP-Characteristic-Properties-Descriptor. */
    kHAPBLEPDUTLVType_HAPCharacteristicPropertiesDescriptor = 0x0A,

    /** HAP-Param-GATT-User-Description-Descriptor. */
    kHAPBLEPDUTLVType_GATTUserDescriptionDescriptor = 0x0B,

    /** HAP-Param-GATT-Presentation-Format-Descriptor. */
    kHAPBLEPDUTLVType_GATTPresentationFormatDescriptor = 0x0C,

    /** HAP-Param-GATT-Valid-Range. */
    kHAPBLEPDUTLVType_GATTValidRange = 0x0D,

    /** HAP-Param-HAP-Step-Value-Descriptor. */
    kHAPBLEPDUTLVType_HAPStepValueDescriptor = 0x0E,

    /** HAP-Param-HAP-Service-Properties. */
    kHAPBLEPDUTLVType_HAPServiceProperties = 0x0F,

    /** HAP-Param-HAP-Linked-Services. */
    kHAPBLEPDUTLVType_HAPLinkedServices = 0x10,

    /** HAP-Param-HAP-Valid-Values-Descriptor. */
    kHAPBLEPDUTLVType_HAPValidValuesDescriptor = 0x11,

    /** HAP-Param-HAP-Valid-Values-Range-Descriptor */
    kHAPBLEPDUTLVType_HAPValidValuesRangeDescriptor = 0x12,
} HAP_ENUM_END(uint8_t, HAPBLEPDUTLVType);

/**
 * Serializes HAP-Param-Characteristic-Type.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeCharacteristicType(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-Service-Type.
 *
 * @param      service              Service.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeServiceType(const HAPService* service, HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-Service-Instance-ID.
 *
 * @param      service              Service.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeServiceInstanceID(const HAPService* service, HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Characteristic-Properties-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPCharacteristicPropertiesDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-GATT-User-Description-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeGATTUserDescriptionDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-GATT-Presentation-Format-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeGATTPresentationFormatDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-GATT-Valid-Range.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeGATTValidRange(const HAPCharacteristic* characteristic, HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Step-Value-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.2 HAP-Characteristic-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPStepValueDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Service-Properties.
 *
 * @param      service              Service. NULL if the request had an invalid IID.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.13 HAP-Service-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError
        HAPBLEPDUTLVSerializeHAPServiceProperties(const HAPService* _Nullable service, HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Linked-Services.
 *
 * @param      service              Service. NULL if the request had an invalid IID.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.13 HAP-Service-Signature-Read-Response
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPLinkedServices(const HAPService* _Nullable service, HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Valid-Values-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.3 HAP-Characteristic-Signature-Read-Response (with Valid Values)
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPValidValuesDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

/**
 * Serializes HAP-Param-HAP-Valid-Values-Range-Descriptor.
 *
 * @param      characteristic       Characteristic.
 * @param      responseWriter       TLV writer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If writer does not have enough capacity.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.4.3 HAP-Characteristic-Signature-Read-Response (with Valid Values)
 */
HAP_RESULT_USE_CHECK
HAPError HAPBLEPDUTLVSerializeHAPValidValuesRangeDescriptor(
        const HAPCharacteristic* characteristic,
        HAPTLVWriterRef* responseWriter);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
