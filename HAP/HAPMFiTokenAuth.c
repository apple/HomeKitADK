// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "MFiTokenAuth" };

/**
 * TLV types used in HAP-Token-Response and HAP-Update-Token-Request.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Table 5-4 Software Authentication TLV types
 */
HAP_ENUM_BEGIN(uint8_t, HAPMFiTokenAuthTLVType) { /**
                                                   * UUID (The matching UUID for the initial token provisioned on the
                                                   * accessory). 16 bytes.
                                                   */
                                                  kHAPMFiTokenAuthTLVType_UUID = 0x01,

                                                  /**
                                                   * Software Token.
                                                   * Opaque blob, up to kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes bytes.
                                                   */
                                                  kHAPMFiTokenAuthTLVType_SoftwareToken = 0x02
} HAP_ENUM_END(uint8_t, HAPMFiTokenAuthTLVType);

HAP_RESULT_USE_CHECK
HAPError HAPMFiTokenAuthGetTokenResponse(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPAccessory* accessory,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(accessory);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.15.2 HAP-Token-Response

    // Load Software Token.
    HAPPlatformMFiTokenAuthUUID mfiTokenUUID;
    void* mfiTokenBytes;
    size_t maxMFiTokenBytes;
    size_t numMFiTokenBytes = 0;
    HAPTLVWriterGetScratchBytes(responseWriter, &mfiTokenBytes, &maxMFiTokenBytes);
    bool hasMFiToken = false;
    if (server->platform.authentication.mfiTokenAuth) {
        err = HAPPlatformMFiTokenAuthLoad(
                HAPNonnull(server->platform.authentication.mfiTokenAuth),
                &hasMFiToken,
                &mfiTokenUUID,
                mfiTokenBytes,
                maxMFiTokenBytes,
                &numMFiTokenBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }
    }
    if (!hasMFiToken) {
        HAPLog(&logObject, "Software Token requested but no token is provisioned.");
        return kHAPError_InvalidState;
    }

    // Software Token.
    // Do this first because scratch buffer gets invalidated on TLV append.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPMFiTokenAuthTLVType_SoftwareToken,
                              .value = { .bytes = mfiTokenBytes, .numBytes = numMFiTokenBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // UUID.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPMFiTokenAuthTLVType_UUID,
                              .value = { .bytes = mfiTokenUUID.bytes, .numBytes = sizeof mfiTokenUUID.bytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPMFiTokenAuthHandleTokenUpdateRequest(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        const HAPAccessory* accessory,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(accessory);
    HAPPrecondition(requestReader);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.15.3 HAP-Token-Update-Request

    HAPTLV softwareTokenTLV;
    softwareTokenTLV.type = kHAPMFiTokenAuthTLVType_SoftwareToken;
    err = HAPTLVReaderGetAll(requestReader, (HAPTLV* const[]) { &softwareTokenTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        return err;
    }

    // Validate Software Token.
    if (!softwareTokenTLV.value.bytes) {
        HAPLog(&logObject, "Software Token missing.");
        return kHAPError_InvalidData;
    }
    if (softwareTokenTLV.value.numBytes > kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes) {
        HAPLog(&logObject, "Software Token has invalid length (%lu).", (unsigned long) softwareTokenTLV.value.numBytes);
        return kHAPError_InvalidData;
    }
    HAPLogSensitiveBuffer(&logObject, softwareTokenTLV.value.bytes, softwareTokenTLV.value.numBytes, "Software Token");

    // Update Token.
    HAPLogInfo(
            &logObject,
            "Updating Software Token (length = %lu bytes).",
            (unsigned long) softwareTokenTLV.value.numBytes);
    if (!server->platform.authentication.mfiTokenAuth) {
        HAPLog(&logObject, "Software Authentication not supported.");
        return kHAPError_Unknown;
    }
    err = HAPPlatformMFiTokenAuthUpdate(
            HAPNonnull(server->platform.authentication.mfiTokenAuth),
            HAPNonnullVoid(softwareTokenTLV.value.bytes),
            softwareTokenTLV.value.numBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    return kHAPError_None;
}
