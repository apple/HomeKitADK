// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "AccessoryServer" };

/**
 * Completes accessory server shutdown after HAPAccessoryServerStop.
 *
 * @param      server_              Accessory server.
 */
static void CompleteShutdown(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    // Reset Pair Setup procedure state.
    HAPAssert(!server->pairSetup.sessionThatIsCurrentlyPairing);
    HAPAccessorySetupInfoHandleAccessoryServerStop(server_);

    // Reset state.
    server->primaryAccessory = NULL;
    server->ip.bridgedAccessories = NULL;

    // Check that everything is cleaned up.
    HAPAssert(!server->ip.discoverableService);

    // Shutdown complete.
    HAPLogInfo(&logObject, "Accessory server shutdown completed.");
    server->state = kHAPAccessoryServerState_Idle;
    HAPAssert(server->callbacks.handleUpdatedState);
    server->callbacks.handleUpdatedState(server_, server->context);
}

static void CallbackTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(timer == server->callbackTimer);
    server->callbackTimer = 0;

    HAPAccessorySetupInfoHandleAccessoryServerStateUpdate(server_);

    // Complete shutdown if accessory server has been stopped using a server engine.
    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->stop &&
            HAPAccessoryServerGetState(server_) == kHAPAccessoryServerState_Idle) {
            CompleteShutdown(server_);
            return;
        }
    }

    // Invoke handleUpdatedState callback.
    HAPAssert(server->callbacks.handleUpdatedState);
    server->callbacks.handleUpdatedState(server_, server->context);
}

void HAPAccessoryServerDelegateScheduleHandleUpdatedState(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    if (server->callbackTimer) {
        return;
    }
    err = HAPPlatformTimerRegister(&server->callbackTimer, 0, CallbackTimerExpired, server_);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Not enough resources to allocate accessory server callback timer.");
        HAPFatalError();
    }
}

void HAPAccessoryServerCreate(
        HAPAccessoryServerRef* server_,
        const HAPAccessoryServerOptions* options,
        const HAPPlatform* platform,
        const HAPAccessoryServerCallbacks* callbacks,
        void* _Nullable context) {
    HAPPrecondition(HAPPlatformGetCompatibilityVersion() == HAP_PLATFORM_COMPATIBILITY_VERSION);
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(options);
    HAPPrecondition(platform);
    HAPPrecondition(callbacks);

    if (HAP_LOG_LEVEL >= 1) {
        char stringBuilderBytes[1024];
        HAPStringBuilderRef stringBuilder;
        HAPStringBuilderCreate(&stringBuilder, stringBuilderBytes, sizeof stringBuilderBytes);
        HAPStringBuilderAppend(&stringBuilder, "Version information:");
        HAPStringBuilderAppend(&stringBuilder, "\nlibhap: %s", HAPGetIdentification());
        HAPStringBuilderAppend(
                &stringBuilder,
                "\n  - Version: %s (%s) - compatibility version %lu",
                HAPGetVersion(),
                HAPGetBuild(),
                (unsigned long) HAPGetCompatibilityVersion());
        HAPStringBuilderAppend(&stringBuilder, "\nUsing platform: %s", HAPPlatformGetIdentification());
        HAPStringBuilderAppend(
                &stringBuilder,
                "\n  - Version: %s (%s) - compatibility version %lu",
                HAPPlatformGetVersion(),
                HAPPlatformGetBuild(),
                (unsigned long) HAPPlatformGetCompatibilityVersion());
        HAPStringBuilderAppend(&stringBuilder, "\n  - Available features:");
        if (platform->keyValueStore) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Key-Value store");
        }
        if (platform->accessorySetup) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Accessory setup manager");
        }
        if (platform->setupDisplay) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Accessory setup display");
        }
        if (platform->setupNFC) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Accessory setup programmable NFC tag");
        }
        if (platform->ip.serviceDiscovery) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Service discovery");
        }
        if (platform->ble.blePeripheralManager) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - BLE peripheral manager");
        }
        if (platform->authentication.mfiHWAuth) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Apple Authentication Coprocessor provider");
        }
        if (platform->authentication.mfiTokenAuth) {
            HAPStringBuilderAppend(&stringBuilder, "\n    - Software Token provider");
        }

        if (HAPStringBuilderDidOverflow(&stringBuilder)) {
            HAPLogError(&logObject, "Version information truncated.");
        }
        HAPLog(&logObject, "%s", HAPStringBuilderGetString(&stringBuilder));
    }

    HAPLogDebug(&logObject, "Storage configuration: server = %lu", (unsigned long) sizeof *server);

    HAPRawBufferZero(server, sizeof *server);

    // Copy generic options.
    HAPPrecondition(options->maxPairings >= kHAPPairingStorage_MinElements);
    server->maxPairings = options->maxPairings;

    // Copy platform.
    HAPAssert(sizeof *platform == sizeof server->platform);
    HAPRawBufferCopyBytes(&server->platform, platform, sizeof server->platform);
    HAPPrecondition(server->platform.keyValueStore);
    HAPPrecondition(server->platform.accessorySetup);
    HAPMFiHWAuthCreate(&server->mfi, server->platform.authentication.mfiHWAuth);

    // Deprecation check for accessory setup.
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
    HAPPlatformAccessorySetupCapabilities accessorySetupCapabilities =
            HAPPlatformAccessorySetupGetCapabilities(server->platform.accessorySetup);
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP
    if (accessorySetupCapabilities.supportsDisplay) {
        HAPLogError(
                &logObject,
                "HAPPlatformAccessorySetupGetCapabilities is deprecated. "
                "Return false and use HAPPlatformAccessorySetupDisplay instead.");
    }
    if (accessorySetupCapabilities.supportsProgrammableNFC) {
        HAPLogError(
                &logObject,
                "HAPPlatformAccessorySetupGetCapabilities is deprecated. "
                "Return false and use HAPPlatformAccessorySetupNFC instead.");
    }
    if (server->platform.setupDisplay || server->platform.setupNFC) {
        HAPPrecondition(!accessorySetupCapabilities.supportsDisplay);
        HAPPrecondition(!accessorySetupCapabilities.supportsProgrammableNFC);
    }

    // Copy callbacks.
    HAPPrecondition(callbacks->handleUpdatedState);
    HAPAssert(sizeof *callbacks == sizeof server->callbacks);
    HAPRawBufferCopyBytes(&server->callbacks, callbacks, sizeof server->callbacks);

    // Deprecation check for transports.
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
    if (options->ip.available) {
        HAPLogFault(
                &logObject,
                "HAPAccessoryServerOptions must no longer set ip.available. "
                "Set ip.transport to &kHAPAccessoryServerTransport_IP instead.");
        HAPFatalError();
    }
    if (options->ble.available) {
        HAPLogFault(
                &logObject,
                "HAPAccessoryServerOptions must no longer set ble.available. "
                "Set ble.transport to &kHAPAccessoryServerTransport_BLE instead.");
        HAPFatalError();
    }
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP

    // One transport must be supported.
    HAPPrecondition(options->ip.transport || options->ble.transport);

    // Copy IP parameters.
    server->transports.ip = options->ip.transport;
    if (server->transports.ip) {
        HAPNonnull(server->transports.ip)->create(server_, options);
    } else {
        HAPRawBufferZero(&server->platform.ip, sizeof server->platform.ip);
    }

    // Copy Bluetooth LE parameters.
    server->transports.ble = options->ble.transport;
    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->create(server_, options);
    } else {
        HAPRawBufferZero(&server->platform.ble, sizeof server->platform.ble);
    }

    // Copy client context.
    server->context = context;

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->init) {
            serverEngine->init(server_);
        }
    }
}

