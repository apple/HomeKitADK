// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "AccessorySetupInfo" };

// Use Cases:
//
// 1. Accessory that does not have a display or programmable NFC tag.
//    - No dynamic setup code is generated, and no setup payloads need to be derived.
//
// 2. Pre-R10 accessory with a display that does not have a setup ID.
//    - A dynamic setup code is generated periodically that may be displayed in text form.
//    - Setup payloads may not be derived without a setup ID, so QR codes and NFC tags don't work.
//
// 3. Accessory with a programmable NFC tag but no display.
//    - Programmable NFC tag must only be enabled in response to user interaction.
//    - NFC pairing mode expires after 5 minutes.
//    - While NFC pairing mode is not active (or while accessory is paired) special setup payloads are generated
//      to guide the user into restarting / factory resetting the accessory on iOS.
//    - A static setup code is provisioned and also affixed to the accessory.
//
// 4. Accessory with a display.
//    - The dynamic setup code needs to be refreshed periodically (every 5 minutes).
//    - During a pairing attempt the protocol does not allow changing the setup code.
//      Therefore, the 5 minutes timer is best-effort only.
//    - A new setup code is generated for each pairing attempt, even when this is more frequently than every 5 minutes.
//    - If programmable NFC is available, the same setup payload needs to be used as for the display.
//
// 5. Accessory with complex UI.
//    - Accessories may opt to keep track of the current accessory setup information in background.
//    - When a pairing attempt is registered a popup may be shown that guides the user to the setup code screen.
//    - When a pairing attempt is cancelled the UI may want to indicate that pairing failed / was successful.
//
// 6. Software Token Authentication.
//    - After a Transient Pair Setup procedure the setup code needs to be saved.
//      The next Split Pair Setup procedure will re-use the setup code from the previous pairing attempt.
//    - There is no timeout, if dynamic setup codes are used they cannot be refreshed until the next pairing attempt.
//
// 7. Legacy iOS behaviour.
//    - At start of pairing, iOS controllers first connect to the accessory and then ask for the setup code.
//    - However, if setup code is entered incorrectly, iOS first asks for the setup code and connects after entering it.
//      This makes it necessary to always have a setup code available to anticipate another pairing attempt.
//      Otherwise, the user would need to guess the next upcoming setup code.
//    - Since iOS 12, this was fixed and iOS always first connects and then asks for the setup code.
//
// Power considerations:
// - Constantly having a timer running to refresh displays has negligible energy impact.
// - Computing new SRP salts and verifiers is heavier. Therefore, it is only computed on demand.

//----------------------------------------------------------------------------------------------------------------------

static HAPPlatformAccessorySetupCapabilities GetLegacyAccessorySetupCapabilities(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.accessorySetup);

    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
    HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
    HAPPlatformAccessorySetupCapabilities accessorySetupCapabilities =
            HAPPlatformAccessorySetupGetCapabilities(server->platform.accessorySetup);
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
    HAP_DIAGNOSTIC_POP

    return accessorySetupCapabilities;
}

//----------------------------------------------------------------------------------------------------------------------

