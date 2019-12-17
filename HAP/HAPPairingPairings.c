// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "PairingPairings" };

void HAPPairingPairingsReset(HAPSessionRef* session_) {
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;

    // Reset Pairings state.
    HAPRawBufferZero(&session->state.pairings, sizeof session->state.pairings);
}

/**
 * Add Pairing M1 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;       /**< kTLVType_State. */
    HAPTLV* methodTLV;      /**< kTLVType_Method. */
    HAPTLV* identifierTLV;  /**< kTLVType_Identifier. */
    HAPTLV* publicKeyTLV;   /**< kTLVType_PublicKey. */
    HAPTLV* permissionsTLV; /**< kTLVType_Permissions. */
} HAPPairingPairingsAddPairingM1TLVs;

/**
 * Processes Add Pairing M1.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_Unknown        If persistent store access failed.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairingsAddPairingProcessM1(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        const HAPPairingPairingsAddPairingM1TLVs* tlvs) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 1);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_AddPairing);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->methodTLV);
    HAPPrecondition(tlvs->methodTLV->type == kHAPPairingTLVType_Method);
    HAPPrecondition(tlvs->identifierTLV);
    HAPPrecondition(tlvs->identifierTLV->type == kHAPPairingTLVType_Identifier);
    HAPPrecondition(tlvs->publicKeyTLV);
    HAPPrecondition(tlvs->publicKeyTLV->type == kHAPPairingTLVType_PublicKey);
    HAPPrecondition(tlvs->permissionsTLV);
    HAPPrecondition(tlvs->permissionsTLV->type == kHAPPairingTLVType_Permissions);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.10.1 M1: iOS Device -> Accessory -- `Add Pairing Request'

    HAPLogDebug(&logObject, "Add Pairing M1: Add Pairing Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Add Pairing M1: kTLVType_State has invalid length (%lu).",
               (unsigned long) tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 1) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Method.
    if (!tlvs->methodTLV->value.bytes) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Method missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->methodTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Add Pairing M1: kTLVType_Method has invalid length (%lu).",
               (unsigned long) tlvs->methodTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t method = ((const uint8_t*) tlvs->methodTLV->value.bytes)[0];
    if (method != kHAPPairingMethod_AddPairing) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Method invalid: %u.", method);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Identifier.
    if (!tlvs->identifierTLV->value.bytes) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Identifier missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->identifierTLV->value.numBytes > sizeof(HAPPairingID)) {
        HAPLog(&logObject,
               "Add Pairing M1: kTLVType_Identifier has invalid length (%lu).",
               (unsigned long) tlvs->identifierTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_PublicKey.
    if (!tlvs->publicKeyTLV->value.bytes) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Identifier missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->publicKeyTLV->value.numBytes != sizeof(HAPPairingPublicKey)) {
        HAPLog(&logObject,
               "Add Pairing M1: kTLVType_Identifier has invalid length (%lu).",
               (unsigned long) tlvs->publicKeyTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Permissions.
    if (!tlvs->permissionsTLV->value.bytes) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Permissions missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->permissionsTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Add Pairing M1: kTLVType_Permissions has invalid length (%lu).",
               (unsigned long) tlvs->permissionsTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t permissions = ((const uint8_t*) tlvs->permissionsTLV->value.bytes)[0];
    if (permissions & ~1) {
        HAPLog(&logObject, "Add Pairing M1: kTLVType_Permissions invalid: %u.", permissions);
        return kHAPError_InvalidData;
    }

    // Check if a pairing for the additional controller's pairing identifier exists.
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPRawBufferCopyBytes(
            &pairing.identifier.bytes,
            HAPNonnullVoid(tlvs->identifierTLV->value.bytes),
            tlvs->identifierTLV->value.numBytes);
    HAPAssert(tlvs->identifierTLV->value.numBytes <= UINT8_MAX);
    pairing.numIdentifierBytes = (uint8_t) tlvs->identifierTLV->value.numBytes;
    HAPPlatformKeyValueStoreKey key;
    bool found;
    err = HAPPairingFind(server->platform.keyValueStore, &pairing, &key, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (found) {
        // Check if the additional controller's long-term public key matches the
        // stored public key for the additional controller's pairing identifier.
        if (!HAPRawBufferAreEqual(
                    pairing.publicKey.value,
                    HAPNonnullVoid(tlvs->publicKeyTLV->value.bytes),
                    sizeof pairing.publicKey.value)) {
            HAPLog(&logObject,
                   "Add Pairing M1: Additional controller's long-term public key does not match "
                   "the stored public key for the additional controller's pairing identifier.");
            session->state.pairings.error = kHAPPairingError_Unknown;
            return kHAPError_None;
        }

        // Update the permissions of the controller.
        pairing.permissions = permissions;

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
                server->platform.keyValueStore,
                kHAPKeyValueStoreDomain_Pairings,
                key,
                pairingBytes,
                sizeof pairingBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            return err;
        }

        // If the admin controller pairing is removed, all pairings on the accessory must be removed.
        err = HAPAccessoryServerCleanupPairings(server_);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLog(&logObject, "Add Pairing M1: Failed to cleanup pairings.");
            session->state.pairings.error = kHAPPairingError_Unknown;
            return kHAPError_None;
        }
    } else {
        // Look for free pairing slot.
        for (key = 0; key < server->maxPairings; key++) {
            size_t numBytes;
            uint8_t pairingBytes
                    [sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
            err = HAPPlatformKeyValueStoreGet(
                    server->platform.keyValueStore,
                    kHAPKeyValueStoreDomain_Pairings,
                    key,
                    pairingBytes,
                    sizeof pairingBytes,
                    &numBytes,
                    &found);
            if (err) {
                HAPAssert(err == kHAPError_Unknown);
                return err;
            }
            if (!found) {
                // Pairing found.
                break;
            }
            if (numBytes != sizeof pairingBytes) {
                HAPLog(&logObject, "Invalid pairing 0x%02X size %lu.", key, (unsigned long) numBytes);
                return kHAPError_Unknown;
            }
        }
        if (key == server->maxPairings) {
            HAPLog(&logObject, "Add Pairing M1: No space for additional pairings.");
            session->state.pairings.error = kHAPPairingError_MaxPeers;
            return kHAPError_None;
        }

        // Add pairing.
        HAPRawBufferZero(&pairing, sizeof pairing);
        HAPRawBufferCopyBytes(
                pairing.identifier.bytes,
                HAPNonnullVoid(tlvs->identifierTLV->value.bytes),
                tlvs->identifierTLV->value.numBytes);
        HAPAssert(tlvs->identifierTLV->value.numBytes <= UINT8_MAX);
        pairing.numIdentifierBytes = (uint8_t) tlvs->identifierTLV->value.numBytes;
        HAPRawBufferCopyBytes(
                pairing.publicKey.value,
                HAPNonnullVoid(tlvs->publicKeyTLV->value.bytes),
                tlvs->publicKeyTLV->value.numBytes);
        pairing.permissions = permissions;

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
                server->platform.keyValueStore,
                kHAPKeyValueStoreDomain_Pairings,
                key,
                pairingBytes,
                sizeof pairingBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLog(&logObject, "Add Pairing M1: Failed to add pairing.");
            session->state.pairings.error = kHAPPairingError_Unknown;
            return kHAPError_None;
        }
    }

    return kHAPError_None;
}

/**
 * Processes Add Pairing M2.
 *
 * @param      server               Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairingsAddPairingGetM2(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 2);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_AddPairing);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.10.2 M2: Accessory -> iOS Device -- `Add Pairing Response'

    HAPLogDebug(&logObject, "Add Pairing M2: Add Pairing Response.");

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairings.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // Reset Pairings session.
    HAPPairingPairingsReset(session_);
    return kHAPError_None;
}

/**
 * Remove Pairing M1 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;      /**< kTLVType_State. */
    HAPTLV* methodTLV;     /**< kTLVType_Method. */
    HAPTLV* identifierTLV; /**< kTLVType_Identifier. */
} HAPPairingPairingsRemovePairingM1TLVs;