void HAPAccessoryServerRelease(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    HAPAccessoryServerStop(server_);

    if (server->callbackTimer) {
        HAPPlatformTimerDeregister(server->callbackTimer);
        server->callbackTimer = 0;
    }

    if (server->transports.ble) {
        HAPAssert(server->platform.ble.blePeripheralManager);
        HAPNonnull(server->transports.ble)->peripheralManager.release(server_);
    }

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->deinit) {
            err = serverEngine->deinit(server_);
            if (err) {
                HAPFatalError();
            }
        }
    }

    if (server->transports.ble) {
        if (server->ble.adv.fast_timer) {
            HAPPlatformTimerDeregister(server->ble.adv.fast_timer);
            server->ble.adv.fast_timer = 0;
        }
        if (server->ble.adv.timer) {
            HAPPlatformTimerDeregister(server->ble.adv.timer);
            server->ble.adv.timer = 0;
        }
    }

    HAPMFiHWAuthRelease(&server->mfi);

    if (server->transports.ip) {
        HAPNonnull(server->transports.ip)->serverEngine.uninstall();
    }

    HAPRawBufferZero(server_, sizeof *server_);
}

HAP_RESULT_USE_CHECK
HAPAccessoryServerState HAPAccessoryServerGetState(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->get_state) {
            return serverEngine->get_state(server_);
        }
    }

    return server->state;
}

