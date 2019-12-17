// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

/**
 * TLV types used in HAP-Info-Response.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-9 Info Parameter Types
 */
HAP_ENUM_BEGIN(uint8_t, HAPInfoResponseTLVType) { /**
                                                   * HAP-Param-Current-State-Number.
                                                   * 2 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_StateNumber = 0x01,

                                                  /**
                                                   * HAP-Param-Current-Config-Number.
                                                   * 2 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_ConfigNumber = 0x02,

                                                  /**
                                                   * HAP-Param-Device-Identifier.
                                                   * 6 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_DeviceIdentifier = 0x03,

                                                  /**
                                                   * HAP-Param-Feature-Flags.
                                                   * 1 byte.
                                                   */
                                                  kHAPInfoResponseTLVType_FeatureFlags = 0x04,

                                                  /**
                                                   * HAP-Param-Model-Name.
                                                   * UTF-8 string, maximum 255 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_ModelName = 0x05,

                                                  /**
                                                   * HAP-Param-Protocol-Version.
                                                   * UTF-8 string, maximum 255 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_ProtocolVersion = 0x06,

                                                  /**
                                                   * HAP-Param-Status-Flag.
                                                   * 1 byte.
                                                   */
                                                  kHAPInfoResponseTLVType_StatusFlag = 0x07,

                                                  /**
                                                   * HAP-Param-Category-Identifier.
                                                   * 2 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_CategoryIdentifier = 0x08,

                                                  /**
                                                   * HAP-Param-Setup-Hash.
                                                   * 4 bytes.
                                                   */
                                                  kHAPInfoResponseTLVType_SetupHash = 0x09
} HAP_ENUM_END(uint8_t, HAPInfoResponseTLVType);

HAP_RESULT_USE_CHECK
HAPError HAPAccessoryGetInfoResponse(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(accessory);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.15.6 HAP-Info-Response

    // Response seems to be based on the _hap._tcp Bonjour TXT Record Keys used by IP accessories.
    // See HomeKit Accessory Protocol Specification R14
    // Table 6-7 _hap._tcp Bonjour TXT Record Keys

    // HAP-Param-Current-State-Number.
    uint16_t sn = 0;
    switch (session->transportType) {
        case kHAPTransportType_IP: {
            sn = 1;
        } break;
        case kHAPTransportType_BLE: {
            HAPBLEAccessoryServerGSN gsn;
            err = HAPNonnull(server->transports.ble)->getGSN(server->platform.keyValueStore, &gsn);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            sn = gsn.gsn;
        } break;
    }
    HAPAssert(sn);
    uint8_t snBytes[] = { HAPExpandLittleUInt16(sn) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_StateNumber,
                              .value = { .bytes = snBytes, .numBytes = sizeof snBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Current-Config-Number.
    uint16_t cn;
    err = HAPAccessoryServerGetCN(server->platform.keyValueStore, &cn);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    uint8_t cnBytes[] = { HAPExpandLittleUInt16(cn) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_ConfigNumber,
                              .value = { .bytes = cnBytes, .numBytes = sizeof cnBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Device-Identifier.
    HAPDeviceID deviceID;
    err = HAPDeviceIDGet(server->platform.keyValueStore, &deviceID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_DeviceIdentifier,
                              .value = { .bytes = deviceID.bytes, .numBytes = sizeof deviceID.bytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Feature-Flags.
    uint8_t pairingFeatureFlags = HAPAccessoryServerGetPairingFeatureFlags(server_);
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_FeatureFlags,
                              .value = { .bytes = &pairingFeatureFlags, .numBytes = sizeof pairingFeatureFlags } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Model-Name.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) {
                    .type = kHAPInfoResponseTLVType_ModelName,
                    .value = { .bytes = accessory->model, .numBytes = HAPStringGetNumBytes(accessory->model) } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Protocol-Version.
    const char* pv = NULL;
    switch (session->transportType) {
        case kHAPTransportType_IP: {
            pv = kHAPProtocolVersion_IP;
        } break;
        case kHAPTransportType_BLE: {
            pv = kHAPProtocolVersion_BLE;
        } break;
    }
    HAPAssert(pv);
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_ProtocolVersion,
                              .value = { .bytes = pv, .numBytes = HAPStringGetNumBytes(pv) } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Status-Flag.
    uint8_t statusFlags = HAPAccessoryServerGetStatusFlags(server_);
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPInfoResponseTLVType_StatusFlag,
                              .value = { .bytes = &statusFlags, .numBytes = sizeof statusFlags } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Category-Identifier.
    HAPAssert(accessory->category);
    uint8_t categoryIdentifierBytes[] = { HAPExpandLittleUInt16((uint16_t) accessory->category) };
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) {
                    .type = kHAPInfoResponseTLVType_CategoryIdentifier,
                    .value = { .bytes = categoryIdentifierBytes, .numBytes = sizeof categoryIdentifierBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // HAP-Param-Setup-Hash.
    HAPSetupID setupID;
    bool hasSetupID;
    HAPPlatformAccessorySetupLoadSetupID(server->platform.accessorySetup, &hasSetupID, &setupID);
    if (hasSetupID) {
        // Get Device ID string.
        HAPDeviceIDString deviceIDString;
        err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceIDString);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // Get setup hash.
        HAPAccessorySetupSetupHash setupHash;
        HAPAccessorySetupGetSetupHash(&setupHash, &setupID, &deviceIDString);

        // Append TLV.
        err = HAPTLVWriterAppend(
                responseWriter,
                &(const HAPTLV) { .type = kHAPInfoResponseTLVType_SetupHash,
                                  .value = { .bytes = setupHash.bytes, .numBytes = sizeof setupHash.bytes } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    }

    return kHAPError_None;
}
