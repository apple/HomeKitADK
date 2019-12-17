// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

#if HAP_IP

#include "util_base64.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "IPServiceDiscovery" };

/** _hap service. */
#define kServiceDiscoveryProtocol_HAP "_hap._tcp"

/**
 * TXT record keys for _hap service.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 6-7 _hap._tcp Bonjour TXT Record Keys
 */
/**@{*/
/** Current configuration number. */
#define kHAPTXTRecordKey_ConfigurationNumber "c#"

/** Pairing Feature flags. */
#define kHAPTXTRecordKey_PairingFeatureFlags "ff"

/** Device ID of the accessory. */
#define kHAPTXTRecordKey_DeviceID "id"

/** Model name of the accessory. */
#define kHAPTXTRecordKey_Model "md"

/** Protocol version string. */
#define kHAPTXTRecordKey_ProtocolVersion "pv"

/** Current state number. */
#define kHAPTXTRecordKey_StateNumber "s#"

/** Status flags. */
#define kHAPTXTRecordKey_StatusFlags "sf"

/** Accessory Category Identifier. */
#define kHAPTXTRecordKey_Category "ci"

/** Setup hash. */
#define kHAPTXTRecordKey_SetupHash "sh"
/**@}*/

/** Number of TXT Record keys for _hap service. */
#define kHAPTXTRecordKey_NumKeys (9)