HAP_RESULT_USE_CHECK
void* _Nullable HAPAccessoryServerGetClientContext(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    return server->context;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * Parses a version string where each element is capped at 2^32-1.
 *
 * @param      version              Version string.
 * @param[out] major                Major version number.
 * @param[out] minor                Minor version number.
 * @param[out] revision             Revision version number.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the version string is malformed.
 */
HAP_RESULT_USE_CHECK
static HAPError ParseVersionString(const char* version, uint32_t* major, uint32_t* minor, uint32_t* revision) {
    HAPPrecondition(version);
    HAPPrecondition(major);
    HAPPrecondition(minor);
    HAPPrecondition(revision);

    *major = 0;
    *minor = 0;
    *revision = 0;

    // Read numbers.
    uint32_t* numbers[3] = { major, minor, revision };
    size_t i = 0;
    bool first = true;
    for (const char* c = version; *c; c++) {
        if (!first && *c == '.') {
            // Advance to next number.
            if (i >= 2) {
                HAPLog(&logObject, "Invalid version string: %s.", version);
                return kHAPError_InvalidData;
            }
            i++;
            first = true;
            continue;
        }
        first = false;

        // Add digit.
        if (*c < '0' || *c > '9') {
            HAPLog(&logObject, "Invalid version string: %s.", version);
            return kHAPError_InvalidData;
        }
        if (*numbers[i] > UINT32_MAX / 10) {
            HAPLog(&logObject, "Invalid version string: %s.", version);
            return kHAPError_InvalidData;
        }
        (*numbers[i]) *= 10;
        if (*numbers[i] > UINT32_MAX - (uint32_t)(*c - '0')) {
            HAPLog(&logObject, "Invalid version string: %s.", version);
            return kHAPError_InvalidData;
        }
        (*numbers[i]) += (uint32_t)(*c - '0');
    }
    if (first) {
        HAPLog(&logObject, "Invalid version string: %s.", version);
        return kHAPError_InvalidData;
    }

    return kHAPError_None;
}

void HAPAccessoryServerLoadLTSK(HAPPlatformKeyValueStoreRef keyValueStore, HAPAccessoryServerLongTermSecretKey* ltsk) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(ltsk);

    HAPError err;

    // An attacker who gains application processor code execution privileges can:
    // - Control any accessory functionality.
    // - List, add, remove, and modify HAP pairings.
    // - Provide a service to sign arbitrary messages with the accessory LTSK.
    // These assumptions remain valid even when a separate Trusted Execution Environment (TEE) is present,
    // because as of HomeKit Accessory Protocol R14, HAP only defines transport security.
    // Augmenting the HAP protocol with true end-to-end security for HAP pairings would require a protocol change.
    //
    // The raw accessory LTSK could theoretically be stored in a TEE,
    // but given the user impact when an attacker takes control of the application processor
    // there does not seem to be a realistic threat that can be mitigated if this would be done.
    // The attacker could still set up a service to sign arbitrary messages with the accessory LTSK
    // when the accessory LTSK is stored in a TEE, and could use this service to impersonate the accessory.
    //
    // The only security that can currently be provided is to store all secrets in secure memory
    // so that they cannot easily be extracted at rest (without having code execution privileges or RAM access).
    // It is left up to the platform implementation to store the HAPPlatformKeyValueStore content securely.
    //
    // Note: If this mechanism is ever replaced to redirect to a TEE for the LTSK,
    // an upgrade path must be specified for the following scenarios:
    // - LTSK was stored in HAPPlatformKeyValueStore, and needs to be migrated into a TEE.
    // - HAP protocol gets extended with real TEE support, and LTSK needs to be migrated into a new TEE.

    bool found;
    size_t numBytes;
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_LTSK,
            ltsk->bytes,
            sizeof ltsk->bytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPLogError(&logObject, "Reading LTSK failed.");
        HAPFatalError();
    }
    if (!found) {
        // Reset pairings.
        err = HAPPlatformKeyValueStorePurgeDomain(keyValueStore, kHAPKeyValueStoreDomain_Pairings);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLogError(&logObject, "Purge of pairing domain failed.");
            HAPFatalError();
        }

        // Generate new LTSK.
        HAPPlatformRandomNumberFill(ltsk->bytes, sizeof ltsk->bytes);
        HAPLogSensitiveBufferInfo(&logObject, ltsk->bytes, sizeof ltsk->bytes, "Generated new LTSK.");

        // Store new LTSK.
        err = HAPPlatformKeyValueStoreSet(
                keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_LTSK,
                ltsk->bytes,
                sizeof ltsk->bytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLogError(&logObject, "Storing LTSK failed.");
            HAPFatalError();
        }
    } else if (numBytes != sizeof ltsk->bytes) {
        HAPLogError(&logObject, "Corrupted LTSK in Key-Value Store.");
        HAPFatalError();
    }
}

/**
 * Prepares starting the accessory server.
 *
 * @param      server_              Accessory server.
 * @param      primaryAccessory     Primary accessory to host.
 * @param      bridgedAccessories   NULL-terminated array of bridged accessories for a bridge accessory. NULL otherwise.
 */
