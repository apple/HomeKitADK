// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

// An example that implements the lock HomeKit profile. It can serve as a basic implementation for
// any platform. The accessory logic implementation is reduced to internal state updates and log output.
// The example covers the Lock Mechanism service and the linked Lock Management service.
//
// This implementation is platform-independent.
//
// The code consists of multiple parts:
//
//   1. The definition of the accessory configuration and its internal state.
//
//   2. Helper functions to load and save the state of the accessory.
//
//   3. The definitions for the HomeKit attribute database.
//
//   4. The callbacks that implement the actual behavior of the accessory, in this
//      case here they merely access the global accessory state variable and write
//      to the log to make the behavior easily observable.
//
//   5. The initialization of the accessory state.
//
//   6. Callbacks that notify the server in case their associated value has changed.

#include "HAP.h"

#include "App.h"
#include "DB.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Domain used in the key value store for application data.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreDomain_Configuration ((HAPPlatformKeyValueStoreDomain) 0x00)

/**
 * Key used in the key value store to store the configuration state.
 *
 * Purged: On factory reset.
 */
#define kAppKeyValueStoreKey_Configuration_State ((HAPPlatformKeyValueStoreDomain) 0x00)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Global accessory configuration.
 */
typedef struct {
    struct {
        HAPCharacteristicValue_LockCurrentState currentState;
        HAPCharacteristicValue_LockTargetState targetState;
    } state;
    HAPAccessoryServerRef* server;
    HAPPlatformKeyValueStoreRef keyValueStore;
} AccessoryConfiguration;

static AccessoryConfiguration accessoryConfiguration;

//----------------------------------------------------------------------------------------------------------------------

/**
 * Load the accessory state from persistent memory.
 */
static void LoadAccessoryState(void) {
    HAPPrecondition(accessoryConfiguration.keyValueStore);

    HAPError err;

    // Load persistent state if available
    bool found;
    size_t numBytes;

    err = HAPPlatformKeyValueStoreGet(
            accessoryConfiguration.keyValueStore,
            kAppKeyValueStoreDomain_Configuration,
            kAppKeyValueStoreKey_Configuration_State,
            &accessoryConfiguration.state,
            sizeof accessoryConfiguration.state,
            &numBytes,
            &found);

    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    if (!found || numBytes != sizeof accessoryConfiguration.state) {
        if (found) {
            HAPLogError(&kHAPLog_Default, "Unexpected app state found in key-value store. Resetting to default.");
        }
        HAPRawBufferZero(&accessoryConfiguration.state, sizeof accessoryConfiguration.state);
    }
}

/**
 * Save the accessory state to persistent memory
 */
static void SaveAccessoryState(void) {
    HAPPrecondition(accessoryConfiguration.keyValueStore);

    HAPError err;
    err = HAPPlatformKeyValueStoreSet(
            accessoryConfiguration.keyValueStore,
            kAppKeyValueStoreDomain_Configuration,
            kAppKeyValueStoreKey_Configuration_State,
            &accessoryConfiguration.state,
            sizeof accessoryConfiguration.state);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
}

//----------------------------------------------------------------------------------------------------------------------

/**
 * HomeKit accessory that provides the Lock service.
 */
static const HAPAccessory accessory = { .aid = 1,
                                        .category = kHAPAccessoryCategory_Locks,
                                        .name = "Acme Lock",
                                        .manufacturer = "Acme",
                                        .model = "Lock1,1",
                                        .serialNumber = "099DB48E9E28",
                                        .firmwareVersion = "1",
                                        .hardwareVersion = "1",
                                        .services = (const HAPService* const[]) { &accessoryInformationService,
                                                                                  &hapProtocolInformationService,
                                                                                  &pairingService,
                                                                                  &lockMechanismService,
                                                                                  &lockManagementService,
                                                                                  NULL },
                                        .callbacks = { .identify = IdentifyAccessory } };

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
HAPError IdentifyAccessory(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPAccessoryIdentifyRequest* request HAP_UNUSED,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    return kHAPError_None;
}