static void SynchronizeDisplayAndNFC(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.accessorySetup);
    HAPPlatformAccessorySetupCapabilities legacyCapabilities = GetLegacyAccessorySetupCapabilities(server_);

    if (!server->platform.setupDisplay && !legacyCapabilities.supportsDisplay && !server->platform.setupNFC &&
        !legacyCapabilities.supportsProgrammableNFC) {
        return;
    }

    // See HomeKit Accessory Protocol Specification R14
    // Section 4.4.2.1 Requirements

    // Derive setup payload flags.
    HAPAccessorySetupSetupPayloadFlags flags = { .isPaired = HAPAccessoryServerIsPaired(server_),
                                                 .ipSupported = (server->transports.ip != NULL),
                                                 .bleSupported = (server->transports.ble != NULL) };

    // Generate non-pairable setup payload.
    HAPSetupPayload nonPairablePayload;
    HAPAccessorySetupGetSetupPayload(
            &nonPairablePayload,
            /* setupCode: */ NULL,
            /* setupID: */ NULL,
            flags,
            server->primaryAccessory->category);

    // Fetch setup code.
    HAPSetupCode* _Nullable setupCode = NULL;
    if (server->accessorySetup.state.setupCodeIsAvailable) {
        setupCode = &server->accessorySetup.state.setupCode;
    }

    // Generate pairable setup payload if applicable.
    HAPSetupPayload pairablePayload;
    bool hasPairablePayload = false;
    if (setupCode &&
        (legacyCapabilities.supportsDisplay || legacyCapabilities.supportsProgrammableNFC ||
         server->platform.setupDisplay || (server->platform.setupNFC && server->accessorySetup.nfcPairingModeTimer))) {
        HAPSetupID setupID;
        bool hasSetupID;
        HAPPlatformAccessorySetupLoadSetupID(server->platform.accessorySetup, &hasSetupID, &setupID);

        if (hasSetupID) {
            HAPAssert(!flags.isPaired);
            hasPairablePayload = true;
            HAPAccessorySetupGetSetupPayload(
                    &pairablePayload, setupCode, &setupID, flags, server->primaryAccessory->category);
        } else {
            HAPLog(&logObject, "QR code displays / NFC require a setup ID to be provisioned.");
        }
    }

    // Update legacy API.
    if (legacyCapabilities.supportsDisplay || legacyCapabilities.supportsProgrammableNFC) {
        HAPLogError(
                &logObject,
                "HAPPlatformAccessorySetupUpdateSetupPayload is deprecated. "
                "Use HAPPlatformAccessorySetupDisplay / HAPPlatformAccessorySetupNFC instead.");

        HAP_DIAGNOSTIC_PUSH
        HAP_DIAGNOSTIC_IGNORED_CLANG("-Wdeprecated-declarations")
        HAP_DIAGNOSTIC_IGNORED_GCC("-Wdeprecated-declarations")
        HAP_DIAGNOSTIC_IGNORED_ARMCC(2570)
        HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe1444)
        HAPLogSensitiveInfo(
                &logObject,
                "Updating legacy setup payload: %s.",
                hasPairablePayload ? pairablePayload.stringValue : "NULL");
        HAPPlatformAccessorySetupUpdateSetupPayload(
                server->platform.accessorySetup, hasPairablePayload ? &pairablePayload : NULL, setupCode);
        HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe1444)
        HAP_DIAGNOSTIC_POP
    }

    // Update displays.
    if (server->platform.setupDisplay) {
        HAPLogSensitiveInfo(
                &logObject,
                "Updating display setup payload: %s.",
                hasPairablePayload ? pairablePayload.stringValue : "NULL");
        HAPPlatformAccessorySetupDisplayUpdateSetupPayload(
                HAPNonnull(server->platform.setupDisplay), hasPairablePayload ? &pairablePayload : NULL, setupCode);
    }

    // Update programmable NFC tags.
    if (server->platform.setupNFC) {
        if (server->accessorySetup.nfcPairingModeTimer) {
            HAPLogSensitiveInfo(
                    &logObject,
                    "Updating NFC setup payload: %s.",
                    hasPairablePayload ? pairablePayload.stringValue : nonPairablePayload.stringValue);
            HAPPlatformAccessorySetupNFCUpdateSetupPayload(
                    HAPNonnull(server->platform.setupNFC),
                    hasPairablePayload ? &pairablePayload : &nonPairablePayload,
                    hasPairablePayload);
        } else {
            HAPLogSensitiveInfo(&logObject, "Updating NFC setup payload: %s.", nonPairablePayload.stringValue);
            HAPPlatformAccessorySetupNFCUpdateSetupPayload(
                    HAPNonnull(server->platform.setupNFC), &nonPairablePayload, /* isPairable: */ false);
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

static void DynamicSetupInfoExpired(HAPPlatformTimerRef timer, void* _Nullable context);

static void ClearSetupInfo(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->accessorySetup.state.setupInfoIsAvailable || server->accessorySetup.state.setupCodeIsAvailable) {
        HAPLogDebug(&logObject, "Invalidating setup code.");
        HAPRawBufferZero(&server->accessorySetup.state, sizeof server->accessorySetup.state);
        if (server->accessorySetup.dynamicRefreshTimer) {
            HAPPlatformTimerDeregister(server->accessorySetup.dynamicRefreshTimer);
            server->accessorySetup.dynamicRefreshTimer = 0;
        }
    }
    HAPAssert(!server->accessorySetup.dynamicRefreshTimer);
}

static void PrepareSetupInfo(HAPAccessoryServerRef* server_, bool lockSetupInfo) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.accessorySetup);
    HAPPrecondition(!HAPAccessoryServerIsPaired(server_));
    HAPPlatformAccessorySetupCapabilities legacyCapabilities = GetLegacyAccessorySetupCapabilities(server_);

    HAPError err;

    if (server->accessorySetup.state.setupInfoIsAvailable || server->accessorySetup.state.setupCodeIsAvailable) {
        if (lockSetupInfo) {
            server->accessorySetup.state.lockSetupInfo = lockSetupInfo;

            if (server->accessorySetup.dynamicRefreshTimer) {
                HAPLogDebug(&logObject, "Locking dynamic setup code for pairing attempt.");
                HAPPlatformTimerDeregister(server->accessorySetup.dynamicRefreshTimer);
                server->accessorySetup.dynamicRefreshTimer = 0;
            }
        } else if (server->accessorySetup.state.lockSetupInfo) {
            HAPLogDebug(&logObject, "Keeping setup code locked for pairing attempt.");
        }
        return;
    }

    HAPRawBufferZero(&server->accessorySetup.state, sizeof server->accessorySetup.state);
    server->accessorySetup.state.lockSetupInfo = lockSetupInfo;

    // Get setup info.
    if (server->platform.setupDisplay || legacyCapabilities.supportsDisplay) {
        // See HomeKit Accessory Protocol Specification R14
        // Section 4.2.1.1 Generation of Setup Code
        // See HomeKit Accessory Protocol Specification R14
        // Section 5.6.2 M2: Accessory -> iOS Device - `SRP Start Response'
        HAPLogDebug(&logObject, "Generating dynamic setup code.");

        // Generate random setup code.
        HAPAccessorySetupGenerateRandomSetupCode(&server->accessorySetup.state.setupCode);
        server->accessorySetup.state.setupCodeIsAvailable = true;

        // Generation of SRP verifier is delayed until used for the first time.

        // Dynamic setup code needs to be refreshed periodically if it is allowed to change.
        if (!server->accessorySetup.state.lockSetupInfo) {
            HAPPrecondition(!server->accessorySetup.dynamicRefreshTimer);
            err = HAPPlatformTimerRegister(
                    &server->accessorySetup.dynamicRefreshTimer,
                    HAPPlatformClockGetCurrent() + kHAPAccessorySetupInfo_DynamicRefreshInterval,
                    DynamicSetupInfoExpired,
                    server_);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                HAPLogError(&logObject, "Not enough resources to allocate timer.");
                HAPFatalError();
            }
        }
    } else {
        HAPLogDebug(&logObject, "Loading static setup code.");

        // Load static setup code (only available if programmable NFC tag is supported).
        if (server->platform.setupNFC || legacyCapabilities.supportsProgrammableNFC) {
            HAPPlatformAccessorySetupLoadSetupCode(
                    server->platform.accessorySetup, &server->accessorySetup.state.setupCode);
            server->accessorySetup.state.setupCodeIsAvailable = true;
        }

        // Load static setup info.
        HAPPlatformAccessorySetupLoadSetupInfo(
                server->platform.accessorySetup, &server->accessorySetup.state.setupInfo);
        server->accessorySetup.state.setupInfoIsAvailable = true;
    }
    HAPAssert(server->accessorySetup.state.setupInfoIsAvailable || server->accessorySetup.state.setupCodeIsAvailable);

    SynchronizeDisplayAndNFC(server_);
}

