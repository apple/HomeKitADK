// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "PairingPairVerify" };

void HAPPairingPairVerifyReset(HAPSessionRef* session_) {
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    // Reset Pair Verify procedure state.
    HAPRawBufferZero(&session->state.pairVerify, sizeof session->state.pairVerify);
    session->state.pairVerify.pairingID = -1;
}

/**
 * Starts the HAP session after successful Pair Verify / Pair Resume.
 *
 * @param      session_             Session.
 */
static void HAPPairingPairVerifyStartSession(HAPSessionRef* session_) {
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairVerify.pairingID >= 0);

    // Initialize HAP session.
    HAPRawBufferZero(&session->hap, sizeof session->hap);

    // See HomeKit Accessory Protocol Specification R14
    // Section 6.5.2 Session Security
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.7.2 Session Security

    // Derive encryption keys.
    {
        static const uint8_t salt[] = "Control-Salt";
        {
            static const uint8_t info[] = "Control-Read-Encryption-Key";
            HAP_hkdf_sha512(
                    session->hap.accessoryToController.controlChannel.key.bytes,
                    sizeof session->hap.accessoryToController.controlChannel.key.bytes,
                    session->state.pairVerify.cv_KEY,
                    sizeof session->state.pairVerify.cv_KEY,
                    salt,
                    sizeof salt - 1,
                    info,
                    sizeof info - 1);
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    session->hap.accessoryToController.controlChannel.key.bytes,
                    sizeof session->hap.accessoryToController.controlChannel.key.bytes,
                    "Pair Verify Start Session: AccessoryToControllerKey");
        }
        {
            static const uint8_t info[] = "Control-Write-Encryption-Key";
            HAP_hkdf_sha512(
                    session->hap.controllerToAccessory.controlChannel.key.bytes,
                    sizeof session->hap.controllerToAccessory.controlChannel.key.bytes,
                    session->state.pairVerify.cv_KEY,
                    sizeof session->state.pairVerify.cv_KEY,
                    salt,
                    sizeof salt - 1,
                    info,
                    sizeof info - 1);
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    session->hap.controllerToAccessory.controlChannel.key.bytes,
                    sizeof session->hap.controllerToAccessory.controlChannel.key.bytes,
                    "Pair Verify Start Session: ControllerToAccessoryKey");
        }
        session->hap.accessoryToController.controlChannel.nonce = 0;
        session->hap.controllerToAccessory.controlChannel.nonce = 0;
    }

    // Copy shared secret.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.4.7.3 Broadcast Encryption Key Generation
    HAPRawBufferCopyBytes(session->hap.cv_KEY, session->state.pairVerify.cv_KEY, sizeof session->hap.cv_KEY);

    // Copy pairing ID.
    session->hap.pairingID = session->state.pairVerify.pairingID;

    // Activate session.
    session->hap.active = true;

    // Reset Pair Verify procedure.
    HAPPairingPairVerifyReset(session_);

    HAPLogInfo(&logObject, "Pair Verify procedure completed (pairing ID %d).", session->hap.pairingID);

    HAPAccessoryServerRef* server_ = session->server;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    // Inform application.
    if (server->callbacks.handleSessionAccept) {
        server->callbacks.handleSessionAccept(server_, session_, server->context);
    }
    if (server->transports.ble) {
        HAPNonnull(server->transports.ble)->peripheralManager.handleSessionAccept(server_, session_);
    }
}

/**
 * Pair Verify M1 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;     /**< kTLVType_State. */
    HAPTLV* publicKeyTLV; /**< kTLVType_PublicKey. */

    // Pair Resume.
    HAPTLV* methodTLV;        /**< kTLVType_Method. */
    HAPTLV* sessionIDTLV;     /**< kTLVType_SessionID. */
    HAPTLV* encryptedDataTLV; /**< kTLVType_EncryptedData. */
} HAPPairingPairVerifyM1TLVs;