static void HAPAccessoryServerPrepareStart(
        HAPAccessoryServerRef* server_,
        const HAPAccessory* primaryAccessory,
        const HAPAccessory* _Nullable const* _Nullable bridgedAccessories) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->state == kHAPAccessoryServerState_Idle);
    HAPPrecondition(!server->primaryAccessory);
    HAPPrecondition(!server->ip.bridgedAccessories);
    HAPPrecondition(primaryAccessory);

    HAPError err;

    HAPLogInfo(&logObject, "Accessory server starting.");
    server->state = kHAPAccessoryServerState_Running;
    HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);

    // Reset state.
    if (server->transports.ip) {
        HAPNonnull(server->transports.ip)->prepareStart(server_);
    }
    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->prepareStart(server_);
    }

    // Firmware version check.
    {
        // Read firmware version.
        HAPAssert(primaryAccessory->firmwareVersion);
        uint32_t major;
        uint32_t minor;
        uint32_t revision;
        err = ParseVersionString(primaryAccessory->firmwareVersion, &major, &minor, &revision);
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            HAPFatalError();
        }
        HAPLogInfo(
                &logObject,
                "Firmware version: %lu.%lu.%lu",
                (unsigned long) major,
                (unsigned long) minor,
                (unsigned long) revision);

        // Check for configuration change.
        HAPPrecondition(server->platform.keyValueStore);
        uint8_t bytes[3 * 4];
        bool found;
        size_t numBytes;
        err = HAPPlatformKeyValueStoreGet(
                server->platform.keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_FirmwareVersion,
                bytes,
                sizeof bytes,
                &numBytes,
                &found);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }
        bool saveVersion = false;
        if (found) {
            if (numBytes != sizeof bytes) {
                HAPLogError(
                        &logObject,
                        "Key-value store corrupted - unexpected length for firmware revision: %lu.",
                        (unsigned long) numBytes);
                HAPFatalError();
            }
            uint32_t previousMajor = HAPReadLittleUInt32(&bytes[0]);
            uint32_t previousMinor = HAPReadLittleUInt32(&bytes[4]);
            uint32_t previousRevision = HAPReadLittleUInt32(&bytes[8]);
            if (major != previousMajor || minor != previousMinor || revision != previousRevision) {
                if (major < previousMajor || (major == previousMajor && minor < previousMinor) ||
                    (major == previousMajor && minor == previousMinor && revision < previousRevision)) {
                    HAPLogError(
                            &logObject,
                            "[%lu.%lu.%lu > %lu.%lu.%lu] Firmware must not be downgraded! Not starting "
                            "HAPAccessoryServer.",
                            (unsigned long) previousMajor,
                            (unsigned long) previousMinor,
                            (unsigned long) previousRevision,
                            (unsigned long) major,
                            (unsigned long) minor,
                            (unsigned long) revision);
                    server->state = kHAPAccessoryServerState_Idle;
                    return;
                }

                HAPLogInfo(
                        &logObject,
                        "[%lu.%lu.%lu > %lu.%lu.%lu] Performing post firmware update tasks.",
                        (unsigned long) previousMajor,
                        (unsigned long) previousMinor,
                        (unsigned long) previousRevision,
                        (unsigned long) major,
                        (unsigned long) minor,
                        (unsigned long) revision);
                err = HAPHandleFirmwareUpdate(server_);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    HAPFatalError();
                }
                saveVersion = true;
            }
        } else {
            HAPLogInfo(
                    &logObject,
                    "[%lu.%lu.%lu] Storing initial firmware version.",
                    (unsigned long) major,
                    (unsigned long) minor,
                    (unsigned long) revision);
            saveVersion = true;
        }
        if (saveVersion) {
            HAPWriteLittleUInt32(&bytes[0], major);
            HAPWriteLittleUInt32(&bytes[4], minor);
            HAPWriteLittleUInt32(&bytes[8], revision);
            err = HAPPlatformKeyValueStoreSet(
                    server->platform.keyValueStore,
                    kHAPKeyValueStoreDomain_Configuration,
                    kHAPKeyValueStoreKey_Configuration_FirmwareVersion,
                    bytes,
                    sizeof bytes);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                HAPFatalError();
            }
        }
    }

    // Register accessory.
    HAPLogDebug(&logObject, "Registering accessories.");
    server->primaryAccessory = primaryAccessory;
    server->ip.bridgedAccessories = bridgedAccessories;

    // Load LTSK.
    HAPLogDebug(&logObject, "Loading accessory identity.");
    HAPAccessoryServerLoadLTSK(server->platform.keyValueStore, &server->identity.ed_LTSK);
    HAP_ed25519_public_key(server->identity.ed_LTPK, server->identity.ed_LTSK.bytes);

    // Cleanup pairings.
    err = HAPAccessoryServerCleanupPairings(server_);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPLogError(&logObject, "Cleanup pairings failed.");
        HAPFatalError();
    }

    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->start(server_);
    }

    // Update setup payload.
    HAPAccessorySetupInfoHandleAccessoryServerStart(server_);

    // Update advertising state.
    HAPAccessoryServerUpdateAdvertisingData(server_);
}

void HAPAccessoryServerStart(HAPAccessoryServerRef* server_, const HAPAccessory* accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(accessory);

    HAPLogDebug(
            &logObject,
            "Checking accessory definition. "
            "If this crashes, verify that service and characteristic lists are properly NULL-terminated.");
    HAPPrecondition(HAPRegularAccessoryIsValid(server_, accessory));
    HAPLogDebug(&logObject, "Accessory definition ok.");

    // Check Bluetooth LE requirements.
    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->validateAccessory(accessory);
    }

    // Start accessory server.
    HAPAccessoryServerPrepareStart(server_, accessory, /* bridgedAccessories: */ NULL);
    if (server->state != kHAPAccessoryServerState_Running) {
        HAPAssert(server->state == kHAPAccessoryServerState_Idle);
        return;
    }

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->start) {
            serverEngine->start(server_);
        }
    }
}

void HAPAccessoryServerStartBridge(
        HAPAccessoryServerRef* server_,
        const HAPAccessory* bridgeAccessory,
        const HAPAccessory* _Nullable const* _Nullable bridgedAccessories,
        bool configurationChanged) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(bridgeAccessory);

    HAPError err;

    HAPLogDebug(
            &logObject,
            "Checking accessory definition. "
            "If this crashes, verify that accessory, service and characteristic lists are properly NULL-terminated.");
    HAPPrecondition(HAPRegularAccessoryIsValid(server_, bridgeAccessory));
    if (bridgedAccessories) {
        size_t i;
        for (i = 0; bridgedAccessories[i]; i++) {
            HAPPrecondition(HAPBridgedAccessoryIsValid(bridgedAccessories[i]));
        }
        HAPPrecondition(i <= kHAPAccessoryServerMaxBridgedAccessories);
    }
    HAPLogDebug(&logObject, "Accessory definition ok.");

    HAPAccessoryServerPrepareStart(server_, bridgeAccessory, bridgedAccessories);
    if (server->state != kHAPAccessoryServerState_Running) {
        HAPAssert(server->state == kHAPAccessoryServerState_Idle);
        return;
    }

    // Increment configuration number if necessary.
    if (configurationChanged) {
        HAPLogInfo(&logObject, "Configuration changed. Incrementing CN.");
        err = HAPAccessoryServerIncrementCN(server->platform.keyValueStore);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }
    }

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->start) {
            serverEngine->start(server_);
        }
    }
}

