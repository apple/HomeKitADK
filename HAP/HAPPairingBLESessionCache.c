// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

/**
 * BLE: Pair Resume cache entry.
 */
typedef struct {
    HAPPairingBLESessionID sessionID;
    uint8_t sharedSecret[X25519_SCALAR_BYTES];
    int pairingID;
    uint32_t lastUsed; // 0: invalid, >0: timestamp
} HAPPairingBLESessionCacheEntry;

HAP_STATIC_ASSERT(
        sizeof(HAPBLESessionCacheElementRef) >= sizeof(HAPPairingBLESessionCacheEntry),
        HAPPairingBLESessionCacheEntry);

void HAPPairingBLESessionCacheFetch(
        HAPAccessoryServerRef* server_,
        const HAPPairingBLESessionID* sessionID,
        uint8_t sharedSecret[X25519_SCALAR_BYTES],
        int* pairingID) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->transports.ble);
    HAPPrecondition(sessionID);
    HAPPrecondition(sharedSecret);
    HAPPrecondition(pairingID);

    // Fetch session.
    for (size_t i = 0; i < server->ble.storage->numSessionCacheElements; i++) {
        HAPPairingBLESessionCacheEntry* cacheEntry =
                (HAPPairingBLESessionCacheEntry*) &server->ble.storage->sessionCacheElements[i];

        if (cacheEntry->lastUsed && HAPRawBufferAreEqual(&cacheEntry->sessionID, sessionID, sizeof *sessionID)) {
            HAPRawBufferCopyBytes(sharedSecret, cacheEntry->sharedSecret, sizeof cacheEntry->sharedSecret);
            *pairingID = cacheEntry->pairingID;
            HAPRawBufferZero(cacheEntry, sizeof *cacheEntry);
            return;
        }
    }

    // Not found.
    *pairingID = -1;
}

void HAPPairingBLESessionCacheSave(
        HAPAccessoryServerRef* server_,
        const HAPPairingBLESessionID* sessionID,
        uint8_t sharedSecret[X25519_SCALAR_BYTES],
        int pairingID) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->transports.ble);
    HAPPrecondition(sessionID);
    HAPPrecondition(sharedSecret);
    HAPPrecondition(pairingID >= 0);

    // Find free cache entry.
    size_t index = 0;
    {
        // Search least recently used.
        uint32_t min = UINT32_MAX;
        for (size_t i = 0; i < server->ble.storage->numSessionCacheElements; i++) {
            HAPPairingBLESessionCacheEntry* cacheEntry =
                    (HAPPairingBLESessionCacheEntry*) &server->ble.storage->sessionCacheElements[i];

            if (cacheEntry->lastUsed < min) {
                min = cacheEntry->lastUsed;
                index = i;
            }
        }
    }
    HAPPairingBLESessionCacheEntry* cacheEntry =
            (HAPPairingBLESessionCacheEntry*) &server->ble.storage->sessionCacheElements[index];

    // Save session.
    HAPRawBufferCopyBytes(&cacheEntry->sessionID, sessionID, sizeof *sessionID);
    HAPRawBufferCopyBytes(cacheEntry->sharedSecret, sharedSecret, sizeof cacheEntry->sharedSecret);
    cacheEntry->pairingID = pairingID;

    // Update least recently used.
    server->ble.sessionCacheTimestamp++;
    if (server->ble.sessionCacheTimestamp == 0) {
        // Overflow => reset time stamps.
        for (size_t i = 0; i < server->ble.storage->numSessionCacheElements; i++) {
            HAPPairingBLESessionCacheEntry* e =
                    (HAPPairingBLESessionCacheEntry*) &server->ble.storage->sessionCacheElements[i];

            if (e->lastUsed) {
                e->lastUsed = 1;
            }
        }
        server->ble.sessionCacheTimestamp = 2;
    }
    cacheEntry->lastUsed = server->ble.sessionCacheTimestamp;
}

void HAPPairingBLESessionCacheInvalidateEntriesForPairing(HAPAccessoryServerRef* server_, int pairingID) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->transports.ble);
    HAPPrecondition(pairingID >= 0);

    // Remove sessions for pairing. There may be multiple (e.g. pairing synced to multiple controllers).
    for (size_t i = 0; i < server->ble.storage->numSessionCacheElements; i++) {
        HAPPairingBLESessionCacheEntry* cacheEntry =
                (HAPPairingBLESessionCacheEntry*) &server->ble.storage->sessionCacheElements[i];

        if (cacheEntry->pairingID == pairingID) {
            HAPRawBufferZero(cacheEntry, sizeof *cacheEntry);
        }
    }
}
