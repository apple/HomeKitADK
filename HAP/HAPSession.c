// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "Session" };

void HAPSessionCreate(HAPAccessoryServerRef* server_, HAPSessionRef* session_, HAPTransportType transportType) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(transportType == kHAPTransportType_IP || transportType == kHAPTransportType_BLE);

    HAPLogDebug(&logObject, "%s", __func__);

    HAPRawBufferZero(session, sizeof *session);
    session->server = server_;
    session->transportType = transportType;

    // Initialize session state.
    HAPPairingPairVerifyReset(session_);
    HAPPairingPairingsReset(session_);

    // Initialize transport specific part.
    switch (transportType) {
        case kHAPTransportType_IP: {
            HAPAssert(server->transports.ip);
            return;
        }
        case kHAPTransportType_BLE: {
            HAPAssert(server->transports.ble);
            HAPNonnull(server->transports.ble)->session.create(server_, session_);
            return;
        }
    }
    HAPFatalError();
}

void HAPSessionRelease(HAPAccessoryServerRef* server_, HAPSessionRef* session_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    HAPLogDebug(&logObject, "%s", __func__);

    // Invalidate session.
    HAPSessionInvalidate(server_, session_, /* terminateLink: */ true);

    // Deinitialize transport specific part and reset session.
    switch (session->transportType) {
        case kHAPTransportType_IP: {
            HAPAssert(server->transports.ip);
            HAPRawBufferZero(session, sizeof *session);
            return;
        }
        case kHAPTransportType_BLE: {
            HAPAssert(server->transports.ble);
            HAPNonnull(server->transports.ble)->session.release(&session->_.ble);
            HAPRawBufferZero(session, sizeof *session);
            return;
        }
    }
    HAPFatalError();
}

