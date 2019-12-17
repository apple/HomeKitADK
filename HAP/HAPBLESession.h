// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_SESSION_H
#define HAP_BLE_SESSION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Initialize BLE specific part of a session.
 *
 * @param      server               Accessory server.
 * @param      session              Session of which the BLE specific part shall be initialized.
 */
void HAPBLESessionCreate(HAPAccessoryServerRef* server, HAPSessionRef* session);

/**
 * Deinitializes BLE specific part of a session.
 *
 * @param      bleSession           BLE session.
 */
void HAPBLESessionRelease(HAPBLESession* bleSession);

/**
 * Invalidates a BLE session.
 *
 * @param      server               Accessory server.
 * @param      bleSession           BLE Session.
 * @param      terminateLink        Whether or not the underlying connection should also be terminated.
 */
void HAPBLESessionInvalidate(HAPAccessoryServerRef* server, HAPBLESession* bleSession, bool terminateLink);

/**
 * Returns whether the session is terminal soon and no new transactions should be started.
 * This is to prevent ambiguities whether HAP-BLE transactions have been completed successfully, as the
 * HAP specification does not define at which stage a transaction actually executes.
 *
 * - If this function is called in response to a GATT request, it is safe to disconnect immediately.
 *   Otherwise, recently written data may still be transmitting and one should wait until
 *   #HAPBLESessionIsSafeToDisconnect returns true.
 *
 * @param      bleSession           BLE session.
 *
 * @return true                     If session is terminal soon.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLESessionIsTerminalSoon(const HAPBLESession* bleSession);

/**
 * Returns whether the session is terminal and must be disconnected.
 *
 * - If this function is called in response to a GATT request, it is safe to disconnect immediately.
 *   Otherwise, recently written data may still be transmitting and one should wait until
 *   #HAPBLESessionIsSafeToDisconnect returns true.
 *
 * @param      bleSession           BLE session.
 *
 * @return true                     If session is terminal.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPBLESessionIsTerminal(const HAPBLESession* bleSession);

/**
 * Returns whether it is safe to disconnect the Bluetooth link.
 *
 * @param      bleSession           BLE session.
 *
 * @return true                     If it is safe to disconnect the Bluetooth link.
 * @return false                    Otherwise. Data may still being sent.
 */
HAP_RESULT_USE_CHECK
bool HAPBLESessionIsSafeToDisconnect(const HAPBLESession* bleSession);

/**
 * Handles a sent GATT response.
 *
 * @param      server               Accessory server.
 * @param      session              Session.
 */
void HAPBLESessionDidSendGATTResponse(HAPAccessoryServerRef* server, HAPSessionRef* session);

/**
 * Handles a started HAP-BLE procedure.
 *
 * @param      server               Accessory server.
 * @param      session              Session.
 */
void HAPBLESessionDidStartBLEProcedure(HAPAccessoryServerRef* server, HAPSessionRef* session);

/**
 * Handles a started pairing procedure (Pair Verify / Add Pairing / Remove Pairing / List Pairing).
 * Pair Setup is not listed on purpose.
 *
 * @param      server               Accessory server.
 * @param      session              Session.
 * @param      pairingProcedureType Pairing procedure type.
 */
void HAPBLESessionDidStartPairingProcedure(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPPairingProcedureType pairingProcedureType);

/**
 * Handles a completed pairing procedure (Pair Verify / Add Pairing / Remove Pairing / List Pairing).
 * Pair Setup is not listed on purpose.
 *
 * @param      server               Accessory server.
 * @param      session              Session.
 * @param      pairingProcedureType Pairing procedure type.
 */
void HAPBLESessionDidCompletePairingProcedure(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPPairingProcedureType pairingProcedureType);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