static void DynamicSetupInfoExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(timer == server->accessorySetup.dynamicRefreshTimer);
    server->accessorySetup.dynamicRefreshTimer = 0;
    HAPPrecondition(!HAPAccessoryServerIsPaired(server_));
    HAPPrecondition(!server->accessorySetup.state.lockSetupInfo);
    HAPPrecondition(
            server->accessorySetup.state.setupInfoIsAvailable || server->accessorySetup.state.setupCodeIsAvailable);

    HAPLogInfo(&logObject, "Dynamic setup code expired.");
    ClearSetupInfo(server_);

    // Refresh setup code (legacy pairing mode needs explicit request to re-enter pairing mode).
    if (server->platform.setupDisplay) {
        PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
    }
}

//----------------------------------------------------------------------------------------------------------------------

HAPSetupInfo* _Nullable HAPAccessorySetupInfoGetSetupInfo(HAPAccessoryServerRef* server_, bool restorePrevious) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (restorePrevious && !server->accessorySetup.state.setupInfoIsAvailable &&
        !server->accessorySetup.state.setupCodeIsAvailable) {
        HAPLog(&logObject, "Cannot restore setup code from previous pairing attempt.");
        return NULL;
    }
    HAPPrecondition(
            server->accessorySetup.state.setupInfoIsAvailable || server->accessorySetup.state.setupCodeIsAvailable);

    // TODO Or should we keep it?
    if (!restorePrevious && server->accessorySetup.state.keepSetupInfo) {
        HAPLog(&logObject, "Discarding setup code from previous pairing attempt.");
        ClearSetupInfo(server_);
        PrepareSetupInfo(server_, /* lockSetupInfo: */ true);
        HAPAssert(!server->accessorySetup.state.keepSetupInfo);
    }

    // Generate SRP salt and derive SRP verifier for dynamic setup code if it has not yet been computed.
    if (!server->accessorySetup.state.setupInfoIsAvailable) {

        HAPLogDebug(&logObject, "Generating SRP verifier for dynamic setup code.");
        HAPPlatformRandomNumberFill(
                server->accessorySetup.state.setupInfo.salt, sizeof server->accessorySetup.state.setupInfo.salt);
        static const uint8_t srpUserName[] = "Pair-Setup";
        HAP_srp_verifier(
                server->accessorySetup.state.setupInfo.verifier,
                server->accessorySetup.state.setupInfo.salt,
                srpUserName,
                sizeof srpUserName - 1,
                (const uint8_t*) &server->accessorySetup.state.setupCode.stringValue,
                sizeof server->accessorySetup.state.setupCode.stringValue - 1);
        server->accessorySetup.state.setupInfoIsAvailable = true;
    }
    HAPAssert(server->accessorySetup.state.setupInfoIsAvailable);

    return &server->accessorySetup.state.setupInfo;
}

