// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_CHARACTERISTIC_H
#define HAP_CHARACTERISTIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * HomeKit characteristic.
 *
 * This contains common fields across all characteristics.
 * Do not use this directly, but use the concrete HAPXxxCharacteristic types instead.
 *
 * - IMPORTANT: This must stay in sync across ALL characteristic structures!
 */
typedef struct {
    /**
     * Format.
     *
     * - IMPORTANT: This must remain the very first field! Sometimes, we cast the opaque pointers to this item.
     */
    HAPCharacteristicFormat format;

    /**
     * Instance ID.
     *
     * - Must not be 0.
     * - Must not change while the accessory is paired, including over firmware updates.
     * - For accessories that support Bluetooth LE, must not exceed UINT16_MAX.
     */
    uint64_t iid;

    /**
     * The type of the characteristic.
     */
    const HAPUUID* characteristicType;

    /**
     * Description for debugging (based on "Type" field of HAP specification).
     */
    const char* debugDescription;

    /**
     * Description of the characteristic provided by the manufacturer of the accessory.
     */
    const char* _Nullable manufacturerDescription;

    /**
     * Characteristic properties.
     */
    HAPCharacteristicProperties properties;
} HAPBaseCharacteristic;
HAP_NONNULL_SUPPORT(HAPBaseCharacteristic)

// <editor-fold desc="Static asserts" defaultstate="collapsed">

HAP_STATIC_ASSERT(HAP_OFFSETOF(HAPBaseCharacteristic, format) == 0, HAPBaseCharacteristic_format);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPDataCharacteristic, format),
        HAPDataCharacteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPDataCharacteristic, iid),
        HAPDataCharacteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPDataCharacteristic, characteristicType),
        HAPDataCharacteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPDataCharacteristic, debugDescription),
        HAPDataCharacteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPDataCharacteristic, manufacturerDescription),
        HAPDataCharacteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPDataCharacteristic, properties),
        HAPDataCharacteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPBoolCharacteristic, format),
        HAPBoolCharacteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPBoolCharacteristic, iid),
        HAPBoolCharacteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPBoolCharacteristic, characteristicType),
        HAPBoolCharacteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPBoolCharacteristic, debugDescription),
        HAPBoolCharacteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPBoolCharacteristic, manufacturerDescription),
        HAPBoolCharacteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPBoolCharacteristic, properties),
        HAPBoolCharacteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPUInt8Characteristic, format),
        HAPUInt8Characteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPUInt8Characteristic, iid),
        HAPUInt8Characteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPUInt8Characteristic, characteristicType),
        HAPUInt8Characteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPUInt8Characteristic, debugDescription),
        HAPUInt8Characteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPUInt8Characteristic, manufacturerDescription),
        HAPUInt8Characteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPUInt8Characteristic, properties),
        HAPUInt8Characteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPUInt16Characteristic, format),
        HAPUInt16Characteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPUInt16Characteristic, iid),
        HAPUInt16Characteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPUInt16Characteristic, characteristicType),
        HAPUInt16Characteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) ==
                HAP_OFFSETOF(HAPUInt16Characteristic, debugDescription),
        HAPUInt16Characteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPUInt16Characteristic, manufacturerDescription),
        HAPUInt16Characteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPUInt16Characteristic, properties),
        HAPUInt16Characteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPUInt32Characteristic, format),
        HAPUInt32Characteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPUInt32Characteristic, iid),
        HAPUInt32Characteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPUInt32Characteristic, characteristicType),
        HAPUInt32Characteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) ==
                HAP_OFFSETOF(HAPUInt32Characteristic, debugDescription),
        HAPUInt32Characteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPUInt32Characteristic, manufacturerDescription),
        HAPUInt32Characteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPUInt32Characteristic, properties),
        HAPUInt32Characteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPUInt64Characteristic, format),
        HAPUInt64Characteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPUInt64Characteristic, iid),
        HAPUInt64Characteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPUInt64Characteristic, characteristicType),
        HAPUInt64Characteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) ==
                HAP_OFFSETOF(HAPUInt64Characteristic, debugDescription),
        HAPUInt64Characteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPUInt64Characteristic, manufacturerDescription),
        HAPUInt64Characteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPUInt64Characteristic, properties),
        HAPUInt64Characteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPIntCharacteristic, format),
        HAPIntCharacteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPIntCharacteristic, iid),
        HAPIntCharacteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPIntCharacteristic, characteristicType),
        HAPIntCharacteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPIntCharacteristic, debugDescription),
        HAPIntCharacteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPIntCharacteristic, manufacturerDescription),
        HAPIntCharacteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPIntCharacteristic, properties),
        HAPIntCharacteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPFloatCharacteristic, format),
        HAPFloatCharacteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPFloatCharacteristic, iid),
        HAPFloatCharacteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPFloatCharacteristic, characteristicType),
        HAPFloatCharacteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPFloatCharacteristic, debugDescription),
        HAPFloatCharacteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPFloatCharacteristic, manufacturerDescription),
        HAPFloatCharacteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPFloatCharacteristic, properties),
        HAPFloatCharacteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPStringCharacteristic, format),
        HAPStringCharacteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPStringCharacteristic, iid),
        HAPStringCharacteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPStringCharacteristic, characteristicType),
        HAPStringCharacteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) ==
                HAP_OFFSETOF(HAPStringCharacteristic, debugDescription),
        HAPStringCharacteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPStringCharacteristic, manufacturerDescription),
        HAPStringCharacteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPStringCharacteristic, properties),
        HAPStringCharacteristic_properties);

HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, format) == HAP_OFFSETOF(HAPTLV8Characteristic, format),
        HAPTLV8Characteristic_format);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, iid) == HAP_OFFSETOF(HAPTLV8Characteristic, iid),
        HAPTLV8Characteristic_iid);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, characteristicType) ==
                HAP_OFFSETOF(HAPTLV8Characteristic, characteristicType),
        HAPTLV8Characteristic_characteristicType);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, debugDescription) == HAP_OFFSETOF(HAPTLV8Characteristic, debugDescription),
        HAPTLV8Characteristic_debugDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, manufacturerDescription) ==
                HAP_OFFSETOF(HAPTLV8Characteristic, manufacturerDescription),
        HAPTLV8Characteristic_manufacturerDescription);
HAP_STATIC_ASSERT(
        HAP_OFFSETOF(HAPBaseCharacteristic, properties) == HAP_OFFSETOF(HAPTLV8Characteristic, properties),
        HAPTLV8Characteristic_properties);

// </editor-fold>

/**
 * Returns whether a characteristic is only accessible for read operations by admin controllers.
 *
 * @param      characteristic       Characteristic.
 *
 * @return true                     If the characteristic is only accessible for read operations by admin controllers.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPCharacteristicReadRequiresAdminPermissions(const HAPCharacteristic* characteristic);

/**
 * Returns whether a characteristic is only accessible for write operations by admin controllers.
 *
 * @param      characteristic       Characteristic.
 *
 * @return true                     If the characteristic is only accessible for write operations by admin controllers.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPCharacteristicWriteRequiresAdminPermissions(const HAPCharacteristic* characteristic);

/**
 * Reads a Data characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] valueBytes           Value buffer.
 * @param      maxValueBytes        Capacity of value buffer.
 * @param[out] numValueBytes        Length of value buffer.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPDataCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicReadRequest* request,
        void* valueBytes,
        size_t maxValueBytes,
        size_t* numValueBytes,
        void* _Nullable context);

/**
 * Writes a Data characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      valueBytes           Value buffer.
 * @param      numValueBytes        Length of value buffer.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPDataCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicWriteRequest* request,
        const void* valueBytes,
        size_t numValueBytes,
        void* _Nullable context);

/**
 * Invokes the Data characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPDataCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the Data characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPDataCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPDataCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a Bool characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBoolCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicReadRequest* request,
        bool* value,
        void* _Nullable context);

/**
 * Writes a Bool characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPBoolCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context);

/**
 * Invokes the Bool characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPBoolCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the Bool characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPBoolCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a UInt8 characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt8CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicReadRequest* request,
        uint8_t* value,
        void* _Nullable context);

/**
 * Writes a UInt8 characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt8CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicWriteRequest* request,
        uint8_t value,
        void* _Nullable context);

/**
 * Invokes the UInt8 characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt8CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the UInt8 characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt8CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a UInt16 characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt16CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicReadRequest* request,
        uint16_t* value,
        void* _Nullable context);

/**
 * Writes a UInt16 characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt16CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicWriteRequest* request,
        uint16_t value,
        void* _Nullable context);

/**
 * Invokes the UInt16 characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt16CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the UInt16 characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt16CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt16CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a UInt32 characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt32CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicReadRequest* request,
        uint32_t* value,
        void* _Nullable context);

/**
 * Writes a UInt32 characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt32CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicWriteRequest* request,
        uint32_t value,
        void* _Nullable context);

/**
 * Invokes the UInt32 characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt32CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the UInt32 characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt32CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt32CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a UInt64 characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt64CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicReadRequest* request,
        uint64_t* value,
        void* _Nullable context);

/**
 * Writes a UInt64 characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPUInt64CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicWriteRequest* request,
        uint64_t value,
        void* _Nullable context);

/**
 * Invokes the UInt64 characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt64CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the UInt64 characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPUInt64CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPUInt64CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads an Int characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIntCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicReadRequest* request,
        int32_t* value,
        void* _Nullable context);

/**
 * Writes an Int characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIntCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicWriteRequest* request,
        int32_t value,
        void* _Nullable context);

/**
 * Invokes the Int characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPIntCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the Int characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPIntCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPIntCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a Float characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPFloatCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicReadRequest* request,
        float* value,
        void* _Nullable context);

/**
 * Writes a Float characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPFloatCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicWriteRequest* request,
        float value,
        void* _Nullable context);

/**
 * Invokes the Float characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPFloatCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the Float characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPFloatCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPFloatCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a String characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param[out] value                Value. NULL-terminated.
 * @param      maxValueBytes        Capacity of value. NULL-terminator must fit within capacity.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPStringCharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Writes a String characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      value                Value. NULL-terminated.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPStringCharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicWriteRequest* request,
        const char* value,
        void* _Nullable context);

/**
 * Invokes the String characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPStringCharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the String characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPStringCharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicSubscriptionRequest* request,
        void* _Nullable context);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Reads a TLV8 characteristic value.
 *
 * - It is ensured that the returned value satisfies the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      responseWriter       TLV writer for serializing the response.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPTLV8CharacteristicHandleRead(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicReadRequest* request,
        HAPTLVWriterRef* responseWriter,
        void* _Nullable context);

/**
 * Writes a TLV8 characteristic value.
 *
 * - The value is checked against the constraints of the characteristic.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      requestReader        TLV reader for parsing the value.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If unable to perform operation with requested service or characteristic.
 * @return kHAPError_InvalidState   If the request cannot be processed in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If out of resources to process request.
 * @return kHAPError_NotAuthorized  If additional authorization data is insufficient.
 * @return kHAPError_Busy           If the request failed temporarily.
 */
HAP_RESULT_USE_CHECK
HAPError HAPTLV8CharacteristicHandleWrite(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicWriteRequest* request,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context);

/**
 * Invokes the TLV8 characteristic subscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPTLV8CharacteristicHandleSubscribe(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

/**
 * Invokes the TLV8 characteristic unsubscribe handler, if available.
 *
 * @param      server               Accessory server.
 * @param      request              Request.
 * @param      context              The context parameter given to the HAPAccessoryServerCreate function.
 */
void HAPTLV8CharacteristicHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        const HAPTLV8CharacteristicSubscriptionRequest* request,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