/**
 * Processes Remove Pairing M1.
 *
 * @param      server               Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairingsRemovePairingProcessM1(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        const HAPPairingPairingsRemovePairingM1TLVs* tlvs) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 1);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_RemovePairing);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->methodTLV);
    HAPPrecondition(tlvs->methodTLV->type == kHAPPairingTLVType_Method);
    HAPPrecondition(tlvs->identifierTLV);
    HAPPrecondition(tlvs->identifierTLV->type == kHAPPairingTLVType_Identifier);

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.11.1 M1: iOS Device -> Accessory -- `Remove Pairing Request'

    HAPLogDebug(&logObject, "Remove Pairing M1: Remove Pairing Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "Remove Pairing M1: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Remove Pairing M1: kTLVType_State has invalid length (%lu).",
               (unsigned long) tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 1) {
        HAPLog(&logObject, "Remove Pairing M1: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Method.
    if (!tlvs->methodTLV->value.bytes) {
        HAPLog(&logObject, "Remove Pairing M1: kTLVType_Method missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->methodTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "Remove Pairing M1: kTLVType_Method has invalid length (%lu).",
               (unsigned long) tlvs->methodTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t method = ((const uint8_t*) tlvs->methodTLV->value.bytes)[0];
    if (method != kHAPPairingMethod_RemovePairing) {
        HAPLog(&logObject, "Remove Pairing M1: kTLVType_Method invalid: %u.", method);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Identifier.
    if (!tlvs->identifierTLV->value.bytes) {
        HAPLog(&logObject, "Remove Pairing M1: kTLVType_Identifier missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->identifierTLV->value.numBytes > sizeof(HAPPairingID)) {
        HAPLog(&logObject,
               "Remove Pairing M1: kTLVType_Identifier has invalid length (%lu).",
               (unsigned long) tlvs->identifierTLV->value.numBytes);
        return kHAPError_InvalidData;
    }

    // Store pairing to remove.
    HAPRawBufferCopyBytes(
            session->state.pairings.removedPairingID.bytes,
            HAPNonnullVoid(tlvs->identifierTLV->value.bytes),
            tlvs->identifierTLV->value.numBytes);
    session->state.pairings.removedPairingIDLength = tlvs->identifierTLV->value.numBytes;
    return kHAPError_None;
}

/**
 * Processes Remove Pairing M2.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the response will be sent.
 * @param      responseWriter       TLV writer for serializing the response.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_Unknown        If persistent store access failed.
 * @return kHAPError_OutOfResources If response writer does not have enough capacity.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairingsRemovePairingGetM2(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 2);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_RemovePairing);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.11.2 M2: Accessory -> iOS Device -- `Remove Pairing Response'

    HAPLogDebug(&logObject, "Remove Pairing M2: Remove Pairing Response.");

    // Find pairing.
    HAPPairing pairing;
    HAPRawBufferZero(&pairing, sizeof pairing);
    HAPRawBufferCopyBytes(
            &pairing.identifier.bytes,
            session->state.pairings.removedPairingID.bytes,
            session->state.pairings.removedPairingIDLength);
    HAPAssert(session->state.pairings.removedPairingIDLength <= UINT8_MAX);
    pairing.numIdentifierBytes = (uint8_t) session->state.pairings.removedPairingIDLength;
    HAPPlatformKeyValueStoreKey key;
    bool found;
    err = HAPPairingFind(server->platform.keyValueStore, &pairing, &key, &found);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }

    // If the pairing exists, remove RemovedControllerPairingIdentifier and its corresponding long-term public
    // key from persistent storage. If a pairing for RemovedControllerPairingIdentifier does not exist, the
    // accessory must return success.
    if (found) {
        // Remove the pairing.
        err = HAPPlatformKeyValueStoreRemove(server->platform.keyValueStore, kHAPKeyValueStoreDomain_Pairings, key);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLog(&logObject, "Remove Pairing M2: Failed to remove pairing.");
            session->state.pairings.error = kHAPPairingError_Unknown;
            return kHAPError_None;
        }

        // BLE: Remove all Pair Resume cache entries related to this pairing.
        if (server->transports.ble) {
            HAPNonnull(server->transports.ble)->sessionCache.invalidateEntriesForPairing(server_, (int) key);
        }

        // If the admin controller pairing is removed, all pairings on the accessory must be removed.
        err = HAPAccessoryServerCleanupPairings(server_);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPLog(&logObject, "Remove Pairing M2: Failed to cleanup pairings.");
            session->state.pairings.error = kHAPPairingError_Unknown;
            return kHAPError_None;
        }
    }

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairings.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // Reset Pairings session.
    HAPPairingPairingsReset(session_);
    return kHAPError_None;
}

/**
 * List Pairings M1 TLVs.
 */