//----------------------------------------------------------------------------------------------------------------------

void HAPAccessorySetupInfoHandleAccessoryServerStart(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(!server->accessorySetup.dynamicRefreshTimer);

    HAPLogDebug(&logObject, "%s", __func__);

    // Start generating dynamic setup codes (legacy pairing mode needs explicit request to enter pairing mode).
    if (server->platform.setupDisplay && !HAPAccessoryServerIsPaired(server_)) {
        PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
    }
}

void HAPAccessorySetupInfoHandleAccessoryServerStop(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "%s", __func__);

    if (server->accessorySetup.dynamicRefreshTimer) {
        HAPPlatformTimerDeregister(server->accessorySetup.dynamicRefreshTimer);
        server->accessorySetup.dynamicRefreshTimer = 0;
    }
    if (server->accessorySetup.nfcPairingModeTimer) {
        HAPPlatformTimerDeregister(server->accessorySetup.nfcPairingModeTimer);
        server->accessorySetup.nfcPairingModeTimer = 0;
    }
    HAPRawBufferZero(&server->accessorySetup, sizeof server->accessorySetup);
    SynchronizeDisplayAndNFC(server_);
}

void HAPAccessorySetupInfoHandleAccessoryServerStateUpdate(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (!HAPAccessoryServerIsPaired(server_)) {
        // Resume generating dynamic setup codes.
        if (server->platform.setupDisplay) {
            PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
        } else {
            SynchronizeDisplayAndNFC(server_);
        }
    } else {
        // Exit NFC pairing mode.
        if (server->platform.setupNFC && server->accessorySetup.nfcPairingModeTimer) {
            HAPLogInfo(&logObject, "Pairing complete. Exiting NFC pairing mode.");
            HAPAccessorySetupInfoExitNFCPairingMode(server_);
        }
    }
}

void HAPAccessorySetupInfoHandlePairingStart(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(!HAPAccessoryServerIsPaired(server_));

    HAPLogDebug(&logObject, "Pairing attempt started.");

    // Lock setup code so that it cannot change during the pairing attempt.
    PrepareSetupInfo(server_, /* lockSetupInfo: */ true);
    HAPAssert(!server->accessorySetup.dynamicRefreshTimer);

    // Inform display that pairing is ongoing.
    if (server->platform.setupDisplay) {
        HAPPlatformAccessorySetupDisplayHandleStartPairing(HAPNonnull(server->platform.setupDisplay));
    }
}

void HAPAccessorySetupInfoHandlePairingStop(HAPAccessoryServerRef* server_, bool keepSetupInfo) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "Pairing attempt completed.");

    if (keepSetupInfo) {
        HAPLogInfo(&logObject, "Keeping setup code for next pairing attempt.");
        HAPAssert(server->accessorySetup.state.lockSetupInfo);
        server->accessorySetup.state.keepSetupInfo = true;
    } else {
        // Use a different code for next pairing attempt.
        ClearSetupInfo(server_);
        SynchronizeDisplayAndNFC(server_);
    }

    // Inform display that pairing has completed.
    if (server->platform.setupDisplay) {
        HAPPlatformAccessorySetupDisplayHandleStopPairing(HAPNonnull(server->platform.setupDisplay));
    }

    // Resume generating dynamic setup codes.
    if (server->platform.setupDisplay && !HAPAccessoryServerIsPaired(server_)) {
        PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
    }
}

