// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_REQUEST_HANDLERS_ACCESSORY_INFORMATION_H
#define HAP_REQUEST_HANDLERS_ACCESSORY_INFORMATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Handle write request to the 'Identify' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationIdentifyWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context);

/**
 * Handle read request to the 'Manufacturer' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationManufacturerRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'Model' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationModelRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'Name' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationNameRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'Serial Number' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationSerialNumberRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'Firmware Revision' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationFirmwareRevisionRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'Hardware Revision' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationHardwareRevisionRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

/**
 * Handle read request to the 'ADK Version' characteristic of the Accessory Information service.
 */
HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationADKVersionRead(
        HAPAccessoryServerRef* server,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