void HAPSessionInvalidate(HAPAccessoryServerRef* server_, HAPSessionRef* session_, bool terminateLink) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    HAPLogDebug(&logObject, "%s", __func__);

    // Invalidate dependent state.
    if (session->hap.active) {
        // Invalidate transport specific state.
        switch (session->transportType) {
            case kHAPTransportType_IP: {
                HAPAssert(server->transports.ip);
                HAPNonnull(server->transports.ip)->session.invalidateDependentIPState(server_, session_);
            } break;
            case kHAPTransportType_BLE: {
                HAPAssert(server->transports.ble);
            } break;
        }

        // Inform application.
        if (server->transports.ble) {
            HAPNonnull(server->transports.ble)->peripheralManager.handleSessionInvalidate(server_, session_);
        }
        if (server->callbacks.handleSessionInvalidate) {
            server->callbacks.handleSessionInvalidate(server_, session_, server->context);
        }
    }

    // Clear security state.
    HAPPairingPairSetupResetForSession(server_, session_);
    HAPRawBufferZero(&session->hap, sizeof session->hap);
    HAPRawBufferZero(&session->state, sizeof session->state);

    // Re-initialize session state.
    HAPPairingPairVerifyReset(session_);
    HAPPairingPairingsReset(session_);

    // Invalidate transport-specific state.
    switch (session->transportType) {
        case kHAPTransportType_IP: {
            HAPAssert(server->transports.ip);
            return;
        }
        case kHAPTransportType_BLE: {
            HAPAssert(server->transports.ble);
            HAPNonnull(server->transports.ble)->session.invalidate(server_, &session->_.ble, terminateLink);
            return;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
bool HAPSessionIsSecured(const HAPSessionRef* session_) {
    HAPPrecondition(session_);
    const HAPSession* session = (const HAPSession*) session_;
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    HAPError err;

    // Pairing is active when the Pair Verify procedure ran through.
    if (!session->hap.active) {
        return false;
    }

    // Check for transient session.
    if (HAPSessionIsTransient(session_)) {
        return true;
    }

    // To detect concurrent Remove Pairing operations, the persistent cache is also checked.
    HAPAssert(session->hap.pairingID >= 0);
    bool found;
    size_t numBytes;
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Pairings,
            (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
            pairingBytes,
            sizeof pairingBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return false;
    }
    if (!found) {
        return false;
    } else if (numBytes != sizeof pairingBytes) {
        HAPLog(&logObject,
               "Invalid pairing 0x%02X size %lu.",
               (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
               (unsigned long) numBytes);
        return false;
    }

    return true;
}

HAP_RESULT_USE_CHECK
bool HAPSessionIsTransient(const HAPSessionRef* session_) {
    HAPPrecondition(session_);
    const HAPSession* session = (const HAPSession*) session_;

    if (!session->hap.active) {
        return false;
    }

    if (session->hap.isTransient) {
        HAPAssert(!HAPAccessoryServerIsPaired(session->server));
    }
    return session->hap.isTransient;
}

HAP_RESULT_USE_CHECK
bool HAPSessionControllerIsAdmin(const HAPSessionRef* session_) {
    HAPPrecondition(session_);
    const HAPSession* session = (const HAPSession*) session_;
    HAPPrecondition(session->server);
    HAPAccessoryServer* server = (HAPAccessoryServer*) session->server;

    HAPError err;

    if (!session->hap.active) {
        return false;
    }
    if (HAPSessionIsTransient(session_)) {
        return false;
    }

    HAPAssert(session->hap.pairingID >= 0);
    bool found;
    size_t numBytes;
    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Pairings,
            (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
            pairingBytes,
            sizeof pairingBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return false;
    }
    if (!found) {
        return false;
    } else if (numBytes != sizeof pairingBytes) {
        HAPLog(&logObject,
               "Invalid pairing 0x%02X size %lu.",
               (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
               (unsigned long) numBytes);
        return false;
    }
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPAssert(sizeof pairing.identifier.bytes == 36);
    HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
    pairing.numIdentifierBytes = pairingBytes[36];
    HAPAssert(sizeof pairing.publicKey.value == 32);
    HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
    pairing.permissions = pairingBytes[69];
    return (pairing.permissions & 0x01) == 0x01;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairSetupWrite(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(requestReader);

    HAPError err;

    bool wasPaired = HAPAccessoryServerIsPaired(server_);
    err = HAPPairingPairSetupHandleWrite(server_, session_, requestReader);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData || err == kHAPError_OutOfResources);
        return err;
    }
    bool isPaired = HAPAccessoryServerIsPaired(server_);

    if (wasPaired && !isPaired) {
        // Invalidate transport specific dependent state.
        switch (session->transportType) {
            case kHAPTransportType_IP: {
                HAPAssert(server->transports.ip);
                HAPNonnull(server->transports.ip)->session.invalidateDependentIPState(server_, session_);
            } break;
            case kHAPTransportType_BLE: {
                HAPAssert(server->transports.ble);
            } break;
        }
    }

    if (wasPaired != isPaired) {
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairSetupRead(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    bool wasPaired = HAPAccessoryServerIsPaired(server_);
    err = HAPPairingPairSetupHandleRead(server_, session_, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_Unknown || err == kHAPError_OutOfResources);
        return err;
    }
    bool isPaired = HAPAccessoryServerIsPaired(server_);

    if (wasPaired && !isPaired) {
        // Invalidate transport specific dependent state.
        switch (session->transportType) {
            case kHAPTransportType_IP: {
                HAPAssert(server->transports.ip);
                HAPNonnull(server->transports.ip)->session.invalidateDependentIPState(server_, session_);
            } break;
            case kHAPTransportType_BLE: {
                HAPAssert(server->transports.ble);
            } break;
        }
    }

    if (wasPaired != isPaired) {
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    }

    return kHAPError_None;
}

/**
 * Reports the start of a pairing procedure.
 *
 * @param      server_              Accessory server.
 * @param      session_             Session.
 * @param      pairingProcedureType Pairing procedure type.
 */
static void HAPSessionStartPairingProcedure(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPPairingProcedureType pairingProcedureType) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    switch (session->transportType) {
        case kHAPTransportType_IP: {
            HAPAssert(server->transports.ip);
            return;
        }
        case kHAPTransportType_BLE: {
            HAPAssert(server->transports.ble);
            HAPNonnull(server->transports.ble)
                    ->session.didStartPairingProcedure(server_, session_, pairingProcedureType);
            return;
        }
    }
    HAPFatalError();
}

/**
 * Reports the completion of a pairing procedure.
 *
 * @param      server_              Accessory server.
 * @param      session_             Session.
 * @param      pairingProcedureType Pairing procedure type.
 */
static void HAPSessionCompletePairingProcedure(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPPairingProcedureType pairingProcedureType) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    switch (session->transportType) {
        case kHAPTransportType_IP: {
            HAPAssert(server->transports.ip);
            return;
        }
        case kHAPTransportType_BLE: {
            HAPAssert(server->transports.ble);
            HAPNonnull(server->transports.ble)
                    ->session.didCompletePairingProcedure(server_, session_, pairingProcedureType);
            return;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairVerifyWrite(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(requestReader);

    HAPError err;

    if (session->state.pairVerify.state) {
        HAPSessionStartPairingProcedure(server, session_, kHAPPairingProcedureType_PairVerify);
    }

    err = HAPPairingPairVerifyHandleWrite(server, session_, requestReader);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData || err == kHAPError_OutOfResources);
        return err;
    }

    if (!session->state.pairVerify.state) {
        HAPSessionCompletePairingProcedure(server, session_, kHAPPairingProcedureType_PairVerify);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairVerifyRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    if (session->state.pairVerify.state) {
        HAPSessionStartPairingProcedure(server, session_, kHAPPairingProcedureType_PairVerify);
    }

    err = HAPPairingPairVerifyHandleRead(server, session_, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
        return err;
    }

    if (!session->state.pairVerify.state) {
        HAPSessionCompletePairingProcedure(server, session_, kHAPPairingProcedureType_PairVerify);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairingsWrite(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(requestReader);

    HAPError err;

    if (session->state.pairings.state) {
        HAPSessionStartPairingProcedure(server_, session_, kHAPPairingProcedureType_PairingPairings);
    }

    bool wasPaired = HAPAccessoryServerIsPaired(server_);
    err = HAPPairingPairingsHandleWrite(server_, session_, requestReader);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData);
        return err;
    }
    bool isPaired = HAPAccessoryServerIsPaired(server_);

    if (wasPaired && !isPaired) {
        // Invalidate transport specific dependent state.
        switch (session->transportType) {
            case kHAPTransportType_IP: {
                HAPAssert(server->transports.ip);
                HAPNonnull(server->transports.ip)->session.invalidateDependentIPState(server_, session_);
            } break;
            case kHAPTransportType_BLE: {
                HAPAssert(server->transports.ble);
            } break;
        }
    }

    if (wasPaired != isPaired) {
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    }

    if (!session->state.pairings.state) {
        HAPSessionCompletePairingProcedure(server_, session_, kHAPPairingProcedureType_PairingPairings);
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionHandlePairingsRead(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    if (session->state.pairings.state) {
        HAPSessionStartPairingProcedure(server_, session_, kHAPPairingProcedureType_PairingPairings);
    }

    bool wasPaired = HAPAccessoryServerIsPaired(server_);
    err = HAPPairingPairingsHandleRead(server_, session_, responseWriter);
    if (err) {
        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
        return err;
    }
    bool isPaired = HAPAccessoryServerIsPaired(server_);

    if (wasPaired && !isPaired) {
        // Invalidate transport specific dependent state.
        switch (session->transportType) {
            case kHAPTransportType_IP: {
                HAPAssert(server->transports.ip);
                HAPNonnull(server->transports.ip)->session.invalidateDependentIPState(server_, session_);
            } break;
            case kHAPTransportType_BLE: {
                HAPAssert(server->transports.ble);
            } break;
        }
    }

    if (wasPaired != isPaired) {
        HAPAccessoryServerDelegateScheduleHandleUpdatedState(server_);
    }

    if (!session->state.pairings.state) {
        HAPSessionCompletePairingProcedure(server_, session_, kHAPPairingProcedureType_PairingPairings);
    }

    return kHAPError_None;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HAP_RESULT_USE_CHECK
static HAPError
        Encrypt(HAPSessionChannelState* channel,
                void* encryptedBytes_,
                const void* plaintextBytes_,
                size_t numPlaintextBytes,
                const void* _Nullable aadBytes_,
                size_t numAADBytes) {
    HAPPrecondition(channel);
    HAPPrecondition(encryptedBytes_);
    uint8_t* encryptedBytes = encryptedBytes_;
    HAPPrecondition(plaintextBytes_);
    const uint8_t* plaintextBytes = plaintextBytes_;
    HAPPrecondition(!numAADBytes || aadBytes_);
    const uint8_t* _Nullable aadBytes = aadBytes_;

    // Encrypt message. Tag is appended to cipher text.
    uint8_t nonce[] = { HAPExpandLittleUInt64(channel->nonce) };
    if (aadBytes) {
        HAP_chacha20_poly1305_encrypt_aad(
                /* tag: */ &encryptedBytes[numPlaintextBytes],
                encryptedBytes,
                plaintextBytes,
                numPlaintextBytes,
                aadBytes,
                numAADBytes,
                nonce,
                sizeof nonce,
                channel->key.bytes);
    } else {
        HAP_chacha20_poly1305_encrypt(
                /* tag: */ &encryptedBytes[numPlaintextBytes],
                encryptedBytes,
                plaintextBytes,
                numPlaintextBytes,
                nonce,
                sizeof nonce,
                channel->key.bytes);
    }

    // Increment message counter.
    channel->nonce++;

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionEncryptControlMessage(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        void* encryptedBytes,
        const void* plaintextBytes,
        size_t numPlaintextBytes) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(encryptedBytes);
    HAPPrecondition(plaintextBytes);

    if (!session->hap.active) {
        HAPLog(&logObject, "Cannot encrypt message: Session not active.");
        return kHAPError_InvalidState;
    }

    return Encrypt(
            &session->hap.accessoryToController.controlChannel,
            encryptedBytes,
            plaintextBytes,
            numPlaintextBytes,
            /* aadBytes: */ NULL,
            /* numAADBytes: */ 0);
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionEncryptControlMessageWithAAD(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        void* encryptedBytes,
        const void* plaintextBytes,
        size_t numPlaintextBytes,
        const void* aadBytes,
        size_t numAADBytes) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(encryptedBytes);
    HAPPrecondition(plaintextBytes);
    HAPPrecondition(aadBytes);

    if (!session->hap.active) {
        HAPLog(&logObject, "Cannot encrypt message: Session not active.");
        return kHAPError_InvalidState;
    }

    return Encrypt(
            &session->hap.accessoryToController.controlChannel,
            encryptedBytes,
            plaintextBytes,
            numPlaintextBytes,
            aadBytes,
            numAADBytes);
}

//----------------------------------------------------------------------------------------------------------------------

HAP_RESULT_USE_CHECK
static HAPError
        Decrypt(HAPSessionChannelState* channel,
                void* plaintextBytes_,
                const void* encryptedBytes_,
                size_t numEncryptedBytes,
                const void* _Nullable aadBytes_,
                size_t numAADBytes) {
    HAPPrecondition(channel);
    HAPPrecondition(plaintextBytes_);
    uint8_t* plaintextBytes = plaintextBytes_;
    HAPPrecondition(encryptedBytes_);
    const uint8_t* encryptedBytes = encryptedBytes_;
    HAPPrecondition(!numAADBytes || aadBytes_);
    const uint8_t* aadBytes = aadBytes_;

    // Decrypt message. Tag is appended to cipher text.
    if (numEncryptedBytes < CHACHA20_POLY1305_TAG_BYTES) {
        HAPLog(&logObject, "Ciphertext not long enough for auth tag (length %zu).", numEncryptedBytes);
        return kHAPError_InvalidData;
    }

    uint8_t nonce[] = { HAPExpandLittleUInt64(channel->nonce) };
    if (aadBytes) {
        int e = HAP_chacha20_poly1305_decrypt_aad(
                /* tag: */ &encryptedBytes[numEncryptedBytes - CHACHA20_POLY1305_TAG_BYTES],
                plaintextBytes,
                encryptedBytes,
                /* c_len: */ numEncryptedBytes - CHACHA20_POLY1305_TAG_BYTES,
                aadBytes,
                numAADBytes,
                nonce,
                sizeof nonce,
                channel->key.bytes);
        if (e) {
            HAPAssert(e == -1);
            HAPLog(&logObject, "Decryption of message %llu failed.", (unsigned long long) channel->nonce);
            HAPLogSensitiveBuffer(&logObject, channel->key.bytes, sizeof channel->key.bytes, "Decryption key.");
            return kHAPError_InvalidData;
        }
    } else {
        int e = HAP_chacha20_poly1305_decrypt(
                /* tag: */ &encryptedBytes[numEncryptedBytes - CHACHA20_POLY1305_TAG_BYTES],
                plaintextBytes,
                encryptedBytes,
                /* c_len: */ numEncryptedBytes - CHACHA20_POLY1305_TAG_BYTES,
                nonce,
                sizeof nonce,
                channel->key.bytes);
        if (e) {
            HAPAssert(e == -1);
            HAPLog(&logObject, "Decryption of message %llu failed.", (unsigned long long) channel->nonce);
            HAPLogSensitiveBuffer(&logObject, channel->key.bytes, sizeof channel->key.bytes, "Decryption key.");
            return kHAPError_InvalidData;
        }
    }

    // Increment message counter.
    channel->nonce++;

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionDecryptControlMessage(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        void* plaintextBytes,
        const void* encryptedBytes,
        size_t numEncryptedBytes) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(plaintextBytes);
    HAPPrecondition(encryptedBytes);

    HAPError err;

    if (!session->hap.active) {
        HAPLog(&logObject, "Cannot decrypt message: Session not active.");
        return kHAPError_InvalidState;
    }

    err =
            Decrypt(&session->hap.controllerToAccessory.controlChannel,
                    plaintextBytes,
                    encryptedBytes,
                    numEncryptedBytes,
                    /* aadBytes: */ NULL,
                    /* numAADBytes: */ 0);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPRawBufferZero(&session->hap, sizeof session->hap);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPSessionDecryptControlMessageWithAAD(
        const HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        void* plaintextBytes,
        const void* encryptedBytes,
        size_t numEncryptedBytes,
        const void* aadBytes,
        size_t numAADBytes) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(plaintextBytes);
    HAPPrecondition(encryptedBytes);
    HAPPrecondition(aadBytes);

    HAPError err;

    if (!session->hap.active) {
        HAPLog(&logObject, "Cannot decrypt message: Session not active.");
        return kHAPError_InvalidState;
    }

    err =
            Decrypt(&session->hap.controllerToAccessory.controlChannel,
                    plaintextBytes,
                    encryptedBytes,
                    numEncryptedBytes,
                    aadBytes,
                    numAADBytes);
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPRawBufferZero(&session->hap, sizeof session->hap);
        return err;
    }

    return kHAPError_None;
}
