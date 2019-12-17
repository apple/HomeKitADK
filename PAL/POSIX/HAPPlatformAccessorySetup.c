// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform+Init.h"
#include "HAPPlatformAccessorySetup+Init.h"
#include "HAPPlatformFileManager.h"
#include "HAPPlatformKeyValueStore+SDKDomains.h"

static const HAPLogObject logObject = { .subsystem = kHAPPlatform_LogSubsystem, .category = "AccessorySetup" };

void HAPPlatformAccessorySetupCreate(
        HAPPlatformAccessorySetupRef accessorySetup,
        const HAPPlatformAccessorySetupOptions* options) {
    HAPPrecondition(accessorySetup);
    HAPPrecondition(options);
    HAPPrecondition(options->keyValueStore);

    HAPLogDebug(&logObject, "Storage configuration: accessorySetup = %lu", (unsigned long) sizeof *accessorySetup);

    HAPRawBufferZero(accessorySetup, sizeof *accessorySetup);

    accessorySetup->keyValueStore = options->keyValueStore;
}

void HAPPlatformAccessorySetupLoadSetupInfo(HAPPlatformAccessorySetupRef accessorySetup, HAPSetupInfo* setupInfo) {
    HAPPrecondition(accessorySetup);
    HAPPrecondition(setupInfo);

    HAPError err;

    bool found;
    size_t numBytes;
    err = HAPPlatformKeyValueStoreGet(
            accessorySetup->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_SetupInfo,
            setupInfo,
            sizeof *setupInfo,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    if (!found) {
        HAPLogError(&logObject, "No setup code found in key-value store.");
        HAPFatalError();
    }
    if (numBytes != sizeof *setupInfo) {
        HAPLogError(&logObject, "Invalid setup code size %zu.", numBytes);
        HAPFatalError();
    }
}

void HAPPlatformAccessorySetupLoadSetupCode(HAPPlatformAccessorySetupRef accessorySetup, HAPSetupCode* setupCode) {
    HAPPrecondition(accessorySetup);
    HAPPrecondition(setupCode);

    HAPError err;

    bool found;
    size_t numBytes;
    err = HAPPlatformKeyValueStoreGet(
            accessorySetup->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_SetupCode,
            setupCode,
            sizeof *setupCode,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    if (!found) {
        HAPLogError(&logObject, "No setup code found in key-value store.");
        HAPFatalError();
    }
    if (numBytes != sizeof *setupCode) {
        HAPLogError(&logObject, "Invalid setup code size %zu.", numBytes);
        HAPFatalError();
    }
}

void HAPPlatformAccessorySetupLoadSetupID(
        HAPPlatformAccessorySetupRef accessorySetup,
        bool* valid,
        HAPSetupID* setupID) {
    HAPPrecondition(accessorySetup);
    HAPPrecondition(valid);
    HAPPrecondition(setupID);

    HAPError err;

    size_t numBytes;
    err = HAPPlatformKeyValueStoreGet(
            accessorySetup->keyValueStore,
            kSDKKeyValueStoreDomain_Provisioning,
            kSDKKeyValueStoreKey_Provisioning_SetupID,
            setupID,
            sizeof *setupID,
            &numBytes,
            valid);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    if (!*valid) {
        HAPLog(&logObject, "No setup ID found. QR codes and NFC require provisioning a setup ID.");
        return;
    }
    if (numBytes != sizeof *setupID) {
        HAPLogError(&logObject, "Invalid setup ID size %zu.", numBytes);
        HAPFatalError();
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Deprecated APIs.

HAP_DEPRECATED_MSG("Return false and use HAPPlatformAccessorySetupDisplay / HAPPlatformAccessorySetupNFC instead.")
HAP_RESULT_USE_CHECK
HAPPlatformAccessorySetupCapabilities
        HAPPlatformAccessorySetupGetCapabilities(HAPPlatformAccessorySetupRef accessorySetup) {
    HAPPrecondition(accessorySetup);

    // Deprecated. Return false and use HAPPlatformAccessorySetupDisplay / HAPPlatformAccessorySetupNFC instead.
    return (HAPPlatformAccessorySetupCapabilities) { .supportsDisplay = false, .supportsProgrammableNFC = false };
}

void HAPPlatformAccessorySetupUpdateSetupPayload(
        HAPPlatformAccessorySetupRef accessorySetup,
        const HAPSetupPayload* _Nullable setupPayload,
        const HAPSetupCode* _Nullable setupCode) {
    HAPPrecondition(accessorySetup);

    (void) setupPayload;
    (void) setupCode;
    HAPLogError(&logObject, "[NYI] %s.", __func__);
    HAPFatalError();
}
