// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "PairingPairSetup" };

void HAPPairingPairSetupResetForSession(HAPAccessoryServerRef* server_, HAPSessionRef* session_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    // Reset session-specific Pair Setup procedure state that is stored in shared memory.
    if (server->pairSetup.sessionThatIsCurrentlyPairing == session_) {
        bool keepSetupInfo = server->pairSetup.keepSetupInfo;
        HAPRawBufferZero(&server->pairSetup, sizeof server->pairSetup);
        HAPAccessorySetupInfoHandlePairingStop(server_, keepSetupInfo);
    }

    // Reset session-specific Pair Setup procedure state.
    HAPRawBufferZero(&session->state.pairSetup, sizeof session->state.pairSetup);
}

/**
 * Pair Setup M1 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;  /**< kTLVType_State. */
    HAPTLV* methodTLV; /**< kTLVType_Method. */
    HAPTLV* flagsTLV;  /**< kTLVType_Flags. */
} HAPPairingPairSetupM1TLVs;

/**
 * Processes Pair Setup M1.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairSetupProcessM1(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        const HAPPairingPairSetupM1TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 1);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->methodTLV);
    HAPPrecondition(tlvs->methodTLV->type == kHAPPairingTLVType_Method);
    HAPPrecondition(tlvs->flagsTLV);
    HAPPrecondition(tlvs->flagsTLV->type == kHAPPairingTLVType_Flags);

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.1 M1: iOS Device -> Accessory -- `SRP Start Request'

    HAPLogDebug(&logObject, "Pair Setup M1: SRP Start Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_State has invalid length (%zu).", tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 1) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Method.
    if (!tlvs->methodTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_Method missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->methodTLV->value.numBytes != 1) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_Method has invalid length (%zu).", tlvs->methodTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t method = ((const uint8_t*) tlvs->methodTLV->value.bytes)[0];
    if (method != kHAPPairingMethod_PairSetupWithAuth && method != kHAPPairingMethod_PairSetup) {
        HAPLog(&logObject, "Pair Setup M1: kTLVType_Method invalid: %u.", method);
        return kHAPError_InvalidData;
    }

    // Store method.
    HAPLogDebug(&logObject, "Pair Setup M1: kTLVType_Method = %u.", method);
    session->state.pairSetup.method = method;

    // Validate and store kTLVType_Flags.
    if (tlvs->flagsTLV->value.bytes) {
        if (tlvs->flagsTLV->value.numBytes > sizeof(uint32_t)) {
            HAPLog(&logObject,
                   "Pair Setup M1: kTLVType_Flags has invalid length (%zu).",
                   tlvs->flagsTLV->value.numBytes);
            return kHAPError_InvalidData;
        }
        if (server->pairSetup.sessionThatIsCurrentlyPairing == session_) {
            server->pairSetup.flagsPresent = true;
            server->pairSetup.flags = HAPPairingReadFlags(tlvs->flagsTLV);
        }
    } else {
        if (server->pairSetup.sessionThatIsCurrentlyPairing == session_) {
            server->pairSetup.flagsPresent = false;
            server->pairSetup.flags = 0;
        }
    }

    return kHAPError_None;
}

/**
 * Processes Pair Setup M2.
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
static HAPError HAPPairingPairSetupGetM2(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 2);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.2 M2: Accessory -> iOS Device -- `SRP Start Response'

    HAPLogDebug(&logObject, "Pair Setup M2: SRP Start Response.");

    // Check if the accessory is already paired.
    if (!server->pairSetup.sessionThatIsCurrentlyPairing || HAPAccessoryServerIsPaired(server_)) {
        HAPLog(&logObject, "Pair Setup M2: Accessory is already paired.");
        session->state.pairSetup.error = kHAPPairingError_Unavailable;
        return kHAPError_None;
    }

    // Check if the accessory has received more than 100 unsuccessful authentication attempts.
    bool found;
    size_t numBytes;
    uint8_t numAuthAttemptsBytes[sizeof(uint8_t)];
    err = HAPPlatformKeyValueStoreGet(
            server->platform.keyValueStore,
            kHAPKeyValueStoreDomain_Configuration,
            kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts,
            numAuthAttemptsBytes,
            sizeof numAuthAttemptsBytes,
            &numBytes,
            &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (!found) {
        HAPRawBufferZero(numAuthAttemptsBytes, sizeof numAuthAttemptsBytes);
    } else if (numBytes != sizeof numAuthAttemptsBytes) {
        HAPLog(&logObject, "Invalid authentication attempts counter.");
        return kHAPError_Unknown;
    }
    uint8_t numAuthAttempts = numAuthAttemptsBytes[0];
    if (numAuthAttempts >= 100) {
        HAPLog(&logObject, "Pair Setup M2: Accessory has received more than 100 unsuccessful authentication attempts.");
        session->state.pairSetup.error = kHAPPairingError_MaxTries;
        return kHAPError_None;
    }

    // Check if the accessory is currently performing a Pair Setup procedure with a different controller.
    if (server->pairSetup.sessionThatIsCurrentlyPairing != session_) {
        HAPLog(&logObject,
               "Pair Setup M2: Accessory is performing a Pair Setup procedure with a different controller.");
        session->state.pairSetup.error = kHAPPairingError_Busy;
        return kHAPError_None;
    }

    // Get pairing flags.
    uint32_t otherFlags = 0;
    bool isTransient = false;
    bool isSplit = false;
    if (server->pairSetup.flagsPresent) {
        uint32_t flags = server->pairSetup.flags;
        if (flags & kHAPPairingFlag_Transient) {
            if (session->state.pairSetup.method == kHAPPairingMethod_PairSetupWithAuth) {
                HAPLog(&logObject,
                       "Pair Setup M2: Ignoring %s because Pair Setup with Auth was requested.",
                       "kPairingFlag_Transient");
            } else {
                HAPAssert(session->state.pairSetup.method == kHAPPairingMethod_PairSetup);
                isTransient = true;
            }
            flags &= ~(uint32_t) kHAPPairingFlag_Transient;
        }
        if (flags & kHAPPairingFlag_Split) {
            if (session->state.pairSetup.method == kHAPPairingMethod_PairSetupWithAuth) {
                HAPLog(&logObject,
                       "Pair Setup M2: Ignoring %s because Pair Setup with Auth was requested.",
                       "kPairingFlag_Split");
            } else {
                HAPAssert(session->state.pairSetup.method == kHAPPairingMethod_PairSetup);
                isSplit = true;
            }
            flags &= ~(uint32_t) kHAPPairingFlag_Split;
        }
        if (flags) {
            HAPLog(&logObject, "Pair Setup M2: Ignoring unrecognized kTLVType_Flags: 0x%8lX.", (unsigned long) flags);
        }
    }
    HAPLogDebug(
            &logObject,
            "Pair Setup M2: Processing using %s = %s / %s = %s.",
            "kPairingFlag_Transient",
            isTransient ? "true" : "false",
            "kPairingFlag_Split",
            isSplit ? "true" : "false");

    // Recover setup info if requested.
    HAPSetupInfo* _Nullable setupInfo =
            HAPAccessorySetupInfoGetSetupInfo(server_, /* restorePrevious: */ !isTransient && isSplit);
    if (!setupInfo) {
        HAPLog(&logObject, "Pair Setup M2: kPairingFlag_Split requested but no previous setup info found.");
        session->state.pairSetup.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }
    HAPLogBufferDebug(&logObject, setupInfo->salt, sizeof setupInfo->salt, "Pair Setup M2: salt.");
    HAPLogSensitiveBufferDebug(&logObject, setupInfo->verifier, sizeof setupInfo->verifier, "Pair Setup M2: verifier.");

    // Generate private key b.
    HAPPlatformRandomNumberFill(server->pairSetup.b, sizeof server->pairSetup.b);
    HAPLogSensitiveBufferDebug(&logObject, server->pairSetup.b, sizeof server->pairSetup.b, "Pair Setup M2: b.");

    // Derive public key B.
    HAP_srp_public_key(server->pairSetup.B, server->pairSetup.b, setupInfo->verifier);
    HAPLogBufferDebug(&logObject, server->pairSetup.B, sizeof server->pairSetup.B, "Pair Setup M2: B.");

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairSetup.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_PublicKey.
    // Skip leading zeros.
    size_t size = SRP_PUBLIC_KEY_BYTES;
    uint8_t* B = server->pairSetup.B;
    while (size && !(*B)) {
        B++;
        size--;
    }
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_PublicKey, .value = { .bytes = B, .numBytes = size } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Salt.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Salt,
                              .value = { .bytes = setupInfo->salt, .numBytes = sizeof setupInfo->salt } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Flags.
    uint32_t flags = otherFlags;
    if (isTransient && isSplit) {
        flags |= kHAPPairingFlag_Transient | kHAPPairingFlag_Split;
    } else if (isSplit) {
        flags |= kHAPPairingFlag_Split;
    }

    if (flags) {
        uint8_t flagsBytes[] = { HAPExpandLittleUInt32(flags) };
        err = HAPTLVWriterAppend(
                responseWriter,
                &(const HAPTLV) { .type = kHAPPairingTLVType_Flags,
                                  .value = { .bytes = flagsBytes, .numBytes = HAPPairingGetNumBytes(flags) } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    }

    return kHAPError_None;
}

/**
 * Pair Setup M3 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;     /**< kTLVType_State. */
    HAPTLV* publicKeyTLV; /**< kTLVType_PublicKey. */
    HAPTLV* proofTLV;     /**< kTLVType_Proof. */
} HAPPairingPairSetupM3TLVs;

