// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PAIRING_BLE_SESSION_CACHE_H
#define HAP_PAIRING_BLE_SESSION_CACHE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * BLE: Pair Resume cache session ID.
 */
typedef struct {
    uint8_t value[8]; /**< Session ID. */
} HAPPairingBLESessionID;
HAP_STATIC_ASSERT(sizeof(HAPPairingBLESessionID) == 8, HAPPairingBLESessionID);

/**
 * Retrieves the shared secret and pairing ID for a session ID, if available.
 *
 * - The stored information is invalidated after fetching.
 *
 * @param      server               Accessory server.
 * @param      sessionID            Session ID to retrieve data for.
 * @param[out] sharedSecret         Shared secret.
 * @param[out] pairingID            Pairing ID. -1, if session not found.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.7 Pair-Resume Procedure
 */
void HAPPairingBLESessionCacheFetch(
        HAPAccessoryServerRef* server,
        const HAPPairingBLESessionID* sessionID,
        uint8_t sharedSecret[_Nonnull X25519_SCALAR_BYTES],
        int* pairingID);

/**
 * Stores the shared secret and pairing ID for a session ID.
 *
 * @param      server               Accessory server.
 * @param      sessionID            Session ID.
 * @param      sharedSecret         Shared secret.
 * @param      pairingID            Pairing ID.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 7.3.7 Pair-Resume Procedure
 */
void HAPPairingBLESessionCacheSave(
        HAPAccessoryServerRef* server,
        const HAPPairingBLESessionID* sessionID,
        uint8_t sharedSecret[_Nonnull X25519_SCALAR_BYTES],
        int pairingID);

/**
 * Invalidates Pair Resume cache entries related to a pairing.
 *
 * @param      server               Accessory server.
 * @param      pairingID            Pairing ID.
 */
void HAPPairingBLESessionCacheInvalidateEntriesForPairing(HAPAccessoryServerRef* server, int pairingID);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
