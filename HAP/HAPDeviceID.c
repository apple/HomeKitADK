// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "DeviceID" };

HAP_RESULT_USE_CHECK
HAPError HAPDeviceIDGet(HAPPlatformKeyValueStoreRef keyValueStore, HAPDeviceID* deviceID) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(deviceID);

    HAPError err;

    // Try to load Device ID.
    bool found;
    size_t numBytes;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_DeviceID,
            deviceID->bytes,
            sizeof deviceID->bytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        // Generate new Device ID.
        HAPPlatformRandomNumberFill(deviceID->bytes, sizeof deviceID->bytes);
        HAPLogBufferInfo(&logObject, deviceID->bytes, sizeof deviceID->bytes, "Generated new Device ID.");

        // Store new Device ID.
        err = HAPPlatformKeyValueStoreSet(
                keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_DeviceID,
                deviceID->bytes,
                sizeof deviceID->bytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    } else if (numBytes != sizeof deviceID->bytes) {
        HAPLog(&logObject, "Invalid Device ID.");
        return kHAPError_Unknown;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPDeviceIDGetAsString(HAPPlatformKeyValueStoreRef keyValueStore, HAPDeviceIDString* deviceIDString) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(deviceIDString);

    HAPError err;

    HAPDeviceID deviceID;
    err = HAPDeviceIDGet(keyValueStore, &deviceID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    HAPAssert(sizeof deviceID.bytes == 6);
    err = HAPStringWithFormat(
            deviceIDString->stringValue,
            sizeof deviceIDString->stringValue,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            deviceID.bytes[0],
            deviceID.bytes[1],
            deviceID.bytes[2],
            deviceID.bytes[3],
            deviceID.bytes[4],
            deviceID.bytes[5]);
    HAPAssert(!err);

    return kHAPError_None;
}