/**
 * Processes Pair Setup M3.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairSetupProcessM3(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        const HAPPairingPairSetupM3TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->pairSetup.sessionThatIsCurrentlyPairing == session_);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 3);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->publicKeyTLV);
    HAPPrecondition(tlvs->publicKeyTLV->type == kHAPPairingTLVType_PublicKey);
    HAPPrecondition(tlvs->proofTLV);
    HAPPrecondition(tlvs->proofTLV->type == kHAPPairingTLVType_Proof);

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.3 M3: iOS Device -> Accessory -- `SRP Verify Request'

    HAPLogDebug(&logObject, "Pair Setup M3: SRP Verify Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_State has invalid length (%zu).", tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 3) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_PublicKey.
    if (!tlvs->publicKeyTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_PublicKey missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->publicKeyTLV->value.numBytes > sizeof server->pairSetup.A) {
        HAPLog(&logObject,
               "Pair Setup M3: kTLVType_PublicKey has invalid length (%zu).",
               tlvs->publicKeyTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Proof.
    if (!tlvs->proofTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_Proof missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->proofTLV->value.numBytes != sizeof server->pairSetup.M1) {
        HAPLog(&logObject, "Pair Setup M3: kTLVType_Proof has invalid length (%zu).", tlvs->proofTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Copy public key to A and zero-extend big-endian.
    void* A = &server->pairSetup.A[sizeof server->pairSetup.A - tlvs->publicKeyTLV->value.numBytes];
    HAPRawBufferZero(server->pairSetup.A, sizeof server->pairSetup.A - tlvs->publicKeyTLV->value.numBytes);
    HAPRawBufferCopyBytes(A, HAPNonnullVoid(tlvs->publicKeyTLV->value.bytes), tlvs->publicKeyTLV->value.numBytes);
    HAPLogBufferDebug(&logObject, server->pairSetup.A, sizeof server->pairSetup.A, "Pair Setup M3: A.");

    // Copy proof.
    HAPRawBufferCopyBytes(
            server->pairSetup.M1, HAPNonnullVoid(tlvs->proofTLV->value.bytes), tlvs->proofTLV->value.numBytes);
    HAPLogBufferDebug(&logObject, server->pairSetup.M1, sizeof server->pairSetup.M1, "Pair Setup M3: M1.");

    return kHAPError_None;
}

/**
 * Processes Pair Setup M4.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with Apple Auth Coprocessor or persistent store access failed.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairSetupGetM4(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->pairSetup.sessionThatIsCurrentlyPairing == session_);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 4);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.4 M4: Accessory -> iOS Device -- `SRP Verify Response'

    HAPLogDebug(&logObject, "Pair Setup M4: SRP Verify Response.");

    // Compute SRP shared secret key.
    {
        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);

        void* u = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, SRP_SCRAMBLING_PARAMETER_BYTES);
        void* S = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, SRP_PREMASTER_SECRET_BYTES);
        void* M1 = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, SRP_PROOF_BYTES);
        if (!u || !S || !M1) {
            HAPLog(&logObject, "Pair Setup M4: Not enough memory to allocate u / S / M1.");
            return kHAPError_OutOfResources;
        }

        HAP_srp_scrambling_parameter(u, server->pairSetup.A, server->pairSetup.B);
        HAPLogSensitiveBufferDebug(&logObject, u, SRP_SCRAMBLING_PARAMETER_BYTES, "Pair Setup M4: u.");

        bool restorePrevious = false;
        if (server->pairSetup.flagsPresent) {
            restorePrevious = !(server->pairSetup.flags & kHAPPairingFlag_Transient) &&
                              server->pairSetup.flags & kHAPPairingFlag_Split;
        }
        HAPSetupInfo* _Nullable setupInfo = HAPAccessorySetupInfoGetSetupInfo(server_, restorePrevious);
        HAPAssert(setupInfo);

        int e = HAP_srp_premaster_secret(S, server->pairSetup.A, server->pairSetup.b, u, setupInfo->verifier);
        if (e) {
            HAPAssert(e == 1);
            // Illegal key A.
            HAPLog(&logObject, "Pair Setup M4: Illegal key A.");
            session->state.pairSetup.error = kHAPPairingError_Authentication;
            return kHAPError_None;
        }
        HAPLogSensitiveBufferDebug(&logObject, S, SRP_PREMASTER_SECRET_BYTES, "Pair Setup M4: S.");

        HAP_srp_session_key(server->pairSetup.K, S);
        HAPLogSensitiveBufferDebug(&logObject, server->pairSetup.K, sizeof server->pairSetup.K, "Pair Setup M4: K.");

        static const uint8_t userName[] = "Pair-Setup";
        HAP_srp_proof_m1(
                M1,
                userName,
                sizeof userName - 1,
                setupInfo->salt,
                server->pairSetup.A,
                server->pairSetup.B,
                server->pairSetup.K);
        HAPLogSensitiveBufferDebug(&logObject, M1, SRP_PROOF_BYTES, "Pair Setup M4: M1");

        // Verify the controller's SRP proof.
        if (!HAPRawBufferAreEqual(M1, server->pairSetup.M1, SRP_PROOF_BYTES)) {
            bool found;
            size_t numBytes;
            uint8_t numAuthAttemptsBytes[sizeof(uint8_t)];
            err = HAPPlatformKeyValueStoreGet(
                    server->platform.keyValueStore,
                    kHAPKeyValueStoreDomain_Configuration,
                    kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts,
                    numAuthAttemptsBytes,
                    sizeof numAuthAttemptsBytes,
                    &numBytes,
                    &found);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            if (!found) {
                HAPRawBufferZero(numAuthAttemptsBytes, sizeof numAuthAttemptsBytes);
            } else if (numBytes != sizeof numAuthAttemptsBytes) {
                HAPLog(&logObject, "Invalid authentication attempts counter.");
                return kHAPError_Unknown;
            }
            uint8_t numAuthAttempts = numAuthAttemptsBytes[0];
            HAPAssert(numAuthAttempts < UINT8_MAX);
            numAuthAttempts++;
            numAuthAttemptsBytes[0] = numAuthAttempts;
            err = HAPPlatformKeyValueStoreSet(
                    server->platform.keyValueStore,
                    kHAPKeyValueStoreDomain_Configuration,
                    kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts,
                    numAuthAttemptsBytes,
                    sizeof numAuthAttemptsBytes);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            HAPLog(&logObject,
                   "Pair Setup M4: Incorrect setup code. Unsuccessful authentication attempts = %u / 100.",
                   numAuthAttempts);
            session->state.pairSetup.error = kHAPPairingError_Authentication;
            return kHAPError_None;
        }

        // Reset authentication attempts counter.
        err = HAPPlatformKeyValueStoreRemove(
                server->platform.keyValueStore,
                kHAPKeyValueStoreDomain_Configuration,
                kHAPKeyValueStoreKey_Configuration_NumUnsuccessfulAuthAttempts);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // Generate accessory-side SRP proof.
        HAP_srp_proof_m2(server->pairSetup.M2, server->pairSetup.A, M1, server->pairSetup.K);
        HAPLogBufferDebug(&logObject, server->pairSetup.M2, sizeof server->pairSetup.M2, "Pair Setup M4: M2.");

        // Derive the symmetric session encryption key.
        static const uint8_t salt[] = "Pair-Setup-Encrypt-Salt";
        static const uint8_t info[] = "Pair-Setup-Encrypt-Info";
        HAP_hkdf_sha512(
                server->pairSetup.SessionKey,
                sizeof server->pairSetup.SessionKey,
                server->pairSetup.K,
                sizeof server->pairSetup.K,
                salt,
                sizeof salt - 1,
                info,
                sizeof info - 1);
        HAPLogSensitiveBufferDebug(
                &logObject,
                server->pairSetup.SessionKey,
                sizeof server->pairSetup.SessionKey,
                "Pair Setup M4: SessionKey");
    }

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairSetup.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Proof.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Proof,
                              .value = { .bytes = server->pairSetup.M2, .numBytes = sizeof server->pairSetup.M2 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_EncryptedData.
    if (session->state.pairSetup.method == kHAPPairingMethod_PairSetupWithAuth) {
        // Construct sub-TLV writer.
        HAPTLVWriterRef subWriter;
        {
            void* bytes;
            size_t maxBytes;
            HAPTLVWriterGetScratchBytes(responseWriter, &bytes, &maxBytes);
            if (maxBytes < CHACHA20_POLY1305_TAG_BYTES) {
                HAPLog(&logObject, "Pair Setup M4: Not enough memory for kTLVType_EncryptedData auth tag.");
                return kHAPError_OutOfResources;
            }
            maxBytes -= CHACHA20_POLY1305_TAG_BYTES;
            HAPTLVWriterCreate(&subWriter, bytes, maxBytes);
        }

        if (session->state.pairSetup.method == kHAPPairingMethod_PairSetupWithAuth) {
            HAPMFiAuth mfiAuth;

            {
                if (!server->platform.authentication.mfiHWAuth || !HAPAccessoryServerSupportsMFiHWAuth(server_)) {
                    HAPLog(&logObject, "Pair Setup M4: Apple Authentication Coprocessor is not available.");
                    return kHAPError_InvalidState;
                }
                HAPLogInfo(&logObject, "Using Apple Authentication Coprocessor.");
                mfiAuth.copyCertificate = HAPMFiHWAuthCopyCertificate;
                mfiAuth.createSignature = HAPMFiHWAuthCreateSignature;
            }

            // kTLVType_Signature.
            {
                void* bytes;
                size_t maxBytes;
                HAPTLVWriterGetScratchBytes(&subWriter, &bytes, &maxBytes);

                const size_t numChallengeBytes = 32;
                void* challengeBytes = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, numChallengeBytes);
                const size_t maxMFiProofBytes = maxBytes;
                void* mfiProofBytes = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, maxMFiProofBytes);
                if (!challengeBytes || !mfiProofBytes) {
                    HAPLog(&logObject, "Pair Setup M4: Not enough memory to allocate MFiChallenge / MFi Proof.");
                    return kHAPError_OutOfResources;
                }

                // Generate MFi challenge.
                static const uint8_t salt[] = "MFi-Pair-Setup-Salt";
                static const uint8_t info[] = "MFi-Pair-Setup-Info";
                HAP_hkdf_sha512(
                        challengeBytes,
                        numChallengeBytes,
                        server->pairSetup.K,
                        sizeof server->pairSetup.K,
                        salt,
                        sizeof salt - 1,
                        info,
                        sizeof info - 1);
                HAPLogSensitiveBufferDebug(
                        &logObject, challengeBytes, numChallengeBytes, "Pair Setup M4: MFiChallenge.");

                // Generate the MFi proof.
                size_t numMFiProofBytes;
                err = mfiAuth.createSignature(
                        server_, challengeBytes, numChallengeBytes, mfiProofBytes, maxMFiProofBytes, &numMFiProofBytes);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    return err;
                }
                HAPLogSensitiveBufferDebug(
                        &logObject, mfiProofBytes, numMFiProofBytes, "Pair Setup M4: kTLVType_Signature.");

                // kTLVType_Signature.
                err = HAPTLVWriterAppend(
                        &subWriter,
                        &(const HAPTLV) { .type = kHAPPairingTLVType_Signature,
                                          .value = { .bytes = mfiProofBytes, .numBytes = numMFiProofBytes } });
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                    return err;
                }
            }

            // kTLVType_Certificate.
            {
                void* bytes;
                size_t maxBytes;
                HAPTLVWriterGetScratchBytes(&subWriter, &bytes, &maxBytes);

                const size_t maxCertificateBytes = maxBytes;
                void* certificateBytes = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, maxCertificateBytes);
                if (!certificateBytes) {
                    HAPLog(&logObject, "Pair Setup M4: Not enough memory to allocate Accessory Certificate.");
                    return kHAPError_OutOfResources;
                }

                // Read the Accessory Certificate.
                size_t numCertificateBytes;
                err = mfiAuth.copyCertificate(server_, certificateBytes, maxCertificateBytes, &numCertificateBytes);
                if (err) {
                    HAPAssert(err == kHAPError_Unknown);
                    return err;
                }
                HAPLogSensitiveBufferDebug(
                        &logObject, certificateBytes, numCertificateBytes, "Pair Setup M4: kTLVType_Certificate.");

                // kTLVType_Certificate.
                err = HAPTLVWriterAppend(
                        &subWriter,
                        &(const HAPTLV) { .type = kHAPPairingTLVType_Certificate,
                                          .value = { .bytes = certificateBytes, .numBytes = numCertificateBytes } });
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                    return err;
                }
            }
        }

        // Encrypt the sub-TLV.
        void* bytes;
        size_t numBytes;
        HAPTLVWriterGetBuffer(&subWriter, &bytes, &numBytes);
        static const uint8_t nonce[] = "PS-Msg04";
        HAP_chacha20_poly1305_encrypt(
                &((uint8_t*) bytes)[numBytes],
                bytes,
                bytes,
                numBytes,
                nonce,
                sizeof nonce - 1,
                server->pairSetup.SessionKey);
        numBytes += CHACHA20_POLY1305_TAG_BYTES;
        HAPLogBufferDebug(&logObject, bytes, numBytes, "Pair Setup M4: kTLVType_EncryptedData.");

        // kTLVType_EncryptedData.
        err = HAPTLVWriterAppend(
                responseWriter,
                &(const HAPTLV) { .type = kHAPPairingTLVType_EncryptedData,
                                  .value = { .bytes = bytes, .numBytes = numBytes } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            return err;
        }
    }
    if (session->state.pairSetup.method == kHAPPairingMethod_PairSetup && server->pairSetup.flagsPresent &&
        server->pairSetup.flags & kHAPPairingFlag_Transient) {
        // Initialize HAP session.
        HAPRawBufferZero(&session->hap, sizeof session->hap);

        // Derive encryption keys.
        static const uint8_t salt[] = "SplitSetupSalt";
        {
            static const uint8_t info[] = "AccessoryEncrypt-Control";
            HAP_hkdf_sha512(
                    session->hap.accessoryToController.controlChannel.key.bytes,
                    sizeof session->hap.accessoryToController.controlChannel.key.bytes,
                    server->pairSetup.K,
                    sizeof server->pairSetup.K,
                    salt,
                    sizeof salt - 1,
                    info,
                    sizeof info - 1);
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    session->hap.accessoryToController.controlChannel.key.bytes,
                    sizeof session->hap.accessoryToController.controlChannel.key.bytes,
                    "Transient Pair Setup Start Session: AccessoryEncryptKey");
        }
        {
            static const uint8_t info[] = "ControllerEncrypt-Control";
            HAP_hkdf_sha512(
                    session->hap.controllerToAccessory.controlChannel.key.bytes,
                    sizeof session->hap.controllerToAccessory.controlChannel.key.bytes,
                    server->pairSetup.K,
                    sizeof server->pairSetup.K,
                    salt,
                    sizeof salt - 1,
                    info,
                    sizeof info - 1);
            HAPLogSensitiveBufferDebug(
                    &logObject,
                    session->hap.controllerToAccessory.controlChannel.key.bytes,
                    sizeof session->hap.controllerToAccessory.controlChannel.key.bytes,
                    "Transient Pair Setup Start Session: ControllerEncryptKey");
        }
        session->hap.accessoryToController.controlChannel.nonce = 0;
        session->hap.controllerToAccessory.controlChannel.nonce = 0;

        // Activate session.
        session->hap.isTransient = true;
        session->hap.active = true;

        // Persist setup info for next Pair Setup procedure if requested.
        if (server->pairSetup.flags & kHAPPairingFlag_Split) {
            server->pairSetup.keepSetupInfo = true;
        } else {
            HAPLog(&logObject, "Transient Pair Setup procedure requested without kHAPPairingFlag_Split.");
        }

        // Reset Pair Setup procedure.
        HAPPairingPairSetupResetForSession(server_, session_);

        HAPLogInfo(&logObject, "Transient Pair Setup procedure completed.");

        // Inform application.
        if (server->callbacks.handleSessionAccept) {
            server->callbacks.handleSessionAccept(server_, session_, server->context);
        }
        if (server->transports.ble) {
            HAPNonnull(server->transports.ble)->peripheralManager.handleSessionAccept(server_, session_);
        }
    }

    return kHAPError_None;
}

/**
 * Pair Setup M5 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;         /**< kTLVType_State. */
    HAPTLV* encryptedDataTLV; /**< kTLVType_EncryptedData. */
} HAPPairingPairSetupM5TLVs;