void HAPIPServiceDiscoverySetHAPService(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(
            !server->ip.discoverableService || server->ip.discoverableService == kHAPIPServiceDiscoveryType_HAP);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.4 Discovery

    HAPPlatformServiceDiscoveryTXTRecord txtRecords[kHAPTXTRecordKey_NumKeys];
    size_t numTXTRecords = 0;

    // Configuration number.
    uint16_t configurationNumber;
    err = HAPAccessoryServerGetCN(server->platform.keyValueStore, &configurationNumber);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    char configurationNumberBytes[kHAPUInt16_MaxDescriptionBytes];
    err = HAPUInt64GetDescription(configurationNumber, configurationNumberBytes, sizeof configurationNumberBytes);
    HAPAssert(!err);
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kHAPTXTRecordKey_ConfigurationNumber,
        .value = { .bytes = configurationNumberBytes, .numBytes = HAPStringGetNumBytes(configurationNumberBytes) }
    };

    // Pairing Feature flags.
    uint8_t pairingFeatureFlags = HAPAccessoryServerGetPairingFeatureFlags(server_);
    char pairingFeatureFlagsBytes[kHAPUInt8_MaxDescriptionBytes];
    err = HAPUInt64GetDescription(pairingFeatureFlags, pairingFeatureFlagsBytes, sizeof pairingFeatureFlagsBytes);
    HAPAssert(!err);
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kHAPTXTRecordKey_PairingFeatureFlags,
        .value = { .bytes = pairingFeatureFlagsBytes, .numBytes = HAPStringGetNumBytes(pairingFeatureFlagsBytes) }
    };

    // Device ID.
    HAPDeviceIDString deviceIDString;
    err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceIDString);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kHAPTXTRecordKey_DeviceID,
        .value = { .bytes = deviceIDString.stringValue, .numBytes = HAPStringGetNumBytes(deviceIDString.stringValue) }
    };

    // Model.
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kHAPTXTRecordKey_Model,
        .value = { .bytes = server->primaryAccessory->model,
                   .numBytes = HAPStringGetNumBytes(server->primaryAccessory->model) }
    };

    // Protocol version.
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kHAPTXTRecordKey_ProtocolVersion,
        .value = { .bytes = kHAPShortProtocolVersion_IP, .numBytes = HAPStringGetNumBytes(kHAPShortProtocolVersion_IP) }
    };

    // Current state number. Must always be set to 1 for IP.
    const char* stateNumberBytes = "1";
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] =
            (HAPPlatformServiceDiscoveryTXTRecord) { .key = kHAPTXTRecordKey_StateNumber,
                                                     .value = { .bytes = stateNumberBytes,
                                                                .numBytes = HAPStringGetNumBytes(stateNumberBytes) } };

    // Status flags.
    uint8_t statusFlags = HAPAccessoryServerGetStatusFlags(server_);
    char statusFlagsBytes[kHAPUInt8_MaxDescriptionBytes];
    err = HAPUInt64GetDescription(statusFlags, statusFlagsBytes, sizeof statusFlagsBytes);
    HAPAssert(!err);
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] =
            (HAPPlatformServiceDiscoveryTXTRecord) { .key = kHAPTXTRecordKey_StatusFlags,
                                                     .value = { .bytes = statusFlagsBytes,
                                                                .numBytes = HAPStringGetNumBytes(statusFlagsBytes) } };

    // Category.
    char categoryBytes[kHAPUInt16_MaxDescriptionBytes];
    err = HAPUInt64GetDescription(server->primaryAccessory->category, categoryBytes, sizeof categoryBytes);
    HAPAssert(!err);
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] =
            (HAPPlatformServiceDiscoveryTXTRecord) { .key = kHAPTXTRecordKey_Category,
                                                     .value = { .bytes = categoryBytes,
                                                                .numBytes = HAPStringGetNumBytes(categoryBytes) } };

    // Setup hash. Optional.
    HAPSetupID setupID;
    bool hasSetupID = false;
    HAPPlatformAccessorySetupLoadSetupID(server->platform.accessorySetup, &hasSetupID, &setupID);
    char setupHashBytes[util_base64_encoded_len(sizeof(HAPAccessorySetupSetupHash)) + 1];
    if (hasSetupID) {
        // Get raw setup hash from setup ID.
        HAPAccessorySetupSetupHash setupHash;
        HAPAccessorySetupGetSetupHash(&setupHash, &setupID, &deviceIDString);

        // Base64 encode.
        size_t numSetupHashBytes;
        util_base64_encode(
                setupHash.bytes, sizeof setupHash.bytes, setupHashBytes, sizeof setupHashBytes, &numSetupHashBytes);
        HAPAssert(numSetupHashBytes == sizeof setupHashBytes - 1);
        setupHashBytes[sizeof setupHashBytes - 1] = '\0';

        // Append TXT record.
        HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
        txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
            .key = kHAPTXTRecordKey_SetupHash,
            .value = { .bytes = setupHashBytes, .numBytes = HAPStringGetNumBytes(setupHashBytes) }
        };
    }

    // Register service.
    HAPAssert(numTXTRecords <= HAPArrayCount(txtRecords));
    if (!server->ip.discoverableService) {
        server->ip.discoverableService = kHAPIPServiceDiscoveryType_HAP;
        HAPLogInfo(&logObject, "Registering %s service.", kServiceDiscoveryProtocol_HAP);
        HAPPlatformServiceDiscoveryRegister(
                HAPNonnull(server->platform.ip.serviceDiscovery),
                server->primaryAccessory->name,
                kServiceDiscoveryProtocol_HAP,
                HAPPlatformTCPStreamManagerGetListenerPort(HAPNonnull(server->platform.ip.tcpStreamManager)),
                txtRecords,
                numTXTRecords);
    } else {
        HAPAssert(server->ip.discoverableService == kHAPIPServiceDiscoveryType_HAP);
        HAPLogInfo(&logObject, "Updating %s service.", kServiceDiscoveryProtocol_HAP);
        HAPPlatformServiceDiscoveryUpdateTXTRecords(
                HAPNonnull(server->platform.ip.serviceDiscovery), txtRecords, numTXTRecords);
    }
}

/**
 * _mfi-config service.
 */
#define kServiceDiscoveryProtocol_MFiConfig "_mfi-config._tcp"

/**
 * TXT Record keys for _mfi-config service.
 */
/**@{*/
/**
 * DeviceID.
 */
#define kMFiConfigTXTRecordKey_DeviceID "deviceid"

/**
 * Bonjour seed.
 */
#define kMFiConfigTXTRecordKey_Seed "seed"

/**
 * Features.
 */
#define kMFiConfigTXTRecordKey_Features "features"

/**
 * Source version.
 */
#define kMFiConfigTXTRecordKey_SourceVersion "srcvers"
/**@}*/

/**
 * Number of TXT Record keys for _mfi-config service.
 */
#define kMFiConfigTXTRecordKey_NumKeys (4)

/**
 * Version number of the latest POSIX server reference code.
 */