typedef struct {
    HAPTLV* stateTLV;  /**< kTLVType_State. */
    HAPTLV* methodTLV; /**< kTLVType_Method. */
} HAPPairingPairingsListPairingsM1TLVs;

/**
 * Processes List Pairings M1.
 *
 * @param      server               Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      tlvs                 TLVs.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If a different request is expected in the current state.
 * @return kHAPError_InvalidData    If the controller sent a malformed request.
 */
HAP_RESULT_USE_CHECK
static HAPError HAPPairingPairingsListPairingsProcessM1(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        const HAPPairingPairingsListPairingsM1TLVs* tlvs) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 1);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_ListPairings);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(tlvs);
    HAPPrecondition(tlvs->stateTLV);
    HAPPrecondition(tlvs->stateTLV->type == kHAPPairingTLVType_State);
    HAPPrecondition(tlvs->methodTLV);
    HAPPrecondition(tlvs->methodTLV->type == kHAPPairingTLVType_Method);

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.12.1 M1: iOS Device -> Accessory -- `List Pairings Request'

    HAPLogDebug(&logObject, "List Pairings M1: List Pairings Request.");

    // Validate kTLVType_State.
    if (!tlvs->stateTLV->value.bytes) {
        HAPLog(&logObject, "List Pairings M1: kTLVType_State missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->stateTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "List Pairings M1: kTLVType_State has invalid length (%lu).",
               (unsigned long) tlvs->stateTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t state = ((const uint8_t*) tlvs->stateTLV->value.bytes)[0];
    if (state != 1) {
        HAPLog(&logObject, "List Pairings M1: kTLVType_State invalid: %u.", state);
        return kHAPError_InvalidData;
    }

    // Validate kTLVType_Method.
    if (!tlvs->methodTLV->value.bytes) {
        HAPLog(&logObject, "List Pairings M1: kTLVType_Method missing.");
        return kHAPError_InvalidData;
    }
    if (tlvs->methodTLV->value.numBytes != 1) {
        HAPLog(&logObject,
               "List Pairings M1: kTLVType_Method has invalid length (%lu).",
               (unsigned long) tlvs->methodTLV->value.numBytes);
        return kHAPError_InvalidData;
    }
    uint8_t method = ((const uint8_t*) tlvs->methodTLV->value.bytes)[0];
    if (method != kHAPPairingMethod_ListPairings) {
        HAPLog(&logObject, "List Pairings M1: kTLVType_Method invalid: %u.", method);
        return kHAPError_InvalidData;
    }

    return kHAPError_None;
}

typedef struct {
    HAPTLVWriterRef* responseWriter;
    bool needsSeparator;
    HAPError err;
} ListPairingsEnumerateContext;

HAP_RESULT_USE_CHECK
static HAPError ListPairingsEnumerateCallback(
        void* _Nullable context,
        HAPPlatformKeyValueStoreRef keyValueStore,
        HAPPlatformKeyValueStoreDomain domain,
        HAPPlatformKeyValueStoreKey key,
        bool* shouldContinue) {
    HAPPrecondition(context);
    ListPairingsEnumerateContext* arguments = context;
    HAPPrecondition(keyValueStore);
    HAPPrecondition(arguments->responseWriter);
    HAPPrecondition(!arguments->err);
    HAPPrecondition(domain == kHAPKeyValueStoreDomain_Pairings);
    HAPPrecondition(shouldContinue);

    HAPError err;

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
    if (pairing.numIdentifierBytes > sizeof pairing.identifier.bytes) {
        HAPLogError(&logObject, "Invalid pairing 0x%02X ID size %u.", key, pairing.numIdentifierBytes);
        return kHAPError_Unknown;
    }

    // Write separator if necessary.
    if (arguments->needsSeparator) {
        // kTLVType_Separator.
        err = HAPTLVWriterAppend(
                arguments->responseWriter,
                &(const HAPTLV) { .type = kHAPPairingTLVType_Separator, .value = { .bytes = NULL, .numBytes = 0 } });
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            arguments->err = err;
            *shouldContinue = false;
            return kHAPError_None;
        }
    }
    arguments->needsSeparator = true;

    // Write pairing.
    err = HAPTLVWriterAppend(
            arguments->responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Identifier,
                              .value = { .bytes = pairing.identifier.bytes, .numBytes = pairing.numIdentifierBytes } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        arguments->err = err;
        *shouldContinue = false;
        return kHAPError_None;
    }
    err = HAPTLVWriterAppend(
            arguments->responseWriter,
            &(const HAPTLV) {
                    .type = kHAPPairingTLVType_PublicKey,
                    .value = { .bytes = pairing.publicKey.value, .numBytes = sizeof pairing.publicKey.value } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        arguments->err = err;
        *shouldContinue = false;
        return kHAPError_None;
    }
    err = HAPTLVWriterAppend(
            arguments->responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Permissions,
                              .value = { .bytes = &pairing.permissions, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        arguments->err = err;
        *shouldContinue = false;
        return kHAPError_None;
    }

    return kHAPError_None;
}

/**
 * Processes List Pairings M2.
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
static HAPError HAPPairingPairingsListPairingsGetM2(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->state.pairings.state == 2);
    HAPPrecondition(session->state.pairings.method == kHAPPairingMethod_ListPairings);
    HAPPrecondition(!session->state.pairings.error);
    HAPPrecondition(session->hap.active);
    HAPPrecondition(responseWriter);

    HAPError err;

    // See HomeKit Accessory Protocol Specification R14
    // Section 5.12.2 M2: Accessory -> iOS Device -- `List Pairings Response'

    HAPLogDebug(&logObject, "List Pairings M2: List Pairings Response.");

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairings.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // List pairings.
    ListPairingsEnumerateContext context = { .responseWriter = responseWriter,
                                             .needsSeparator = false,
                                             .err = kHAPError_None };
    err = HAPPlatformKeyValueStoreEnumerate(
            server->platform.keyValueStore, kHAPKeyValueStoreDomain_Pairings, ListPairingsEnumerateCallback, &context);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        return err;
    }
    if (context.err) {
        return context.err;
    }

    // Reset Pairings session.
    HAPPairingPairingsReset(session_);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairingsHandleWrite(
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
    HAPTLV methodTLV, identifierTLV, publicKeyTLV, stateTLV, permissionsTLV;
    methodTLV.type = kHAPPairingTLVType_Method;
    identifierTLV.type = kHAPPairingTLVType_Identifier;
    publicKeyTLV.type = kHAPPairingTLVType_PublicKey;
    stateTLV.type = kHAPPairingTLVType_State;
    permissionsTLV.type = kHAPPairingTLVType_Permissions;
    err = HAPTLVReaderGetAll(
            requestReader,
            (HAPTLV* const[]) { &methodTLV, &identifierTLV, &publicKeyTLV, &stateTLV, &permissionsTLV, NULL });
    if (err) {
        HAPAssert(err == kHAPError_InvalidData);
        HAPPairingPairingsReset(session_);
        return err;
    }

    // Process request.
    switch (session->state.pairings.state) {
        case 0: {
            session->state.pairings.state++;
            if (!methodTLV.value.bytes || methodTLV.value.numBytes != 1) {
                err = kHAPError_InvalidData;
                break;
            }
            uint8_t method = ((const uint8_t*) methodTLV.value.bytes)[0];
            if (method != kHAPPairingMethod_AddPairing && method != kHAPPairingMethod_RemovePairing &&
                method != kHAPPairingMethod_ListPairings) {
                err = kHAPError_InvalidData;
                break;
            }
            session->state.pairings.method = method;

            // Admin access only.
            if (!session->hap.active) {
                HAPLog(&logObject, "Pairings M1: Rejected access from non-secure session.");
                session->state.pairings.error = kHAPPairingError_Authentication;
                err = kHAPError_None;
                break;
            }
            HAPAssert(session->hap.pairingID >= 0);
            bool found;
            size_t numBytes;
            uint8_t pairingBytes
                    [sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
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
                break;
            }
            if (!found) {
                err = kHAPError_Unknown;
                break;
            } else if (numBytes != sizeof pairingBytes) {
                HAPLog(&logObject,
                       "Invalid pairing 0x%02X size %lu.",
                       (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
                       (unsigned long) numBytes);
                err = kHAPError_Unknown;
                break;
            }
            HAPPairing pairing;
            HAPRawBufferZero(&pairing, sizeof pairing);
            HAPAssert(sizeof pairing.identifier.bytes == 36);
            HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
            pairing.numIdentifierBytes = pairingBytes[36];
            HAPAssert(sizeof pairing.publicKey.value == 32);
            HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
            pairing.permissions = pairingBytes[69];
            if (!(pairing.permissions & 0x01)) {
                HAPLog(&logObject, "Pairings M1: Rejected access from non-admin controller.");
                session->state.pairings.error = kHAPPairingError_Authentication;
                err = kHAPError_None;
                break;
            }

            switch (session->state.pairings.method) {
                case kHAPPairingMethod_AddPairing: {
                    err = HAPPairingPairingsAddPairingProcessM1(
                            server_,
                            session_,
                            &(const HAPPairingPairingsAddPairingM1TLVs) { .stateTLV = &stateTLV,
                                                                          .methodTLV = &methodTLV,
                                                                          .identifierTLV = &identifierTLV,
                                                                          .publicKeyTLV = &publicKeyTLV,
                                                                          .permissionsTLV = &permissionsTLV });
                    if (err) {
                        HAPAssert(err == kHAPError_InvalidData);
                    }
                } break;
                case kHAPPairingMethod_RemovePairing: {
                    err = HAPPairingPairingsRemovePairingProcessM1(
                            server_,
                            session_,
                            &(const HAPPairingPairingsRemovePairingM1TLVs) {
                                    .stateTLV = &stateTLV,
                                    .methodTLV = &methodTLV,
                                    .identifierTLV = &identifierTLV,
                            });
                    if (err) {
                        HAPAssert(err == kHAPError_InvalidData);
                    }
                } break;
                case kHAPPairingMethod_ListPairings: {
                    err = HAPPairingPairingsListPairingsProcessM1(
                            server_,
                            session_,
                            &(const HAPPairingPairingsListPairingsM1TLVs) { .stateTLV = &stateTLV,
                                                                            .methodTLV = &methodTLV });
                    if (err) {
                        HAPAssert(err == kHAPError_InvalidData);
                    }
                } break;
                default: {
                    HAPFatalError();
                } break;
            }
        } break;
        default: {
            HAPLog(&logObject, "Received unexpected Pairings write in state M%d.", session->state.pairings.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairingsReset(session_);
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
static HAPError HAPPairingPairingsGetErrorResponse(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);
    HAPPrecondition(session->state.pairings.error);

    HAPError err;

    // kTLVType_State.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_State,
                              .value = { .bytes = &session->state.pairings.state, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    // kTLVType_Error.
    err = HAPTLVWriterAppend(
            responseWriter,
            &(const HAPTLV) { .type = kHAPPairingTLVType_Error,
                              .value = { .bytes = &session->state.pairings.error, .numBytes = 1 } });
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        return err;
    }

    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPPairingPairingsHandleRead(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPTLVWriterRef* responseWriter) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(responseWriter);

    HAPError err;

    // Handle pending error.
    if (session->state.pairings.error) {
        // Advance state.
        session->state.pairings.state++;

        err = HAPPairingPairingsGetErrorResponse(server_, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairingsReset(session_);
            return err;
        }

        // Reset session.
        HAPPairingPairingsReset(session_);
        return kHAPError_None;
    }

    // Process request.
    switch (session->state.pairings.state) {
        case 1: {
            session->state.pairings.state++;

            // Admin access only.
            if (!session->hap.active) {
                HAPLog(&logObject, "Pairings M1: Rejected access from non-secure session.");
                session->state.pairings.error = kHAPPairingError_Authentication;
                err = kHAPError_None;
                break;
            }
            HAPAssert(session->hap.pairingID >= 0);
            bool found;
            size_t numBytes;
            uint8_t pairingBytes
                    [sizeof(HAPPairingID) + sizeof(uint8_t) + sizeof(HAPPairingPublicKey) + sizeof(uint8_t)];
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
                break;
            }
            if (!found) {
                err = kHAPError_Unknown;
                break;
            } else if (numBytes != sizeof pairingBytes) {
                HAPLog(&logObject,
                       "Invalid pairing 0x%02X size %lu.",
                       (HAPPlatformKeyValueStoreKey) session->hap.pairingID,
                       (unsigned long) numBytes);
                err = kHAPError_Unknown;
                break;
            }
            HAPPairing pairing;
            HAPRawBufferZero(&pairing, sizeof pairing);
            HAPAssert(sizeof pairing.identifier.bytes == 36);
            HAPRawBufferCopyBytes(pairing.identifier.bytes, &pairingBytes[0], 36);
            pairing.numIdentifierBytes = pairingBytes[36];
            HAPAssert(sizeof pairing.publicKey.value == 32);
            HAPRawBufferCopyBytes(pairing.publicKey.value, &pairingBytes[37], 32);
            pairing.permissions = pairingBytes[69];
            if (!(pairing.permissions & 0x01)) {
                HAPLog(&logObject, "Pairings M1: Rejected access from non-admin controller.");
                session->state.pairings.error = kHAPPairingError_Authentication;
                err = kHAPError_None;
                break;
            }

            switch (session->state.pairings.method) {
                case kHAPPairingMethod_AddPairing: {
                    err = HAPPairingPairingsAddPairingGetM2(server_, session_, responseWriter);
                    if (err) {
                        HAPAssert(err == kHAPError_OutOfResources);
                    }
                } break;
                case kHAPPairingMethod_RemovePairing: {
                    err = HAPPairingPairingsRemovePairingGetM2(server_, session_, responseWriter);
                    if (err) {
                        HAPAssert(err == kHAPError_OutOfResources);
                    }
                } break;
                case kHAPPairingMethod_ListPairings: {
                    err = HAPPairingPairingsListPairingsGetM2(server_, session_, responseWriter);
                    if (err) {
                        HAPAssert(err == kHAPError_OutOfResources);
                    }
                } break;
                default: {
                    HAPFatalError();
                } break;
            }
            break;
        }
        default: {
            HAPLog(&logObject, "Received unexpected Pairings read in state M%d.", session->state.pairings.state);
            err = kHAPError_InvalidState;
        } break;
    }
    if (err) {
        HAPPairingPairingsReset(session_);
        return err;
    }

    // Handle pending error.
    if (session->state.pairings.error) {
        err = HAPPairingPairingsGetErrorResponse(server_, session_, responseWriter);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPPairingPairingsReset(session_);
            return err;
        }

        // Reset session.
        HAPPairingPairingsReset(session_);
        return kHAPError_None;
    }

    return kHAPError_None;
}