/**
 * Processes Pair Setup M5.
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
static HAPError HAPPairingPairSetupProcessM5(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        void* scratchBytes,
        size_t numScratchBytes,
        const HAPPairingPairSetupM5TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->pairSetup.sessionThatIsCurrentlyPairing == session_);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 5);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->encryptedDataTLV);
    HAPPrecondition(tlvs->encryptedDataTLV->type == kHAPPairingTLVType_EncryptedData);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.5 M5: iOS Device -> Accessory -- `Exchange Request'

    HAPLogDebug(&logObject, "Pair Setup M5: Exchange Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M5: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject, "Pair Setup M5: kTLVType_State has invalid length (%zu).", tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 5) {
        HAPLog(&logObject, "Pair Setup M5: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_EncryptedData.
    if (!tlvs->encryptedDataTLV->value.bytes) {
        HAPLog(&logObject, "Pair Setup M5: kTLVType_EncryptedData missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->encryptedDataTLV->value.numBytes < CHACHA20_POLY1305_TAG_BYTES) {
        HAPLog(&logObject,
               "Pair Setup M5: kTLVType_EncryptedData has invalid length (%zu).",
               tlvs->encryptedDataTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Verify auth tag and decrypt.
    HAPLogBufferDebug(
            &logObject,
            tlvs->encryptedDataTLV->value.bytes,
            tlvs->encryptedDataTLV->value.numBytes,
            "Pair Setup M5: kTLVType_EncryptedData.");
    void* bytes = (void*) (uintptr_t) tlvs->encryptedDataTLV->value.bytes;
    size_t numBytes = tlvs->encryptedDataTLV->value.numBytes - CHACHA20_POLY1305_TAG_BYTES;
    static const uint8_t nonce[] = "PS-Msg05";
    int e = HAP_chacha20_poly1305_decrypt(
            &((uint8_t*) bytes)[numBytes],
            bytes,
            bytes,
            numBytes,
            nonce,
            sizeof nonce - 1,
            server->pairSetup.SessionKey);
    if (e) {
        HAPAssert(e == -1);
        HAPLog(&logObject, "Pair Setup M5: Failed to decrypt kTLVType_EncryptedData.");
        session->state.pairSetup.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }
    HAPLogSensitiveBufferDebug(&logObject, bytes, numBytes, "Pair Setup M5: kTLVType_EncryptedData (decrypted).");

    // Parse sub-TLV.
    HAPTLV identifierTLV, publicKeyTLV, signatureTLV;
    identifierTLV.type = kHAPPairingTLVType_Identifier;
    publicKeyTLV.type = kHAPPairingTLVType_PublicKey;
    signatureTLV.type = kHAPPairingTLVType_Signature;
    {
        HAPTLVReaderRef subReader;
        HAPTLVReaderCreate(&subReader, bytes, numBytes);

        err = HAPTLVReaderGetAll(&subReader, (HAPTLV* const[]) { &identifierTLV, &publicKeyTLV, &signatureTLV, NULL });
        if (err) {
            HAPAssert(err == kHAPError_InvalidData);
            return err;
        }

        // Validate kTLVType_Identifier.
        if (!identifierTLV.value.bytes) {
            HAPLog(&logObject, "Pair Setup M5: kTLVType_Identifier missing.");
            return kHAPError_InvalidData;
        }
        if (identifierTLV.value.numBytes > sizeof(HAPPairingID)) {
            HAPLog(&logObject,
                   "Pair Setup M5: kTLVType_Identifier has invalid length (%zu).",
                   identifierTLV.value.numBytes);
            return kHAPError_InvalidData;
        }

        // Validate kTLVType_PublicKey.
        if (!publicKeyTLV.value.bytes) {
            HAPLog(&logObject, "Pair Setup M5: kTLVType_PublicKey missing.");
            return kHAPError_InvalidData;
        }
        if (publicKeyTLV.value.numBytes != sizeof(HAPPairingPublicKey)) {
            HAPLog(&logObject,
                   "Pair Setup M5: kTLVType_PublicKey has invalid length (%zu).",
                   publicKeyTLV.value.numBytes);
            return kHAPError_InvalidData;
        }

        // Validate kTLVType_Signature.
        if (!signatureTLV.value.bytes) {
            HAPLog(&logObject, "Pair Setup M5: kTLVType_Signature missing.");
            return kHAPError_InvalidData;
        }
        if (signatureTLV.value.numBytes != ED25519_BYTES) {
            HAPLog(&logObject,
                   "Pair Setup M5: kTLVType_Signature has invalid length (%zu).",
                   signatureTLV.value.numBytes);
            return kHAPError_InvalidData;
        }
    }

    const size_t XLength = 32;
    void* X = HAPTLVScratchBufferAlloc(&scratchBytes, &numScratchBytes, XLength);
    void* pairingID = HAPTLVScratchBufferAllocUnaligned(&scratchBytes, &numScratchBytes, identifierTLV.value.numBytes);
    void* ltpk = HAPTLVScratchBufferAllocUnaligned(&scratchBytes, &numScratchBytes, ED25519_PUBLIC_KEY_BYTES);
    if (!X || !pairingID || !ltpk) {
        HAPLog(&logObject,
               "Pair Setup M5: Not enough memory to allocate iOSDeviceX / iOSDevicePairingID / iOSDeviceLTPK.");
        return kHAPError_OutOfResources;
    }

    // Derive iOSDeviceX from the SRP shared secret.
    static const uint8_t salt[] = "Pair-Setup-Controller-Sign-Salt";
    static const uint8_t info[] = "Pair-Setup-Controller-Sign-Info";
    HAP_hkdf_sha512(
            X, XLength, server->pairSetup.K, sizeof server->pairSetup.K, salt, sizeof salt - 1, info, sizeof info - 1);

    // Construct iOSDeviceInfo: iOSDeviceX, iOSDevicePairingID, iOSDeviceLTPK.
    HAPRawBufferCopyBytes(pairingID, HAPNonnullVoid(identifierTLV.value.bytes), identifierTLV.value.numBytes);
    HAPRawBufferCopyBytes(ltpk, HAPNonnullVoid(publicKeyTLV.value.bytes), publicKeyTLV.value.numBytes);

    // Finalize info.
    void* infoBytes = X;
    size_t numInfoBytes = XLength + identifierTLV.value.numBytes + ED25519_PUBLIC_KEY_BYTES;
    HAPLogSensitiveBufferDebug(&logObject, infoBytes, numInfoBytes, "Pair Setup M5: iOSDeviceInfo.");

    // Verify signature.
    HAPLogSensitiveBufferDebug(
            &logObject, signatureTLV.value.bytes, signatureTLV.value.numBytes, "Pair Setup M5: kTLVType_Signature.");
    e = HAP_ed25519_verify(signatureTLV.value.bytes, infoBytes, numInfoBytes, ltpk);
    if (e) {
        HAPAssert(e == -1);
        HAPLog(&logObject, "Pair Setup M5: iOSDeviceInfo signature is incorrect.");
        session->state.pairSetup.error = kHAPPairingError_Authentication;
        return kHAPError_None;
    }

    // Persistently save the iOSDevicePairingID and iOSDeviceLTPK as a pairing.
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPRawBufferCopyBytes(
            pairing.identifier.bytes, HAPNonnullVoid(identifierTLV.value.bytes), identifierTLV.value.numBytes);
    HAPAssert(identifierTLV.value.numBytes <= UINT8_MAX);
    pairing.numIdentifierBytes = (uint8_t) identifierTLV.value.numBytes;
    HAPRawBufferCopyBytes(
            pairing.publicKey.value, HAPNonnullVoid(publicKeyTLV.value.bytes), publicKeyTLV.value.numBytes);
    pairing.permissions = 0x01;

    uint8_t pairingBytes[sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
    HAPRawBufferZero(pairingBytes, sizeof pairingBytes);
    HAPAssert(sizeof pairing.identifier.bytes == 36);
    HAPAssert(pairing.numIdentifierBytes <= sizeof pairing.identifier.bytes);
    HAPRawBufferCopyBytes(&pairingBytes[0], pairing.identifier.bytes, pairing.numIdentifierBytes);
    pairingBytes[36] = (uint8_t) pairing.numIdentifierBytes;
    HAPAssert(sizeof pairing.publicKey.value == 32);
    HAPRawBufferCopyBytes(&pairingBytes[37], pairing.publicKey.value, 32);
    pairingBytes[69] = pairing.permissions;
    err = HAPPlatformKeyValueStoreSet(
            server->platform.keyValueStore, kHAPKeyValueStoreDomain_Pairings, 0, pairingBytes, sizeof pairingBytes);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    return kHAPError_None;
}

/**
 * Processes Pair Setup M6.
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
static HAPError HAPPairingPairSetupGetM6(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->pairSetup.sessionThatIsCurrentlyPairing == session_);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairSetup.state == 6);
    HAPPrecondition(!session->state.pairSetup.error);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.6.6 M6: Accessory -> iOS Device -- `Exchange Response'

    HAPLogDebug(&logObject, "Pair Setup M6: Exchange Response.");

    // Accessory Long Term Keys are already generated earlier.

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairSetup.state, .numBytes = 1 } });
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
            HAPLog(&logObject, "Pair Setup M4: Not enough memory for kTLVType_EncryptedData auth tag.");
            return kHAPError_OutOfResources;
        }
        maxBytes -= CHACHA20_POLY1305_TAG_BYTES;
        HAPTLVWriterCreate(&subWriter, bytes, maxBytes);
    }

    // kTLVType_Identifier.
    HAPDeviceIDString deviceID;
    err = HAPDeviceIDGetAsString(server->platform.keyValueStore, &deviceID);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    size_t numDeviceIDBytes = HAPStringGetNumBytes(deviceID.stringValue);
    err = HAPTLVWriterAppend(
            &subWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Identifier,
                              .value = { .bytes = deviceID.stringValue, .numBytes = numDeviceIDBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_PublicKey.
    HAPLogSensitiveBufferDebug(
            &logObject,
            server->identity.ed_LTSK.bytes,
            sizeof server->identity.ed_LTSK.bytes,
            "Pair Setup M6: ed_LTSK.");
    HAPLogSensitiveBufferDebug(
            &logObject, server->identity.ed_LTPK, sizeof server->identity.ed_LTPK, "Pair Setup M6: ed_LTPK.");
    err = HAPTLVWriterAppend(
            &subWriter,
            &(const HAPTLV) {
                    .type = kHAPPairingTLVType_PublicKey,
                    .value = { .bytes = server->identity.ed_LTPK, .numBytes = sizeof server->identity.ed_LTPK } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Signature.
    {
        void* bytes;
        size_t maxBytes;
        HAPTLVWriterGetScratchBytes(&subWriter, &bytes, &maxBytes);

        const size_t XLength = 32;
        void* X = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, XLength);
        void* pairingID = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, numDeviceIDBytes);
        void* ltpk = HAPTLVScratchBufferAllocUnaligned(&bytes, &maxBytes, ED25519_PUBLIC_KEY_BYTES);
        void* signature = HAPTLVScratchBufferAlloc(&bytes, &maxBytes, ED25519_BYTES);
        if (!X || !pairingID || !ltpk || !signature) {
            HAPLog(&logObject,
                   "Pair Setup M6: Not enough memory to allocate "
                   "AccessoryX / AccessoryPairingID / AccessoryLTPK / Signature.");
            return kHAPError_OutOfResources;
        }

        // Derive AccessoryX from the SRP shared secret.
        static const uint8_t salt[] = "Pair-Setup-Accessory-Sign-Salt";
        static const uint8_t info[] = "Pair-Setup-Accessory-Sign-Info";
        HAP_hkdf_sha512(
                X,
                XLength,
                server->pairSetup.K,
                sizeof server->pairSetup.K,
                salt,
                sizeof salt - 1,
                info,
                sizeof info - 1);

        // Construct AccessoryDeviceInfo: AccessoryX, AccessoryPairingID, AccessoryLTPK.
        HAPRawBufferCopyBytes(pairingID, deviceID.stringValue, numDeviceIDBytes);
        HAPRawBufferCopyBytes(ltpk, server->identity.ed_LTPK, sizeof server->identity.ed_LTPK);

        // Finalize info.
        void* infoBytes = X;
        size_t numInfoBytes = XLength + numDeviceIDBytes + ED25519_PUBLIC_KEY_BYTES;

        // Generate signature.
        HAP_ed25519_sign(signature, infoBytes, numInfoBytes, server->identity.ed_LTSK.bytes, server->identity.ed_LTPK);
        HAPLogSensitiveBufferDebug(&logObject, infoBytes, numInfoBytes, "Pair Setup M6: AccessoryDeviceInfo.");
        HAPLogSensitiveBufferDebug(&logObject, signature, ED25519_BYTES, "Pair Setup M6: kTLVType_Signature.");

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

    // Encrypt the sub-TLV.
    void* bytes;
    size_t numBytes;
    HAPTLVWriterGetBuffer(&subWriter, &bytes, &numBytes);
    static const uint8_t nonce[] = "PS-Msg06";
    HAP_chacha20_poly1305_encrypt(
            &((uint8_t*) bytes)[numBytes],
            bytes,
            bytes,
            numBytes,
            nonce,
            sizeof nonce - 1,
            server->pairSetup.SessionKey);
    numBytes += CHACHA20_POLY1305_TAG_BYTES;
    HAPLogBufferDebug(&logObject, bytes, numBytes, "Pair Setup M6: kTLVType_EncryptedData.");

    // kTLVType_EncryptedData.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_EncryptedData,
                              .value = { .bytes = bytes, .numBytes = numBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // Reset Pair Setup procedure.
    HAPPairingPairSetupResetForSession(server_, session_);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairSetupHandleWrite(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVReaderRef* requestReader) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(requestReader);

    HAPError err;

    // Parse request.
    HAPTLV methodTLV, publicKeyTLV, proofTLV, encryptedDataTLV, stateTLV, flagsTLV;
    methodTLV.type = kHAPPairingTLVType_Method;
    publicKeyTLV.type = kHAPPairingTLVType_PublicKey;
    proofTLV.type = kHAPPairingTLVType_Proof;
    encryptedDataTLV.type = kHAPPairingTLVType_EncryptedData;
    stateTLV.type = kHAPPairingTLVType_State;
    flagsTLV.type = kHAPPairingTLVType_Flags;
    err = HAPTLVReaderGetAll(
            requestReader,
            (HAPTLV* const[]) { &methodTLV, &publicKeyTLV, &proofTLV, &encryptedDataTLV, &stateTLV, &flagsTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPPairingPairSetupResetForSession(server_, session_);
        return err;
    }

    // Certain controllers sometimes forget that pairing attempt is in progress and restart Pair Setup procedure at M1.
    // When this situation happens, we would regularly reject the request.
    // However, followup issues lead to those controllers forgetting to send the Pair Setup M3 message
    // after the setup code has been entered by the user.
    // As a mitigation, we cancel an ongoing Pair Setup procedure if the same controller sends Pair Setup M1 again.
    // Observed on iOS 12.1.
    if (server->pairSetup.sessionThatIsCurrentlyPairing == session_ && stateTLV.value.bytes &&
        stateTLV.value.numBytes == 1 && ((const uint8_t*) stateTLV.value.bytes)[0] == 1) {
        HAPLog(&logObject, "Received Pair Setup M1 during ongoing Pair Setup procedure. Aborting previous procedure.");
        server->pairSetup.keepSetupInfo = true;
        HAPPairingPairSetupResetForSession(server_, session_);
        server->accessorySetup.state.keepSetupInfo = false;
    }

    // Try to claim Pair Setup procedure.
    if (!session->state.pairSetup.state && !HAPAccessoryServerIsPaired(server_)) {
        if (server->pairSetup.sessionThatIsCurrentlyPairing &&
            server->pairSetup.sessionThatIsCurrentlyPairing != session_) {
            HAPTime now = HAPPlatformClockGetCurrent();
            HAPTime deadline = server->pairSetup.operationStartTime + kHAPPairing_PairSetupProcedureTimeout;
            if (now >= deadline) {
                HAPLog(&logObject,
                       "Pair Setup: Resetting Pair Setup procedure after %llu seconds.",
                       (unsigned long long) ((now - server->pairSetup.operationStartTime) / HAPSecond));
                server->pairSetup.keepSetupInfo = true;
                HAPPairingPairSetupResetForSession(
                        server_, HAPNonnull(server->pairSetup.sessionThatIsCurrentlyPairing));
                server->accessorySetup.state.keepSetupInfo = false;
            }
        }
        if (!server->pairSetup.sessionThatIsCurrentlyPairing) {
            server->pairSetup.sessionThatIsCurrentlyPairing = session_;
            server->pairSetup.operationStartTime = HAPPlatformClockGetCurrent();
            HAPAccessorySetupInfoHandlePairingStart(server_);
        }
    }

    // Get free memory.
    void* bytes;
    size_t maxBytes;
    HAPTLVReaderGetScratchBytes(requestReader, &bytes, &maxBytes);

    // Process request.
    switch (session->state.pairSetup.state) {
        case 0: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupProcessM1(
                    server_,
                    session_,
                    &(const HAPPairingPairSetupM1TLVs) {
                            .stateTLV = &stateTLV, .methodTLV = &methodTLV, .flagsTLV = &flagsTLV });
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
            }
        } break;
        case 2: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupProcessM3(
                    server_,
                    session_,
                    &(const HAPPairingPairSetupM3TLVs) {
                            .stateTLV = &stateTLV, .publicKeyTLV = &publicKeyTLV, .proofTLV = &proofTLV });
            if (err) {
                HAPAssert(err == kHAPError_InvalidData);
            }
        } break;
        case 4: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupProcessM5(
                    server_,
                    session_,
                    bytes,
                    maxBytes,
                    &(const HAPPairingPairSetupM5TLVs) { .stateTLV = &stateTLV,
                                                         .encryptedDataTLV = &encryptedDataTLV });
            if (err) {
                HAPAssert(err == kHAPError_InvalidData || err == kHAPError_OutOfResources);
            }
        } break;
        default: {
            HAPLog(&logObject, "Received unexpected Pair Setup write in state M%u.", session->state.pairSetup.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairSetupResetForSession(server_, session_);
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
static HAPError HAPPairingPairSetupGetErrorResponse(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);
    HAPPrecondition(session->state.pairSetup.error);

    HAPError err;

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairSetup.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Error.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Error,
                              .value = { .bytes = &session->state.pairSetup.error, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairSetupHandleRead(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    // Handle pending error.
    if (session->state.pairSetup.error) {
        // Advance state.
        session->state.pairSetup.state++;

        err = HAPPairingPairSetupGetErrorResponse(server, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairSetupResetForSession(server, session_);
            return err;
        }

        // Reset session.
        HAPPairingPairSetupResetForSession(server, session_);
        return kHAPError_None;
    }

    // Process request.
    switch (session->state.pairSetup.state) {
        case 1: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupGetM2(server, session_, responseWriter);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_OutOfResources);
            }
        } break;
        case 3: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupGetM4(server, session_, responseWriter);
            if (err) {
                HAPAssert(err == kHAPError_Unknown || err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
            }
        } break;
        case 5: {
            session->state.pairSetup.state++;
            err = HAPPairingPairSetupGetM6(server, session_, responseWriter);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
            }
        } break;
        default: {
            HAPLog(&logObject, "Received unexpected Pair Setup read in state M%u.", session->state.pairSetup.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairSetupResetForSession(server, session_);
        return err;
    }

    // Handle pending error.
    if (session->state.pairSetup.error) {
        err = HAPPairingPairSetupGetErrorResponse(server, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairSetupResetForSession(server, session_);
            return err;
        }

        // Reset session.
        HAPPairingPairSetupResetForSession(server, session_);
        return kHAPError_None;
    }

    return kHAPError_None;
}