#define kMFiConfigTXTRecordValue_SourceVersion "1.22"

void HAPIPServiceDiscoverySetMFiConfigService(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(
            !server->ip.discoverableService || server->ip.discoverableService == kHAPIPServiceDiscoveryType_MFiConfig);

    HAPError err;

    HAPPlatformServiceDiscoveryTXTRecord txtRecords[kMFiConfigTXTRecordKey_NumKeys];
    size_t numTXTRecords = 0;

    // Device ID.
    HAPDeviceIDString deviceIDString;
    err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceIDString);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kMFiConfigTXTRecordKey_DeviceID,
        .value = { .bytes = deviceIDString.stringValue, .numBytes = HAPStringGetNumBytes(deviceIDString.stringValue) }
    };

    // Bonjour seed.
    // Controllers use the "seed", "sd", "c#" keys to derive the seed value that they process.
    // See -[EasyConfigDevice configureStart:] and -[EasyConfigDevice findDevicePostConfigEvent:info:].
    //
    // We choose to synchronize the "_mfi-config._tcp" service's "seed" value with the "_hap._tcp" service's
    // "c#" value for maximum consistency.
    uint16_t configurationNumber;
    err = HAPAccessoryServerGetCN(server->platform.keyValueStore, &configurationNumber);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    char bonjourSeedBytes[kHAPUInt16_MaxDescriptionBytes];
    err = HAPUInt64GetDescription(configurationNumber, bonjourSeedBytes, sizeof bonjourSeedBytes);
    HAPAssert(!err);
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] =
            (HAPPlatformServiceDiscoveryTXTRecord) { .key = kMFiConfigTXTRecordKey_Seed,
                                                     .value = { .bytes = bonjourSeedBytes,
                                                                .numBytes = HAPStringGetNumBytes(bonjourSeedBytes) } };

    // Features. Must always be 4?
    const char* featuresBytes = "4";
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] =
            (HAPPlatformServiceDiscoveryTXTRecord) { .key = kMFiConfigTXTRecordKey_Features,
                                                     .value = { .bytes = featuresBytes,
                                                                .numBytes = HAPStringGetNumBytes(featuresBytes) } };

    // Source version. Must match most recent POSIX server reference code to pass certification.
    HAPAssert(numTXTRecords < HAPArrayCount(txtRecords));
    txtRecords[numTXTRecords++] = (HAPPlatformServiceDiscoveryTXTRecord) {
        .key = kMFiConfigTXTRecordKey_SourceVersion,
        .value = { .bytes = kMFiConfigTXTRecordValue_SourceVersion,
                   .numBytes = HAPStringGetNumBytes(kMFiConfigTXTRecordValue_SourceVersion) }
    };

    // Register service.
    HAPAssert(numTXTRecords <= HAPArrayCount(txtRecords));
    if (!server->ip.discoverableService) {
        server->ip.discoverableService = kHAPIPServiceDiscoveryType_MFiConfig;
        HAPLogInfo(&logObject, "Registering %s service.", kServiceDiscoveryProtocol_MFiConfig);
        HAPPlatformServiceDiscoveryRegister(
                HAPNonnull(server->platform.ip.serviceDiscovery),
                server->primaryAccessory->name,
                kServiceDiscoveryProtocol_MFiConfig,
                HAPPlatformTCPStreamManagerGetListenerPort(HAPNonnull(server->platform.ip.tcpStreamManager)),
                txtRecords,
                numTXTRecords);
    } else {
        HAPAssert(server->ip.discoverableService == kHAPIPServiceDiscoveryType_MFiConfig);
        HAPLogInfo(&logObject, "Updating %s service.", kServiceDiscoveryProtocol_MFiConfig);
        HAPPlatformServiceDiscoveryUpdateTXTRecords(
                HAPNonnull(server->platform.ip.serviceDiscovery), txtRecords, numTXTRecords);
    }
}

void HAPIPServiceDiscoveryStop(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    if (server->ip.discoverableService) {
        HAPLogInfo(&logObject, "Stopping service discovery.");
        HAPPlatformServiceDiscoveryStop(HAPNonnull(server->platform.ip.serviceDiscovery));
        server->ip.discoverableService = kHAPIPServiceDiscoveryType_None;
    }
}

#endif