void HAPAccessoryServerStop(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    if (server->state == kHAPAccessoryServerState_Idle) {
        return;
    }
    if (server->state != kHAPAccessoryServerState_Stopping) {
        HAPAssert(server->state == kHAPAccessoryServerState_Running);
        HAPLogInfo(&logObject, "Accessory server shutting down.");
        server->state = kHAPAccessoryServerState_Stopping;
        if (!server->transports.ip || !HAPNonnull(server->transports.ip)->serverEngine.get()) {
            server->callbacks.handleUpdatedState(server_, server->context);
        }
    }

    // Stop advertising.
    if (server->transports.ble) {
        HAPAccessoryServerUpdateAdvertisingData(server_);
    }

    if (server->transports.ip) {
        HAPNonnull(server->transports.ip)->prepareStop(server_);
    }

    if (server->transports.ble) {
        bool didStop;
        HAPNonnull(server->transports.ble)->tryStop(server_, &didStop);
        if (!didStop) {
            return;
        }
    }

    // Inform server engine.
    // Server engine will complete the shutdown process.
    // - _serverEngine->stop
    // - ...
    // - HAPAccessoryServerDelegateScheduleHandleUpdatedState => kHAPAccessoryServerState_Idle.
    // - CompleteShutdown.
    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->stop) {
            err = serverEngine->stop(server_);
            if (err) {
                HAPFatalError();
            }
            return;
        }
    }

    // Complete shutdown.
    CompleteShutdown(server_);
}

void HAPAccessoryServerUpdateAdvertisingData(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->updateAdvertisingData(server_);
    }
}

typedef struct {
    bool exists; /**< Pairing found. */
} PairingExistsEnumerateContext;

HAP_RESULT_USE_CHECK
static HAPError PairingExistsEnumerateCallback(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key HAP_UNUSED,
        bool* shouldContinue) {
    HAPPrecondition(context);
    PairingExistsEnumerateContext* arguments = context;
    HAPPrecondition(keyValueStore);
    HAPPrecondition(domain == kHAPKeyValueStoreDomain_Pairings);
    HAPPrecondition(shouldContinue);

    arguments->exists = true;
    *shouldContinue = false;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
bool HAPAccessoryServerIsPaired(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    const HAPAccessoryServer* server = (const HAPAccessoryServer*) server_;

    HAPError err;

    // Enumerate pairings.
    PairingExistsEnumerateContext context = { .exists = false };
    err = HAPPlatformKeyValueStoreEnumerate(
            server->platform.keyValueStore, kHAPKeyValueStoreDomain_Pairings, PairingExistsEnumerateCallback, &context);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return false;
    }

    return context.exists;
}

HAP_DEPRECATED_MSG(
        "For displays: See HAPPlatformAccessorySetupDisplay. For NFC: Use HAPAccessoryServerEnterNFCPairingMode "
        "instead.")
void HAPAccessoryServerEnterPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.accessorySetup);
    HAPPrecondition(!server->platform.setupDisplay);
    HAPPrecondition(!server->platform.setupNFC);

    HAPAccessorySetupInfoEnterLegacyPairingMode(server_);
}

void HAPAccessoryServerRefreshSetupPayload(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupDisplay);

    HAPAccessorySetupInfoRefreshSetupPayload(server_);
}

void HAPAccessoryServerEnterNFCPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupNFC);

    HAPAccessorySetupInfoEnterNFCPairingMode(server_);
}

void HAPAccessoryServerExitNFCPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupNFC);

    HAPAccessorySetupInfoExitNFCPairingMode(server_);
}

HAP_RESULT_USE_CHECK
bool HAPAccessoryServerSupportsMFiHWAuth(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    return HAPMFiHWAuthIsAvailable(&server->mfi);
}

HAP_RESULT_USE_CHECK
uint8_t HAPAccessoryServerGetPairingFeatureFlags(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Table 5-15 Pairing Feature Flags

    // Check if Apple Authentication Coprocessor is supported.
    bool supportsAppleAuthenticationCoprocessor = HAPAccessoryServerSupportsMFiHWAuth(server_);

    // Check if Software Authentication is supported.
    bool supportsSoftwareAuthentication = false;
    if (server->platform.authentication.mfiTokenAuth) {
        err = HAPPlatformMFiTokenAuthLoad(
                HAPNonnull(server->platform.authentication.mfiTokenAuth),
                &supportsSoftwareAuthentication,
                NULL,
                NULL,
                0,
                NULL);
        if (err) {
            HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
            HAPLogError(&logObject, "HAPPlatformMFiTokenAuthLoad failed: %u.", err);
            HAPFatalError();
        }
    }

    // Serialize response.
    uint8_t pairingFeatureFlags = 0;
    if (supportsAppleAuthenticationCoprocessor) {
        pairingFeatureFlags |= kHAPCharacteristicValue_PairingFeatures_SupportsAppleAuthenticationCoprocessor;
    }
    if (supportsSoftwareAuthentication) {
        pairingFeatureFlags |= kHAPCharacteristicValue_PairingFeatures_SupportsSoftwareAuthentication;
    }
    return pairingFeatureFlags;
}

