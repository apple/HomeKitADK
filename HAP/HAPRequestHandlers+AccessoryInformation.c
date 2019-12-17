// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "RequestHandlers" };

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationIdentifyWrite(
        HAPAccessoryServerRef* server,
        const HAPBoolCharacteristicWriteRequest* request,
        bool value,
        void* _Nullable context) {
    HAPAssert(request->accessory->callbacks.identify);

    if (!value) {
        HAPLog(&logObject, "Received invalid identify request.");
        return kHAPError_InvalidData;
    }

    return request->accessory->callbacks.identify(
            server,
            &(const HAPAccessoryIdentifyRequest) { .transportType = request->transportType,
                                                   .session = request->session,
                                                   .accessory = request->accessory,
                                                   .remote = request->remote },
            context);
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationManufacturerRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* stringToCopy = request->accessory->manufacturer;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes >= 1);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationModelRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* stringToCopy = request->accessory->model;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationNameRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* stringToCopy = request->accessory->name;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationSerialNumberRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* stringToCopy = request->accessory->serialNumber;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes >= 1);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationFirmwareRevisionRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* stringToCopy = request->accessory->firmwareVersion;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes >= 1);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationHardwareRevisionRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    HAPAssert(request->accessory->hardwareVersion);
    const char* stringToCopy = request->accessory->hardwareVersion;
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    HAPAssert(numBytes >= 1);
    HAPAssert(numBytes <= 64);
    if (numBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleAccessoryInformationADKVersionRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    const char* versionToCopy = HAPGetVersion();
    size_t numVersionBytes = HAPStringGetNumBytes(versionToCopy);
    HAPAssert(numVersionBytes >= 1);
    HAPAssert(numVersionBytes <= 64);
    const char* buildToCopy = HAPGetBuild();
    size_t numBuildBytes = HAPStringGetNumBytes(buildToCopy);
    HAPAssert(numBuildBytes >= 1);
    HAPAssert(numBuildBytes <= 64);
    if (numVersionBytes + 1 + numBuildBytes >= maxValueBytes) {
        HAPLogCharacteristic(
                &logObject,
                request->characteristic,
                request->service,
                request->accessory,
                "Not enough space (needed: %zu, available: %zu).",
                numVersionBytes + 1 + numBuildBytes + 1,
                maxValueBytes);
        return kHAPError_OutOfResources;
    }
    size_t position = 0;
    HAPRawBufferCopyBytes(&value[position], versionToCopy, numVersionBytes);
    position += numVersionBytes;
    value[position] = ';';
    position++;
    HAPRawBufferCopyBytes(&value[position], buildToCopy, numBuildBytes);
    position += numBuildBytes;
    HAPAssert(position < maxValueBytes);
    value[position] = '\0';
    return kHAPError_None;
}