//----------------------------------------------------------------------------------------------------------------------

void HAPAccessorySetupInfoRefreshSetupPayload(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupDisplay);

    if (!server->accessorySetup.dynamicRefreshTimer) {
        HAPLog(&logObject, "Not refreshing setup payload: Current setup payload does not expire.");
        return;
    }

    HAPLogInfo(&logObject, "Refreshing setup payload.");
    ClearSetupInfo(server_);
    PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
}

//----------------------------------------------------------------------------------------------------------------------

static void CompleteExitingNFCPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupNFC);
    HAPPrecondition(!server->accessorySetup.nfcPairingModeTimer);

    // Clear setup code if it is not used for purposes other than NFC.
    if (!server->accessorySetup.state.lockSetupInfo && !server->accessorySetup.dynamicRefreshTimer) {
        ClearSetupInfo(server_);
    }
    SynchronizeDisplayAndNFC(server_);
}

static void NFCPairingModeExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(timer == server->accessorySetup.nfcPairingModeTimer);
    server->accessorySetup.nfcPairingModeTimer = 0;

    HAPLogInfo(&logObject, "NFC pairing mode expired.");
    CompleteExitingNFCPairingMode(server_);
}

void HAPAccessorySetupInfoEnterNFCPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupNFC);

    HAPError err;

    if (HAPAccessoryServerIsPaired(server_)) {
        HAPLog(&logObject, "Not entering NFC pairing mode: Already paired.");
        return;
    }

    // Set up NFC pairing mode timer.
    bool forceSynchronization = false;
    if (server->accessorySetup.nfcPairingModeTimer) {
        HAPLogInfo(&logObject, "Extending ongoing NFC pairing mode.");
        HAPPlatformTimerDeregister(server->accessorySetup.nfcPairingModeTimer);
        server->accessorySetup.nfcPairingModeTimer = 0;
    } else {
        HAPLogInfo(&logObject, "Entering NFC pairing mode.");
        forceSynchronization = true;
    }
    err = HAPPlatformTimerRegister(
            &server->accessorySetup.nfcPairingModeTimer,
            HAPPlatformClockGetCurrent() + kHAPAccessoryServer_NFCPairingModeDuration,
            NFCPairingModeExpired,
            server_);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Not enough resources to allocate timer.");
        HAPFatalError();
    }

    // Prepare setup info.
    if (!server->accessorySetup.state.setupInfoIsAvailable && !server->accessorySetup.state.setupCodeIsAvailable) {
        PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
    } else if (forceSynchronization) {
        SynchronizeDisplayAndNFC(server_);
    }
}

void HAPAccessorySetupInfoExitNFCPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.setupNFC);

    if (!server->accessorySetup.nfcPairingModeTimer) {
        HAPLog(&logObject, "Exit NFC pairing mode ignored: NFC pairing mode is not active.");
        return;
    }

    HAPLogInfo(&logObject, "Exiting NFC pairing mode.");
    HAPPlatformTimerDeregister(server->accessorySetup.nfcPairingModeTimer);
    server->accessorySetup.nfcPairingModeTimer = 0;
    CompleteExitingNFCPairingMode(server_);
}

void HAPAccessorySetupInfoEnterLegacyPairingMode(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.accessorySetup);
    HAPPrecondition(!server->platform.setupDisplay);
    HAPPrecondition(!server->platform.setupNFC);
    HAPPlatformAccessorySetupCapabilities legacyCapabilities = GetLegacyAccessorySetupCapabilities(server_);

    if (!legacyCapabilities.supportsDisplay && !legacyCapabilities.supportsProgrammableNFC) {
        HAPLogInfo(&logObject, "Not entering legacy pairing mode: Static setup code and no NFC.");
        return;
    }
    if (server->pairSetup.sessionThatIsCurrentlyPairing) {
        HAPLogInfo(&logObject, "Not entering legacy pairing mode: Pairing already in progress.");
        return;
    }
    if (HAPAccessoryServerIsPaired(server_)) {
        HAPLogInfo(&logObject, "Not entering legacy pairing mode: Already paired.");
        return;
    }

    HAPLogInfo(&logObject, "Entering legacy pairing mode.");
    PrepareSetupInfo(server_, /* lockSetupInfo: */ false);
}