/**
 * Status flags.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 6-8 Bonjour TXT Status Flags
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.4.2.1.2 Manufacturer Data
 */
HAP_ENUM_BEGIN(uint8_t, HAPAccessoryServerStatusFlags) {
    /** Accessory has not been paired with any controllers. */
    kHAPAccessoryServerStatusFlags_NotPaired = 1 << 0,

    /**
     * A problem has been detected on the accessory.
     *
     * - Used by accessories supporting HAP over IP (Ethernet / Wi-Fi) only.
     */
    kHAPAccessoryServerStatusFlags_ProblemDetected = 1 << 2
} HAP_ENUM_END(uint8_t, HAPAccessoryServerStatusFlags);

HAP_RESULT_USE_CHECK
uint8_t HAPAccessoryServerGetStatusFlags(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);

    uint8_t statusFlags = 0;
    if (!HAPAccessoryServerIsPaired(server_)) {
        statusFlags |= kHAPAccessoryServerStatusFlags_NotPaired;
    }
    return statusFlags;
}

typedef struct {
    bool hasPairings : 1;
    bool adminFound : 1;
} FindAdminPairingEnumerateContext;

HAP_RESULT_USE_CHECK
static HAPError FindAdminPairingEnumerateCallback(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue) {
    FindAdminPairingEnumerateContext* arguments = context;
    HAPPrecondition(arguments);
    HAPPrecondition(!arguments->adminFound);
    HAPPrecondition(domain == kHAPKeyValueStoreDomain_Pairings);
    HAPPrecondition(shouldContinue);

    HAPError err;

    // Load pairing.
    bool found;
    size_t numBytes;
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(keyValueStore, domain, key, pairingBytes, sizeof pairingBytes, &numBytes, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    HAPAssert(found);
    if (numBytes != sizeof pairingBytes) {
        HAPLog(&logObject, "Invalid pairing 0x%02X size %lu.", key, (unsigned long) numBytes);
        return kHAPError_Unknown;
    }
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPAssert(sizeof pairing.identifier.bytes == 36);
    HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
    pairing.numIdentifierBytes = pairingBytes[36];
    HAPAssert(sizeof pairing.publicKey.value == 32);
    HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
    pairing.permissions = pairingBytes[69];

    arguments->hasPairings = true;

    // Check if admin found.
    if (pairing.permissions & 0x01) {
        arguments->adminFound = true;
        *shouldContinue = false;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerCleanupPairings(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    HAPLogDebug(&logObject, "Checking if admin pairing exists.");

    // Look for admin pairing.
    FindAdminPairingEnumerateContext context = { .adminFound = false };
    err = HAPPlatformKeyValueStoreEnumerate(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Pairings,
            FindAdminPairingEnumerateCallback,
            &context);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // If there is no admin, delete all pairings.
    if (!context.adminFound) {
        if (context.hasPairings) {
            // Remove all pairings.
            HAPLogInfo(&logObject, "No admin pairing found. Removing all pairings.");
            HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
            err = HAPPlatformKeyValueStorePurgeDomain(server->platform.keyValueStore, kHAPKeyValueStoreDomain_Pairings);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
        }

        // Purge Pair Resume cache.
        if (server->transports.ble) {
            HAPRawBufferZero(
                    server->ble.storage->sessionCacheElements,
                    server->ble.storage->numSessionCacheElements * sizeof *server->ble.storage->sessionCacheElements);
        }

        // Purge broadcast encryption key and advertising identifier.
        // See HomeKit Certification Test Cases R7.2
        // Test Case TCB052
        err = HAPPlatformKeyValueStoreRemove(
                server->platform.keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_BLEBroadcastParameters);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerGetCN(HAPPlatformKeyValueStoreRef keyValueStore, uint16_t* cn) {
    HAPPrecondition(keyValueStore);
    HAPPrecondition(cn);

    HAPError err;

    // Prior to HomeKit Accessory Protocol Specification R12:
    // - CN was 32-bit for IP:  1 - 4294967295, overflow to 1
    // - CN was  8-bit for BLE: 1 - 255, overflow to 1
    //
    // Since HomeKit Accessory Protocol Specification R12:
    // - CN is 16-bit for IP:  1 - 65535, overflow to 1
    // - CN is  8-bit for BLE: 1 - 255, overflow to 1
    // - CN is 16-bit for HAP-Info-Response: 1 - 65535, overflow to 1
    //
    // To avoid breaking compatibility with legacy versions, we store CN as UInt32
    // and derive the shorter CN variants from it while staying consistent w.r.t. the various overflows to 1.

    // Try to load configuration number.
    bool found;
    size_t numBytes;
    uint8_t cnBytes[sizeof(uint32_t)];
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
            cnBytes,
            sizeof cnBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        // Initialize configuration number.
        HAPWriteLittleUInt32(cnBytes, 1);

        // Store new configuration number.
        err = HAPPlatformKeyValueStoreSet(
                keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
                cnBytes,
                sizeof cnBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    } else if (numBytes != sizeof cnBytes) {
        HAPLog(&logObject, "Invalid configuration number length (%zu).", numBytes);
        return kHAPError_Unknown;
    }

    // Downscale to UInt16.
    uint32_t cn32 = HAPReadLittleUInt32(cnBytes);
    *cn = (uint16_t)((cn32 - 1) % UINT16_MAX + 1);

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPAccessoryServerIncrementCN(HAPPlatformKeyValueStoreRef keyValueStore) {
    HAPPrecondition(keyValueStore);

    HAPError err;

    // Get CN.
    bool found;
    size_t numBytes;
    uint8_t cnBytes[sizeof(uint32_t)];
    err = HAPPlatformKeyValueStoreGet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
            cnBytes,
            sizeof cnBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        // Initialize configuration number.
        HAPWriteLittleUInt32(cnBytes, 1);
    }

    // Increment CN.
    {
        uint32_t cn32 = HAPReadLittleUInt32(cnBytes);
        if (cn32 == UINT32_MAX) {
            cn32 = 1;
        } else {
            cn32++;
        }
        HAPWriteLittleUInt32(cnBytes, cn32);
        HAPLogInfo(&logObject, "Updated CN: %lu.", (unsigned long) cn32);
    }

    // Save CN.
    err = HAPPlatformKeyValueStoreSet(
            keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_ConfigurationNumber,
            cnBytes,
            sizeof cnBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}

void HAPAccessoryServerRaiseEvent(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    HAPError err;

    HAPLogCharacteristicDebug(&logObject, characteristic, service, accessory, "Marking characteristic as modified.");

    if (server->transports.ble) {
        err = HAPNonnull(server->transports.ble)->didRaiseEvent(server_, characteristic, service, accessory, NULL);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }
    }

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->raise_event) {
            err = serverEngine->raise_event(server_, characteristic, service, accessory);
            if (err) {
                HAPFatalError();
            }
        }
    }
}

void HAPAccessoryServerRaiseEventOnSession(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory,
        HAPSessionRef* session) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);
    HAPPrecondition(session);

    HAPError err;

    if (server->transports.ble) {
        err = HAPNonnull(server->transports.ble)->didRaiseEvent(server_, characteristic, service, accessory, session);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }
    }

    if (server->transports.ip) {
        const HAPAccessoryServerServerEngine* _Nullable serverEngine =
                HAPNonnull(server->transports.ip)->serverEngine.get();
        if (serverEngine && serverEngine->raise_event_on_session) {
            err = serverEngine->raise_event_on_session(server_, characteristic, service, accessory, session);
            if (err) {
                HAPFatalError();
            }
        }
    }
}

