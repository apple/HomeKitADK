// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "RequestHandlers" };

HAP_RESULT_USE_CHECK
HAPError HAPHandleHAPProtocolInformationVersionRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request,
        char* value,
        size_t maxValueBytes,
        void* _Nullable context HAP_UNUSED) {
    switch (request->transportType) {
        case kHAPTransportType_IP: {
            static const char version[] = kHAPProtocolVersion_IP;
            size_t numBytes = sizeof version;
            if (numBytes >= maxValueBytes) {
                HAPLog(&logObject, "Not enough space available to send IP protocol version.");
                return kHAPError_OutOfResources;
            }
            HAPRawBufferCopyBytes(value, version, numBytes);
        }
            return kHAPError_None;
        case kHAPTransportType_BLE: {
            static const char version[] = kHAPProtocolVersion_BLE;
            size_t numBytes = sizeof version;
            if (numBytes >= maxValueBytes) {
                HAPLog(&logObject, "Not enough space available to send BLE protocol version.");
                return kHAPError_OutOfResources;
            }
            HAPRawBufferCopyBytes(value, version, numBytes);
        }
            return kHAPError_None;
    }
    HAPFatalError();
}