/**
 * Processes Pair Verify M1.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      scratchBytes         Free memory.
 * @param      numScratchBytes      Length of free memory buffer.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If the free memory buffer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyProcessM1(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        void* scratchBytes,
        size_t numScratchBytes,
        const HAPPairingPairVerifyM1TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairVerify.state == 1);
    HAPPrecondition(!session->state.pairVerify.error);
    HAPPrecondition(!session->hap.active);
    HAPPrecondition(scratchBytes);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->publicKeyTLV);
    HAPPrecondition(tlvs->publicKeyTLV->type == kHAPPairingTLVType_PublicKey);
    HAPPrecondition(tlvs->methodTLV);
    HAPPrecondition(tlvs->methodTLV->type == kHAPPairingTLVType_Method);
    HAPPrecondition(tlvs->sessionIDTLV);
    HAPPrecondition(tlvs->sessionIDTLV->type == kHAPPairingTLVType_SessionID);
    HAPPrecondition(tlvs->encryptedDataTLV);
    HAPPrecondition(tlvs->encryptedDataTLV->type == kHAPPairingTLVType_EncryptedData);

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.7.1 M1: iOS Device -> Accessory -- `Verify Start Request'

    HAPLogDebug(&logObject, "Pair Verify M1: Verify Start Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Pair Verify M1: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Pair Verify M1: kTLVType_State has invalid length (%lu).",
               (unsigned long) tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 1) {
        HAPLog(&logObject, "Pair Verify M1: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Method.
    uint8_t method = kHAPPairingMethod_PairVerify;
    if (tlvs->methodTLV->value.bytes) {
        if (tlvs->methodTLV->value.numBytes != 1) {
            HAPLog(&logObject,
                   "Pair Verify M1: kTLVType_Method has invalid length (%lu).",
                   (unsigned long) tlvs->methodTLV->value.numBytes);
            return kHAPError_InvalidData;
        }
        method = ((const uint8_t*) tlvs->methodTLV->value.bytes)[0];
        if (method != kHAPPairingMethod_PairResume) {
            HAPLog(&logObject, "Pair Verify M1: kTLVType_Method invalid: %u.", method);
            return kHAPError_InvalidData;
        }

        // Validate kTLVType_SessionID.
        if (!tlvs->sessionIDTLV->value.bytes) {
            HAPLog(&logObject, "Pair Verify M1: kTLVType_SessionID missing.");
            return kHAPError_InvalidData;
        }
        if (tlvs->sessionIDTLV->value.numBytes != sizeof(HAPPairingBLESessionID)) {
            HAPLog(&logObject,
                   "Pair Verify M1: kTLVType_SessionID has invalid length (%lu).",
                   (unsigned long) tlvs->sessionIDTLV->value.numBytes);
            return kHAPError_InvalidData;
        }

        // Validate kTLVType_EncryptedData.
        if (!tlvs->encryptedDataTLV->value.bytes) {
            HAPLog(&logObject, "Pair Verify M1: kTLVType_EncryptedData missing.");
            return kHAPError_InvalidData;
        }
        if (tlvs->encryptedDataTLV->value.numBytes != CHACHA20_POLY1305_TAG_BYTES) {
            HAPLog(&logObject,
                   "Pair Verify M1: kTLVType_EncryptedData has invalid length (%lu).",
                   (unsigned long) tlvs->encryptedDataTLV->value.numBytes);
            return kHAPError_InvalidData;
        }

        if (session->transportType != kHAPTransportType_BLE) {
            HAPLog(&logObject, "Pair Verify M1: Pair Resume requested over non-BLE transport.");
            return kHAPError_InvalidData;
        }
    }

    // Validate kTLVType_PublicKey.
    if (!tlvs->publicKeyTLV->value.bytes) {
        HAPLog(&logObject, "Pair Verify M1: kTLVType_PublicKey missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->publicKeyTLV->value.numBytes != X25519_BYTES) {
        HAPLog(&logObject,
               "Pair Verify M1: kTLVType_PublicKey has invalid length (%lu).",
               (unsigned long) tlvs->publicKeyTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Store method.
    HAPLogDebug(&logObject, "Pair Verify M1: kTLVType_Method = %u.", method);
    session->state.pairVerify.method = method;

    // Copy public key.
    HAPRawBufferCopyBytes(
            session->state.pairVerify.Controller_cv_PK,
            HAPNonnullVoid(tlvs->publicKeyTLV->value.bytes),
            tlvs->publicKeyTLV->value.numBytes);
    HAPLogBufferDebug(
            &logObject,
            session->state.pairVerify.Controller_cv_PK,
            sizeof session->state.pairVerify.Controller_cv_PK,
            "Pair Verify M1: Controller_cv_PK.");

    // BLE: Handle Pair Resume.
    if (session->state.pairVerify.method == kHAPPairingMethod_PairResume) {
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.3.7.4.1 M1: Controller -> Accessory - Resume Request

        HAPLogDebug(&logObject, "Pair Resume M1: Resume Request.");

        HAPLogBufferDebug(
                &logObject,
                tlvs->sessionIDTLV->value.bytes,
                tlvs->sessionIDTLV->value.numBytes,
                "Pair Resume M1: kTLVType_SessionID.");

        if (server->transports.ble) {
            HAPNonnull(server->transports.ble)
                    ->sessionCache.fetch(
                            server_,
                            HAPNonnullVoid(tlvs->sessionIDTLV->value.bytes),
                            session->state.pairVerify.cv_KEY,
                            &session->state.pairVerify.pairingID);
        } else {
            session->state.pairVerify.pairingID = -1;
        }

        if (session->state.pairVerify.pairingID >= 0) {
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    session->state.pairVerify.cv_KEY,
                    sizeof session->state.pairVerify.cv_KEY,
                    "Pair Resume M1: cv_KEY.");

            void* key = HAPTLVScratchBufferAlloc(&scratchBytes, &numScratchBytes, CHACHA20_POLY1305_KEY_BYTES);
            void* salt = HAPTLVScratchBufferAlloc(&scratchBytes, &numScratchBytes, X25519_BYTES);
            void* sessionID =
                    HAPTLVScratchBufferAllocUnaligned(&scratchBytes, &numScratchBytes, sizeof(HAPPairingBLESessionID));
            if (!key || !salt || !sessionID) {
                HAPLog(&logObject, "Pair Resume M1: Not enough memory to allocate RequestKey / PublicKey / SessionID.");
                return kHAPError_OutOfResources;
            }

            // Derive request encryption key.
            HAPRawBufferCopyBytes(
                    salt, HAPNonnullVoid(tlvs->publicKeyTLV->value.bytes), tlvs->publicKeyTLV->value.numBytes);
            HAPRawBufferCopyBytes(
                    sessionID, HAPNonnullVoid(tlvs->sessionIDTLV->value.bytes), tlvs->sessionIDTLV->value.numBytes);
            HAPLogSensitiveBufferDebug(
                    &logObject, salt, X25519_BYTES + sizeof(HAPPairingBLESessionID), "Pair Resume M1: Salt.");
            static const uint8_t info[] = "Pair-Resume-Request-Info";
            HAP_hkdf_sha512(
                    key,
                    CHACHA20_POLY1305_KEY_BYTES,
                    session->state.pairVerify.cv_KEY,
                    sizeof session->state.pairVerify.cv_KEY,
                    salt,
                    X25519_BYTES + sizeof(HAPPairingBLESessionID),
                    info,
                    sizeof info - 1);
            HAPLogSensitiveBufferDebug(&logObject, key, CHACHA20_POLY1305_KEY_BYTES, "Pair Resume M1: RequestKey.");

            // Decrypt data.
            HAPLogBufferDebug(
                    &logObject,
                    tlvs->encryptedDataTLV->value.bytes,
                    tlvs->encryptedDataTLV->value.numBytes,
                    "Pair Resume M1: kTLVType_EncryptedData.");
            static const uint8_t nonce[] = "PR-Msg01";
            int e = HAP_chacha20_poly1305_decrypt(
                    tlvs->encryptedDataTLV->value.bytes, NULL, NULL, 0, nonce, sizeof nonce - 1, key);
            if (e) {
                HAPAssert(e == -1);
                HAPLog(&logObject, "Pair Resume M1: Failed to verify auth tag of kTLVType_EncryptedData.");
                session->state.pairVerify.error = kHAPPairingError_Authentication;
                return kHAPError_None;
            }
        } else {
            // Not found. Fall back to Pair Verify.
            HAPLog(&logObject, "Pair Resume M1: Pair Resume Shared Secret not found. Falling back to Pair Verify.");
            session->state.pairVerify.method = kHAPPairingMethod_PairVerify;
        }
    }

    return kHAPError_None;
}

/**
 * Processes Pair Verify M2.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyGetM2(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairVerify.state == 2);
    HAPPrecondition(!session->state.pairVerify.error);
    HAPPrecondition(!session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.7.2 M2: Accessory -> iOS Device -- `Verify Start Response'

    HAPLogDebug(&logObject, "Pair Verify M2: Verify Start Response.");

    // Create new, random key pair.
    HAPPlatformRandomNumberFill(session->state.pairVerify.cv_SK, sizeof session->state.pairVerify.cv_SK);
    HAP_X25519_scalarmult_base(session->state.pairVerify.cv_PK, session->state.pairVerify.cv_SK);
    HAPLogSensitiveBufferDebug(
            &logObject,
            session->state.pairVerify.cv_SK,
            sizeof session->state.pairVerify.cv_SK,
            "Pair Verify M2: cv_SK.");
    HAPLogBufferDebug(
            &logObject,
            session->state.pairVerify.cv_PK,
            sizeof session->state.pairVerify.cv_PK,
            "Pair Verify M2: cv_PK.");

    // Generate the shared secret.
    HAP_X25519_scalarmult(
            session->state.pairVerify.cv_KEY,
            session->state.pairVerify.cv_SK,
            session->state.pairVerify.Controller_cv_PK);
    HAPLogSensitiveBufferDebug(
            &logObject,
            session->state.pairVerify.cv_KEY,
            sizeof session->state.pairVerify.cv_KEY,
            "Pair Verify M2: cv_KEY.");

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairVerify.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_PublicKey.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_PublicKey,
                              .value = { .bytes = session->state.pairVerify.cv_PK,
                                         .numBytes = sizeof session->state.pairVerify.cv_PK } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // Construct sub-TLV writer.
    HAPTLVWriterRef subWriter;
    {
        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);
        if (maxBytes < CHACHA20_POLY1305_TAG_BYTES) {
            HAPLog(&logObject, "Pair Verify M2: Not enough memory for kTLVType_EncryptedData auth tag.");
            return kHAPError_OutOfResources;
        }
        maxBytes -= CHACHA20_POLY1305_TAG_BYTES;
        HAPTLVWriterCreate(&subWriter, bytes, maxBytes);
    }

    // kTLVType_Identifier.
    HAPDeviceIDString deviceIDString;
    err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceIDString);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    size_t numDeviceIDStringBytes = HAPStringGetNumBytes(deviceIDString.stringValue);
    err = HAPTLVWriterAppend(
            &subWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Identifier,
                              .value = { .bytes = deviceIDString.stringValue, .numBytes = numDeviceIDStringBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Signature.
    {
        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(&subWriter, &bytes, &maxBytes);

        void* accessoryCvPK = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, X25519_BYTES);
        void* accessoryPairingID = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, numDeviceIDStringBytes);
        void* iOSDeviceCvPK = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, X25519_BYTES);
        void* signature = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, ED25519_BYTES);
        if (!accessoryCvPK || !accessoryPairingID || !iOSDeviceCvPK || !signature) {
            HAPLog(&logObject,
                   "Pair Verify M2: Not enough memory to allocate "
                   "AccessoryCvPK / AccessoryPairingID / iOSDeviceCvPK / Signature.");
            return kHAPError_OutOfResources;
        }

        // Construct AccessoryInfo: AccessoryCvPK, AccessoryPairingID, iOSDeviceCvPK.
        HAPRawBufferCopyBytes(accessoryCvPK, session->state.pairVerify.cv_PK, sizeof session->state.pairVerify.cv_PK);
        HAPRawBufferCopyBytes(accessoryPairingID, deviceIDString.stringValue, numDeviceIDStringBytes);
        HAPRawBufferCopyBytes(
                iOSDeviceCvPK,
                session->state.pairVerify.Controller_cv_PK,
                sizeof session->state.pairVerify.Controller_cv_PK);

        // Finalize info.
        void* infoBytes = accessoryCvPK;
        size_t numInfoBytes = X25519_BYTES + numDeviceIDStringBytes + X25519_BYTES;

        // Generate signature.
        HAP_ed25519_sign(signature, infoBytes, numInfoBytes, server->identity.ed_LTSK.bytes, server->identity.ed_LTPK);
        HAPLogSensitiveBufferDebug(&logObject, infoBytes, numInfoBytes, "Pair Verify M2: AccessoryInfo");
        HAPLogSensitiveBufferDebug(&logObject, signature, ED25519_BYTES, "Pair Verify M2: kTLVType_Signature");

        // kTLVType_Signature.
        err = HAPTLVWriterAppend(
                &subWriter,
                &(const HAPTLV) { .type = kHAPPairingTLVType_Signature,
                                  .value = { .bytes = signature, .numBytes = ED25519_BYTES } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    }

    // Derive the symmetric session encryption key.
    static const uint8_t salt[] = "Pair-Verify-Encrypt-Salt";
    static const uint8_t info[] = "Pair-Verify-Encrypt-Info";
    HAP_hkdf_sha512(
            session->state.pairVerify.SessionKey,
            sizeof session->state.pairVerify.SessionKey,
            session->state.pairVerify.cv_KEY,
            sizeof session->state.pairVerify.cv_KEY,
            salt,
            sizeof salt - 1,
            info,
            sizeof info - 1);
    HAPLogSensitiveBufferDebug(
            &logObject,
            session->state.pairVerify.SessionKey,
            sizeof session->state.pairVerify.SessionKey,
            "Pair Verify M2: SessionKey");

    // Encrypt the sub-TLV.
    void* bytes;
    size_t numBytes;
    HAPTLVWriterGetBuffer(&subWriter, &bytes, &numBytes);
    static const uint8_t nonce[] = "PV-Msg02";
    HAP_chacha20_poly1305_encrypt(
            &((uint8_t*) bytes)[numBytes],
            bytes,
            bytes,
            numBytes,
            nonce,
            sizeof nonce - 1,
            session->state.pairVerify.SessionKey);
    numBytes += CHACHA20_POLY1305_TAG_BYTES;
    HAPLogBufferDebug(&logObject, bytes, numBytes, "Pair Verify M2: kTLVType_EncryptedData.");

    // kTLVType_EncryptedData.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_EncryptedData,
                              .value = { .bytes = bytes, .numBytes = numBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

/**
 * Processes Pair Resume M2.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyGetM2ForBLEPairResume(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->transports.ble);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);
    HAPPrecondition(session->state.pairVerify.state == 2);
    HAPPrecondition(!session->state.pairVerify.error);
    HAPPrecondition(!session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.7.4.2 M2: Accessory -> Controller - Resume Response

    HAPLogDebug(&logObject, "Pair Resume M2: Resume Response.");

    void* bytes;
    size_t maxBytes;
    HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

    void* key = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, CHACHA20_POLY1305_KEY_BYTES);
    void* salt = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, X25519_BYTES);
    void* sessionID = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, sizeof(HAPPairingBLESessionID));
    if (!key || !salt || !sessionID) {
        HAPLog(&logObject, "Pair Resume M2: Not enough memory to allocate ResponseKey / PublicKey / SessionID.");
        return kHAPError_OutOfResources;
    }

    // Generate new session ID.
    HAPPlatformRandomNumberFill(sessionID, sizeof(HAPPairingBLESessionID));

    // Derive response encryption key.
    HAPRawBufferCopyBytes(
            salt, session->state.pairVerify.Controller_cv_PK, sizeof session->state.pairVerify.Controller_cv_PK);
    HAPLogSensitiveBufferDebug(
            &logObject, salt, X25519_BYTES + sizeof(HAPPairingBLESessionID), "Pair Resume M2: Salt.");
    {
        static const uint8_t info[] = "Pair-Resume-Response-Info";
        HAP_hkdf_sha512(
                key,
                CHACHA20_POLY1305_KEY_BYTES,
                session->state.pairVerify.cv_KEY,
                sizeof session->state.pairVerify.cv_KEY,
                salt,
                X25519_BYTES + sizeof(HAPPairingBLESessionID),
                info,
                sizeof info - 1);
        HAPLogSensitiveBufferDebug(&logObject, key, CHACHA20_POLY1305_KEY_BYTES, "Pair Resume M2: ResponseKey.");
    }

    // Encrypt empty data.
    uint8_t tag[CHACHA20_POLY1305_TAG_BYTES];
    static const uint8_t nonce[] = "PR-Msg02";
    HAP_chacha20_poly1305_encrypt(tag, NULL, NULL, 0, nonce, sizeof nonce - 1, key);

    // Generate new shared secret.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.7.5 Compute Shared Secret
    {
        static const uint8_t info[] = "Pair-Resume-Shared-Secret-Info";
        HAP_hkdf_sha512(
                session->state.pairVerify.cv_KEY,
                sizeof session->state.pairVerify.cv_KEY,
                session->state.pairVerify.cv_KEY,
                sizeof session->state.pairVerify.cv_KEY,
                salt,
                X25519_BYTES + sizeof(HAPPairingBLESessionID),
                info,
                sizeof info - 1);
        HAPLogSensitiveBufferDebug(
                &logObject,
                session->state.pairVerify.cv_KEY,
                sizeof session->state.pairVerify.cv_KEY,
                "Pair Resume M2: cv_KEY.");
    }

    // Save shared secret.
    HAPNonnull(server->transports.ble)
            ->sessionCache.save(
                    server_, sessionID, session->state.pairVerify.cv_KEY, session->state.pairVerify.pairingID);

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairVerify.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Method.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Method,
                              .value = { .bytes = &session->state.pairVerify.method, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_SessionID.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_SessionID,
                              .value = { .bytes = sessionID, .numBytes = sizeof(HAPPairingBLESessionID) } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_EncryptedData.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_EncryptedData,
                              .value = { .bytes = tag, .numBytes = CHACHA20_POLY1305_TAG_BYTES } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // Start HAP session.
    HAPPairingPairVerifyStartSession(session_);
    return kHAPError_None;
}

/**
 * Pair Verify M3 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;         /**< kTLVType_State. */
    HAPTLV* encryptedDataTLV; /**< kTLVType_EncryptedData. */
} HAPPairingPairVerifyM3TLVs;

