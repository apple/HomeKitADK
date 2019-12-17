// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "RequestHandlers" };

HAP_RESULT_USE_CHECK
HAPError HAPHandleServiceSignatureRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPDataCharacteristicReadRequest* request HAP_UNUSED,
        void* valueBytes HAP_UNUSED,
        size_t maxValueBytes HAP_UNUSED,
        size_t* numValueBytes,
        void* _Nullable context HAP_UNUSED) {
    // This seems to be required, as HAT crashes when characteristics properties are 0.
    // Also, iOS 11 does not detect Service Signature for R10+ Protocol Configuration PDU if it is not readable.
    HAPLog(&logObject,
           "Sending dummy response to work around controller bugs with characteristic having 0 properties.");
    *numValueBytes = 0;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPHandleNameRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(request->service->name);
    const char* name = request->service->name;

    size_t numBytes = HAPStringGetNumBytes(name);
    if (numBytes >= maxValueBytes) {
        HAPLog(&logObject, "Not enough space available to send service name.");
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, name, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}