/**
 * Handle read request to the 'Lock Current State' characteristic of the Lock Mechanism service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleLockMechanismLockCurrentStateRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPUInt8CharacteristicReadRequest* request HAP_UNUSED,
        uint8_t* value,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    *value = accessoryConfiguration.state.currentState;
    switch (*value) {
        case kHAPCharacteristicValue_LockCurrentState_Secured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockCurrentState_Secured");
        } break;
        case kHAPCharacteristicValue_LockCurrentState_Unsecured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockCurrentState_Unsecured");
        } break;
        case kHAPCharacteristicValue_LockCurrentState_Jammed: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockCurrentState_Jammed");
        } break;
        case kHAPCharacteristicValue_LockCurrentState_Unknown: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockCurrentState_Unknown");
        } break;
    }
    return kHAPError_None;
}

/**
 * Handle read request to the 'Lock Target State' characteristic of the Lock Mechanism service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleLockMechanismLockTargetStateRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPUInt8CharacteristicReadRequest* request HAP_UNUSED,
        uint8_t* value,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    *value = accessoryConfiguration.state.targetState;
    switch (*value) {
        case kHAPCharacteristicValue_LockTargetState_Secured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockTargetState_Secured");
        } break;
        case kHAPCharacteristicValue_LockTargetState_Unsecured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockTargetState_Unsecured");
        } break;
    }
    return kHAPError_None;
}

/**
 * Handle write request to the 'Lock Target State' characteristic of the Lock Mechanism service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleLockMechanismLockTargetStateWrite(
        HAPAccessoryServerRef* server,
        const HAPUInt8CharacteristicWriteRequest* request,
        uint8_t value,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    HAPCharacteristicValue_LockTargetState targetState = (HAPCharacteristicValue_LockTargetState) value;
    switch (targetState) {
        case kHAPCharacteristicValue_LockTargetState_Secured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockTargetState_Secured");
        } break;
        case kHAPCharacteristicValue_LockTargetState_Unsecured: {
            HAPLogInfo(&kHAPLog_Default, "%s: %s", __func__, "LockTargetState_Unsecured");
        } break;
    }

    if (accessoryConfiguration.state.targetState != targetState) {
        accessoryConfiguration.state.targetState = targetState;
        HAPAccessoryServerRaiseEvent(server, request->characteristic, request->service, request->accessory);
        SaveAccessoryState();
    }
    return kHAPError_None;
}

/**
 * Handle write request to the 'Lock Control Point' characteristic of the Lock Management service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleLockManagementLockControlPointWrite(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPTLV8CharacteristicWriteRequest* request HAP_UNUSED,
        HAPTLVReaderRef* requestReader,
        void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(requestReader);

    HAPLogInfo(&kHAPLog_Default, "%s", __func__);

    HAPError err;

    // Simply validate input.
    err = HAPTLVReaderGetAll(requestReader, (HAPTLV* const[]) { NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }
    return kHAPError_None;
}

/**
 * Handle read request to the 'Version' characteristic of the Lock Management service.
 */
HAP_RESULT_USE_CHECK
HAPError HandleLockManagementVersionRead(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPStringCharacteristicReadRequest* request HAP_UNUSED,
        char* value,
        size_t maxBytes,
        void* _Nullable context HAP_UNUSED) {
    HAPLogInfo(&kHAPLog_Default, "%s", __func__);
    const char* stringToCopy = "1.0";
    size_t numBytes = HAPStringGetNumBytes(stringToCopy);
    if (numBytes >= maxBytes) {
        HAPLogError(
                &kHAPLog_Default,
                "Not enough space to store %s (needed: %zu, available: %zu).",
                "Version",
                numBytes + 1,
                maxBytes);
        return kHAPError_OutOfResources;
    }
    HAPRawBufferCopyBytes(value, stringToCopy, numBytes);
    value[numBytes] = '\0';
    return kHAPError_None;
}

//----------------------------------------------------------------------------------------------------------------------

void AppCreate(HAPAccessoryServerRef* server, HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(server);
    HAPPrecondition(keyValueStore);

    HAPLogInfo(&kHAPLog_Default, "%s", __func__);

    HAPRawBufferZero(&accessoryConfiguration, sizeof accessoryConfiguration);
    accessoryConfiguration.server = server;
    accessoryConfiguration.keyValueStore = keyValueStore;
    LoadAccessoryState();
}

void AppRelease(void) {
}

void AppAccessoryServerStart(void) {
    HAPAccessoryServerStart(accessoryConfiguration.server, &accessory);
}

//----------------------------------------------------------------------------------------------------------------------

void AccessoryServerHandleUpdatedState(HAPAccessoryServerRef* server, void* _Nullable context) {
    HAPPrecondition(server);
    HAPPrecondition(!context);

    switch (HAPAccessoryServerGetState(server)) {
        case kHAPAccessoryServerState_Idle: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Idle.");
            return;
        }
        case kHAPAccessoryServerState_Running: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Running.");
            return;
        }
        case kHAPAccessoryServerState_Stopping: {
            HAPLogInfo(&kHAPLog_Default, "Accessory Server State did update: Stopping.");
            return;
        }
    }
    HAPFatalError();
}

const HAPAccessory* AppGetAccessoryInfo() {
    return &accessory;
}

void AppInitialize(
        HAPAccessoryServerOptions* hapAccessoryServerOptions HAP_UNUSED,
        HAPPlatform* hapPlatform HAP_UNUSED,
        HAPAccessoryServerCallbacks* hapAccessoryServerCallbacks HAP_UNUSED) {
    /*no-op*/
}

void AppDeinitialize() {
    /*no-op*/
}