void HAPAccessoryServerHandleSubscribe(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    switch (((const HAPBaseCharacteristic*) characteristic)->format) {
        case kHAPCharacteristicFormat_Data: {
            HAPDataCharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPDataCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Bool: {
            HAPBoolCharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPBoolCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt8: {
            HAPUInt8CharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt8CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt16: {
            HAPUInt16CharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt16CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt32: {
            HAPUInt32CharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt32CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt64: {
            HAPUInt64CharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt64CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Int: {
            HAPIntCharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPIntCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Float: {
            HAPFloatCharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPFloatCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_String: {
            HAPStringCharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPStringCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_TLV8: {
            HAPTLV8CharacteristicHandleSubscribe(
                    HAPNonnull(session->server),
                    &(const HAPTLV8CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
    }
}

void HAPAccessoryServerHandleUnsubscribe(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(characteristic);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    switch (((const HAPBaseCharacteristic*) characteristic)->format) {
        case kHAPCharacteristicFormat_Data: {
            HAPDataCharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPDataCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Bool: {
            HAPBoolCharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPBoolCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt8: {
            HAPUInt8CharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt8CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt16: {
            HAPUInt16CharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt16CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt32: {
            HAPUInt32CharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt32CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_UInt64: {
            HAPUInt64CharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPUInt64CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Int: {
            HAPIntCharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPIntCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_Float: {
            HAPFloatCharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPFloatCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_String: {
            HAPStringCharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPStringCharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
        case kHAPCharacteristicFormat_TLV8: {
            HAPTLV8CharacteristicHandleUnsubscribe(
                    HAPNonnull(session->server),
                    &(const HAPTLV8CharacteristicSubscriptionRequest) {
                            .transportType = session->transportType,
                            .session = session_,
                            .characteristic = characteristic,
                            .service = service,
                            .accessory = accessory,
                    },
                    HAPAccessoryServerGetClientContext(HAPNonnull(session->server)));
        } break;
    }
}

HAP_RESULT_USE_CHECK
bool HAPAccessoryServerSupportsService(
        HAPAccessoryServerRef* server_,
        HAPTransportType transportType,
        const HAPService* service) {
    HAPPrecondition(server_);
    HAPPrecondition(transportType == kHAPTransportType_IP || transportType == kHAPTransportType_BLE);
    HAPPrecondition(service);

    if (transportType == kHAPTransportType_IP && HAPUUIDAreEqual(service->serviceType, &kHAPServiceType_Pairing)) {
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
size_t HAPAccessoryServerGetNumServiceInstances(HAPAccessoryServerRef* server_, const HAPUUID* serviceType) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(serviceType);

    HAPServiceTypeIndex serviceTypeIndex = 0;

    HAPPrecondition(server->primaryAccessory);
    {
        const HAPAccessory* acc = server->primaryAccessory;
        if (acc->services) {
            for (size_t i = 0; acc->services[i]; i++) {
                const HAPService* svc = acc->services[i];
                if (HAPUUIDAreEqual(svc->serviceType, serviceType)) {
                    serviceTypeIndex++;
                    HAPAssert(serviceTypeIndex); // No overflow.
                }
            }
        }
    }
    if (server->ip.bridgedAccessories) {
        for (size_t j = 0; server->ip.bridgedAccessories[j]; j++) {
            const HAPAccessory* acc = server->ip.bridgedAccessories[j];
            if (acc->services) {
                for (size_t i = 0; acc->services[i]; i++) {
                    const HAPService* svc = acc->services[i];
                    if (HAPUUIDAreEqual(svc->serviceType, serviceType)) {
                        serviceTypeIndex++;
                        HAPAssert(serviceTypeIndex); // No overflow.
                    }
                }
            }
        }
    }

    return serviceTypeIndex;
}

HAP_RESULT_USE_CHECK
HAPServiceTypeIndex HAPAccessoryServerGetServiceTypeIndex(
        HAPAccessoryServerRef* server_,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    HAPServiceTypeIndex serviceTypeIndex = 0;

    HAPPrecondition(server->primaryAccessory);
    {
        const HAPAccessory* acc = server->primaryAccessory;
        if (acc->services) {
            for (size_t i = 0; acc->services[i]; i++) {
                const HAPService* svc = acc->services[i];
                if (svc == service && acc == accessory) {
                    return serviceTypeIndex;
                }
                if (HAPUUIDAreEqual(svc->serviceType, service->serviceType)) {
                    serviceTypeIndex++;
                    HAPAssert(serviceTypeIndex); // No overflow.
                }
            }
        }
    }
    if (server->ip.bridgedAccessories) {
        for (size_t j = 0; server->ip.bridgedAccessories[j]; j++) {
            const HAPAccessory* acc = server->ip.bridgedAccessories[j];
            if (acc->services) {
                for (size_t i = 0; acc->services[i]; i++) {
                    const HAPService* svc = acc->services[i];
                    if (svc == service && acc == accessory) {
                        return serviceTypeIndex;
                    }
                    if (HAPUUIDAreEqual(svc->serviceType, service->serviceType)) {
                        serviceTypeIndex++;
                        HAPAssert(serviceTypeIndex); // No overflow.
                    }
                }
            }
        }
    }

    HAPLogServiceError(&logObject, service, accessory, "Service not found in accessory server's attribute database.");
    HAPFatalError();
}

void HAPAccessoryServerGetServiceFromServiceTypeIndex(
        HAPAccessoryServerRef* server_,
        const HAPUUID* serviceType,
        HAPServiceTypeIndex serviceTypeIndex,
        const HAPService** service,
        const HAPAccessory** accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(serviceType);
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    HAPPrecondition(server->primaryAccessory);
    {
        const HAPAccessory* acc = server->primaryAccessory;
        if (acc->services) {
            for (size_t i = 0; acc->services[i]; i++) {
                const HAPService* svc = acc->services[i];
                if (HAPUUIDAreEqual(svc->serviceType, serviceType)) {
                    if (!serviceTypeIndex) {
                        *service = svc;
                        *accessory = acc;
                        return;
                    }
                    serviceTypeIndex--;
                }
            }
        }
    }
    if (server->ip.bridgedAccessories) {
        for (size_t j = 0; server->ip.bridgedAccessories[j]; j++) {
            const HAPAccessory* acc = server->ip.bridgedAccessories[j];
            if (acc->services) {
                for (size_t i = 0; acc->services[i]; i++) {
                    const HAPService* svc = acc->services[i];
                    if (HAPUUIDAreEqual(svc->serviceType, serviceType)) {
                        if (!serviceTypeIndex) {
                            *service = svc;
                            *accessory = acc;
                            return;
                        }
                        serviceTypeIndex--;
                    }
                }
            }
        }
    }

    HAPLogError(&logObject, "Service type index not found in accessory server's attribute database.");
    HAPFatalError();
}

//----------------------------------------------------------------------------------------------------------------------

void HAPAccessoryServerEnumerateConnectedSessions(
        HAPAccessoryServerRef* server_,
        HAPAccessoryServerEnumerateSessionsCallback callback,
        void* _Nullable context) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(callback);

    bool shouldContinue = true;

    if (server->transports.ble && server->ble.storage) {
        if (server->ble.storage->session && server->ble.connection.connected) {
            callback(context, server_, HAPNonnull(server->ble.storage->session), &shouldContinue);
        }
        if (!shouldContinue) {
            return;
        }
    }

    if (server->transports.ip && server->ip.storage) {
        for (size_t i = 0; shouldContinue && i < server->ip.storage->numSessions; i++) {
            HAPIPSession* ipSession = &server->ip.storage->sessions[i];
            HAPIPSessionDescriptor* session = (HAPIPSessionDescriptor*) &ipSession->descriptor;
            if (!session->server) {
                continue;
            }
            HAPAssert(session->server == server_);
            if (session->securitySession.type != kHAPIPSecuritySessionType_HAP) {
                continue;
            }
            callback(context, server_, &session->securitySession._.hap, &shouldContinue);
        }
        if (!shouldContinue) {
            return;
        }
    }
}