/**
 * Processes Pair Verify M3.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      scratchBytes         Free memory.
 * @param      numScratchBytes      Length of free memory buffer.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 * @return kHAPError_OutOfResources If the free memory buffer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyProcessM3(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        void* scratchBytes,
        size_t numScratchBytes,
        const HAPPairingPairVerifyM3TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairVerify.state == 3);
    HAPPrecondition(!session->state.pairVerify.error);
    HAPPrecondition(!session->hap.active);
    HAPPrecondition(scratchBytes);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->encryptedDataTLV);
    HAPPrecondition(tlvs->encryptedDataTLV->type == kHAPPairingTLVType_EncryptedData);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.7.3 M3: iOS Device -> Accessory -- `Verify Finish Request'

    HAPLogDebug(&logObject, "Pair Verify M3: Verify Finish Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Pair Verify M3: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Pair Setup M3: kTLVType_State has invalid length (%lu).",
               (unsigned long) tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 3) {
        HAPLog(&logObject, "Pair Verify M3: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_EncryptedData.
    if (!tlvs->encryptedDataTLV->value.bytes) {
        HAPLog(&logObject, "Pair Verify M3: kTLVType_EncryptedData missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->encryptedDataTLV->value.numBytes < CHACHA20_POLY1305_TAG_BYTES) {
        HAPLog(&logObject,
               "Pair Verify M3: kTLVType_EncryptedData has invalid length (%lu).",
               (unsigned long) tlvs->encryptedDataTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Verify auth tag and decrypt.
    HAPLogBufferDebug(
            &logObject,
            tlvs->encryptedDataTLV->value.bytes,
            tlvs->encryptedDataTLV->value.numBytes,
            "Pair Verify M3: kTLVType_EncryptedData.");
    void* bytes = (void*) (uintptr_t) tlvs->encryptedDataTLV->value.bytes;
    size_t numBytes = tlvs->encryptedDataTLV->value.numBytes - CHACHA20_POLY1305_TAG_BYTES;
    static const uint8_t nonce[] = "PV-Msg03";
    int e = HAP_chacha20_poly1305_decrypt(
            &((uint8_t*) bytes)[numBytes],
            bytes,
            bytes,
            numBytes,
            nonce,
            sizeof nonce - 1,
            session->state.pairVerify.SessionKey);
    if (e) {
        HAPAssert(e == -1);
        HAPLog(&logObject, "Pair Verify M3: Failed to decrypt kTLVType_EncryptedData.");
        session->state.pairVerify.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }

    // Parse sub-TLV.
    HAPTLV identifierTLV, signatureTLV;
    identifierTLV.type = kHAPPairingTLVType_Identifier;
    signatureTLV.type = kHAPPairingTLVType_Signature;
    {
        HAPTLVReaderRef subReader;
        HAPTLVReaderCreate(&subReader, bytes, numBytes);

        err = HAPTLVReaderGetAll(&subReader, (HAPTLV* const[]) { &identifierTLV, &signatureTLV, NULL });
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        // Validate kTLVType_Identifier.
        if (!identifierTLV.value.bytes) {
            HAPLog(&logObject, "Pair Verify M3: kTLVType_Identifier missing.");
            return kHAPError_InvalidData;
        }
        if (identifierTLV.value.numBytes > sizeof(HAPPairingID)) {
            HAPLog(&logObject,
                   "Pair Verify M3: kTLVType_Identifier has invalid length (%lu).",
                   (unsigned long) identifierTLV.value.numBytes);
            return kHAPError_InvalidData;
        }

        // Validate kTLVType_Signature.
        if (!signatureTLV.value.bytes) {
            HAPLog(&logObject, "Pair Verify M3: kTLVType_Signature missing.");
            return kHAPError_InvalidData;
        }
        if (signatureTLV.value.numBytes != ED25519_BYTES) {
            HAPLog(&logObject,
                   "Pair Verify M3: kTLVType_Signature has invalid length (%lu).",
                   (unsigned long) signatureTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
    }

    // Fetch pairing ID.
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPRawBufferCopyBytes(
            pairing.identifier.bytes, HAPNonnullVoid(identifierTLV.value.bytes), identifierTLV.value.numBytes);
    HAPAssert(identifierTLV.value.numBytes <= UINT8_MAX);
    pairing.numIdentifierBytes = (uint8_t) identifierTLV.value.numBytes;
    HAPPlatformKeyValueStoreKey key;
    bool found;
    err = HAPPairingFind(server->platform.keyValueStore, &pairing, &key, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        // Pairing not found.
        HAPLog(&logObject, "Pair Verify M3: Pairing not found.");
        session->state.pairVerify.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }
    session->state.pairVerify.pairingID = (int) key;

    void* iOSDeviceCvPK = HAPTLVScratchBufferAlloc(&scratchBytes, &numScratchBytes, X25519_BYTES);
    void* iOSDevicePairingID =
            HAPTLVScratchBufferAllocUnaligned(&scratchBytes, &numScratchBytes, identifierTLV.value.numBytes);
    void* accessoryCvPK = HAPTLVScratchBufferAllocUnaligned(&scratchBytes, &numScratchBytes, X25519_BYTES);
    if (!iOSDeviceCvPK || !iOSDevicePairingID || !accessoryCvPK) {
        HAPLog(&logObject,
               "Pair Verify M3: Not enough memory to allocate"
               " iOSDeviceCvPK / iOSDevicePairingID / AccessoryCvPK.");
        return kHAPError_OutOfResources;
    }

    // Construct iOSDeviceInfo: iOSDeviceCvPK, iOSDevicePairingID, AccessoryCvPK.
    HAPRawBufferCopyBytes(
            iOSDeviceCvPK,
            session->state.pairVerify.Controller_cv_PK,
            sizeof session->state.pairVerify.Controller_cv_PK);
    HAPRawBufferCopyBytes(iOSDevicePairingID, HAPNonnullVoid(identifierTLV.value.bytes), identifierTLV.value.numBytes);
    HAPRawBufferCopyBytes(accessoryCvPK, session->state.pairVerify.cv_PK, sizeof session->state.pairVerify.cv_PK);

    // Finalize info.
    void* infoBytes = iOSDeviceCvPK;
    size_t numInfoBytes = X25519_BYTES + identifierTLV.value.numBytes + X25519_BYTES;
    HAPLogSensitiveBufferDebug(&logObject, infoBytes, numInfoBytes, "Pair Verify M3: iOSDeviceInfo.");

    // Verify signature.
    HAPLogSensitiveBufferDebug(
            &logObject, signatureTLV.value.bytes, signatureTLV.value.numBytes, "Pair Verify M3: kTLVType_Signature.");
    e = HAP_ed25519_verify(signatureTLV.value.bytes, infoBytes, numInfoBytes, pairing.publicKey.value);
    if (e) {
        HAPAssert(e == -1);
        HAPLog(&logObject, "Pair Verify M3: iOSDeviceInfo signature is incorrect.");
        session->state.pairVerify.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }

    return kHAPError_None;
}

/**
 * Processes Pair Verify M4.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyGetM4(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairVerify.state == 4);
    HAPPrecondition(!session->state.pairVerify.error);
    HAPPrecondition(!session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.7.4 M4: Accessory -> iOS Device -- `Verify Finish Response'

    HAPLogDebug(&logObject, "Pair Verify M4: Verify Finish Response.");

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairVerify.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // BLE: Handle Pair Resume.
    if (session->transportType == kHAPTransportType_BLE) {
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.3.7.3 Initial SessionID
        HAPAssert(server->transports.ble);

        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

        void* sessionID = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, sizeof(HAPPairingBLESessionID));
        if (!sessionID) {
            HAPLog(&logObject, "Pair Verify M4: Not enough memory to allocate initial SessionID.");
            return kHAPError_OutOfResources;
        }

        // Derive initial session ID.
        static const uint8_t salt[] = "Pair-Verify-ResumeSessionID-Salt";
        static const uint8_t info[] = "Pair-Verify-ResumeSessionID-Info";
        HAP_hkdf_sha512(
                sessionID,
                sizeof(HAPPairingBLESessionID),
                session->state.pairVerify.cv_KEY,
                sizeof session->state.pairVerify.cv_KEY,
                salt,
                sizeof salt - 1,
                info,
                sizeof info - 1);
        HAPLogSensitiveBufferDebug(
                &logObject, sessionID, sizeof(HAPPairingBLESessionID), "Pair Verify M4: ResumeSessionID.");

        // Save shared secret.
        HAPNonnull(server->transports.ble)
                ->sessionCache.save(
                        server_, sessionID, session->state.pairVerify.cv_KEY, session->state.pairVerify.pairingID);
    }

    // Start HAP session.
    HAPPairingPairVerifyStartSession(session_);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairVerifyHandleWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(requestReader);

    HAPError err;

    // Parse request.
    HAPTLV stateTLV, publicKeyTLV, methodTLV, sessionIDTLV, encryptedDataTLV;
    stateTLV.type = kHAPPairingTLVType_State;
    publicKeyTLV.type = kHAPPairingTLVType_PublicKey;
    methodTLV.type = kHAPPairingTLVType_Method;
    sessionIDTLV.type = kHAPPairingTLVType_SessionID;
    encryptedDataTLV.type = kHAPPairingTLVType_EncryptedData;
    err = HAPTLVReaderGetAll(
            requestReader,
            (HAPTLV* const[]) { &stateTLV, &publicKeyTLV, &methodTLV, &sessionIDTLV, &encryptedDataTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPPairingPairVerifyReset(session_);
        return err;
    }

    // Get free memory.
    void* bytes;
    size_t maxBytes;
    HAPTLVReaderGetScratchBytes(requestReader, &bytes, &maxBytes);

    // If a subsequent Pair Verify request from the same controller occurs
    // in the middle of the Pair Verify procedure then the accessory must
    // immediately tear down the existing session with the controller and
    // must accept the newest request.
    // See HomeKit Accessory Protocol Specification R14
    // Section 5.7.4 M4: Accessory -> iOS Device -- `Verify Finish Response'
    if (stateTLV.value.bytes && stateTLV.value.numBytes == 1 && ((const uint8_t*) stateTLV.value.bytes)[0] == 1) {
        HAPPairingPairVerifyReset(session_);
    }

    // Process request.
    switch (session->state.pairVerify.state) {
        case 0: {
            session->state.pairVerify.state++;
            err = HAPPairingPairVerifyProcessM1(
                    server,
                    session_,
                    bytes,
                    maxBytes,
                    &(const HAPPairingPairVerifyM1TLVs) { .stateTLV = &stateTLV,
                                                          .publicKeyTLV = &publicKeyTLV,
                                                          .methodTLV = &methodTLV,
                                                          .sessionIDTLV = &sessionIDTLV,
                                                          .encryptedDataTLV = &encryptedDataTLV });
            if (err) {
                HAPAssert(err == kHAPError_InvalidData || err == kHAPError_OutOfResources);
            }
        } break;
        case 2: {
            session->state.pairVerify.state++;
            err = HAPPairingPairVerifyProcessM3(
                    server,
                    session_,
                    bytes,
                    maxBytes,
                    &(const HAPPairingPairVerifyM3TLVs) { .stateTLV = &stateTLV,
                                                          .encryptedDataTLV = &encryptedDataTLV });
            if (err) {
                HAPAssert(err == kHAPError_InvalidData || err == kHAPError_OutOfResources);
            }
        } break;
        default: {
            HAPLog(&logObject, "Received unexpected Pair Verify write in state M%d.", session->state.pairVerify.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairVerifyReset(session_);
        return err;
    }
    return kHAPError_None;
}

/**
 * Writes the error of a session.
 *
 * @param      server               Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no error is pending.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairVerifyGetErrorResponse(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);
    HAPPrecondition(session->state.pairVerify.error);

    HAPError err;

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairVerify.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Error.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Error,
                              .value = { .bytes = &session->state.pairVerify.error, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairVerifyHandleRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    // Handle pending error.
    if (session->state.pairVerify.error) {
        // Advance state.
        session->state.pairVerify.state++;

        err = HAPPairingPairVerifyGetErrorResponse(server, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairVerifyReset(session_);
            return err;
        }

        // Reset session.
        HAPPairingPairVerifyReset(session_);
        return kHAPError_None;
    }

    // Process request.
    switch (session->state.pairVerify.state) {
        case 1: {
            session->state.pairVerify.state++;
            if (session->state.pairVerify.method == kHAPPairingMethod_PairResume) {
                err = HAPPairingPairVerifyGetM2ForBLEPairResume(server, session_, responseWriter);
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                }
            } else {
                err = HAPPairingPairVerifyGetM2(server, session_, responseWriter);
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                }
            }
        } break;
        case 3: {
            session->state.pairVerify.state++;
            err = HAPPairingPairVerifyGetM4(server, session_, responseWriter);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
            }
        } break;
        default: {
            HAPLog(&logObject, "Received unexpected Pair Verify read in state M%u.", session->state.pairVerify.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairVerifyReset(session_);
        return err;
    }

    // Handle pending error.
    if (session->state.pairVerify.error) {
        err = HAPPairingPairVerifyGetErrorResponse(server, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairVerifyReset(session_);
            return err;
        }

        // Reset session.
        HAPPairingPairVerifyReset(session_);
        return kHAPError_None;
    }

    return kHAPError_None;
}
