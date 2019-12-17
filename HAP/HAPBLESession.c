// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLESession" };

// Set this flag to disable all BLE session timeouts.
#define DEBUG_DISABLE_TIMEOUTS (false)

/**
 * Timeout after which it is assumed that pending responses has been sent by the BLE stack.
 *
 * - BLE stacks typically send responses asynchronously and do not inform the application when it is fully sent.
 *   When we want to disconnect we decide to give pending responses time to be fully sent by the BLE stack.
 *   This timeout specifies how long we wait until pending responses have been sent.
 */
#define kHAPBLESession_SafeToDisconnectTimeout ((HAPTime)(200 * HAPMillisecond))

#if !DEBUG_DISABLE_TIMEOUTS
static void LinkTimerOrPairingProcedureTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPBLESession* bleSession = context;
    if (timer == bleSession->linkTimer) {
        HAPLogInfo(&logObject, "Link timeout expired.");
        bleSession->linkTimer = 0;
        bleSession->linkTimerDeadline = 0;
    } else if (timer == bleSession->pairingProcedureTimer) {
        HAPLogInfo(&logObject, "Pairing procedure timeout expired.");
        bleSession->pairingProcedureTimer = 0;
    } else {
        HAPPreconditionFailure();
    }

    // When link deadline or pairing procedure expires, invalidate security session and terminate BLE link.
    HAPSessionInvalidate(bleSession->server, bleSession->session, /* terminateLink: */ true);
}
#endif

static void SafeToDisconnectTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPBLESession* bleSession = context;
    HAPPrecondition(timer == bleSession->safeToDisconnectTimer);
    HAPLogDebug(&logObject, "Safe to disconnect expired.");
    bleSession->safeToDisconnectTimer = 0;
    HAPPrecondition(bleSession->server);
    HAPAccessoryServerRef* server_ = bleSession->server;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;

    bleSession->isSafeToDisconnect = true;

    if (HAPBLESessionIsTerminal(bleSession)) {
        HAPLogInfo(
                &logObject,
                "Disconnecting BLE connection - Security session marked terminal (safe to disconnect timer).");
        HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                blePeripheralManager, server->ble.connection.connectionHandle);
    } else if (server->state != kHAPAccessoryServerState_Running) {
        HAPLogInfo(&logObject, "Disconnecting BLE connection - Server is stopping (safe to disconnect timer).");
        HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                blePeripheralManager, server->ble.connection.connectionHandle);
    }
}

void HAPBLESessionCreate(HAPAccessoryServerRef* server, HAPSessionRef* session_) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);
    HAPBLESession* bleSession = &session->_.ble;

#if !DEBUG_DISABLE_TIMEOUTS
    HAPError err;
#endif

    HAPRawBufferZero(bleSession, sizeof *bleSession);

    bleSession->server = server;
    bleSession->session = session_;

// 40. After a Bluetooth link is established the first HAP procedure must begin within 10 seconds. Accessories must
// drop the Bluetooth Link if the controller fails to start a HAP procedure within 10 seconds of establishing the
// Bluetooth link.
// See HomeKit Accessory Protocol Specification R14
// Section 7.5 Testing Bluetooth LE Accessories
#if !DEBUG_DISABLE_TIMEOUTS
    bleSession->linkTimerDeadline = HAPPlatformClockGetCurrent() + 10 * HAPSecond;
    err = HAPPlatformTimerRegister(
            &bleSession->linkTimer, bleSession->linkTimerDeadline, LinkTimerOrPairingProcedureTimerExpired, bleSession);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLogError(&logObject, "Not enough timers available to register BLE link timer.");
        HAPFatalError();
    }
#endif
    bleSession->pairingProcedureTimer = 0;

    bleSession->isSafeToDisconnect = true;
    bleSession->safeToDisconnectTimer = 0;
}

void HAPBLESessionRelease(HAPBLESession* bleSession) {
    HAPPrecondition(bleSession);

    if (bleSession->linkTimer) {
        HAPPlatformTimerDeregister(bleSession->linkTimer);
        bleSession->linkTimer = 0;
        bleSession->linkTimerDeadline = 0;
    }
    if (bleSession->pairingProcedureTimer) {
        HAPPlatformTimerDeregister(bleSession->pairingProcedureTimer);
        bleSession->pairingProcedureTimer = 0;
    }
    if (bleSession->safeToDisconnectTimer) {
        HAPPlatformTimerDeregister(bleSession->safeToDisconnectTimer);
        bleSession->safeToDisconnectTimer = 0;
    }
}

void HAPBLESessionInvalidate(HAPAccessoryServerRef* server_, HAPBLESession* bleSession, bool terminateLink) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManager* blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(bleSession);

    if (bleSession->linkTimer) {
        HAPPlatformTimerDeregister(bleSession->linkTimer);
        bleSession->linkTimer = 0;
        bleSession->linkTimerDeadline = 0;
    }

    if (terminateLink) {
        bleSession->isTerminal = true;
        if (HAPBLESessionIsSafeToDisconnect(bleSession) && server->ble.connection.connected) {
            HAPLogInfo(&logObject, "Disconnecting connection - Security session marked terminal.");
            HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                    blePeripheralManager, server->ble.connection.connectionHandle);
        }
    }
    if (bleSession->pairingProcedureTimer) {
        HAPPlatformTimerDeregister(bleSession->pairingProcedureTimer);
        bleSession->pairingProcedureTimer = 0;
    }
}

HAP_RESULT_USE_CHECK
bool HAPBLESessionIsTerminalSoon(const HAPBLESession* bleSession) {
    HAPPrecondition(bleSession);

    if (bleSession->isTerminal) {
        return true;
    }

    if (bleSession->linkTimer) {
        HAPTime now = HAPPlatformClockGetCurrent();
        return bleSession->linkTimerDeadline < now || now - bleSession->linkTimerDeadline <= 200 * HAPMillisecond;
    }

    return false;
}

HAP_RESULT_USE_CHECK
bool HAPBLESessionIsTerminal(const HAPBLESession* bleSession) {
    HAPPrecondition(bleSession);

    return bleSession->isTerminal;
}

HAP_RESULT_USE_CHECK
bool HAPBLESessionIsSafeToDisconnect(const HAPBLESession* bleSession) {
    HAPPrecondition(bleSession);

    return bleSession->isSafeToDisconnect;
}

void HAPBLESessionDidSendGATTResponse(HAPAccessoryServerRef* server, HAPSessionRef* session_) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* p_sess = (HAPSession*) session_;
    HAPPrecondition(p_sess->transportType == kHAPTransportType_BLE);

    HAPError err;

    HAPBLESession* bleSession = &p_sess->_.ble;

    bleSession->isSafeToDisconnect = false;

    // Set safe to disconnect timer to ensure response is being sent before disconnecting.
    if (bleSession->safeToDisconnectTimer) {
        HAPPlatformTimerDeregister(bleSession->safeToDisconnectTimer);
        bleSession->safeToDisconnectTimer = 0;
    }
    err = HAPPlatformTimerRegister(
            &bleSession->safeToDisconnectTimer,
            HAPPlatformClockGetCurrent() + kHAPBLESession_SafeToDisconnectTimeout,
            SafeToDisconnectTimerExpired,
            bleSession);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        HAPLog(&logObject, "Not enough resources to consider safe to dc timer. Reporting safe to dc immediately!");
        bleSession->isSafeToDisconnect = true;
    }
}

void HAPBLESessionDidStartBLEProcedure(HAPAccessoryServerRef* server, HAPSessionRef* session_) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);

#if !DEBUG_DISABLE_TIMEOUTS
    HAPError err;
#endif
    HAPLogDebug(&logObject, "%s", __func__);

    HAPBLESession* bleSession = &session->_.ble;

    if (bleSession->isTerminal) {
        return;
    }

    // 40. After a Bluetooth link is established the first HAP procedure must begin within 10 seconds. Accessories must
    // drop the Bluetooth Link if the controller fails to start a HAP procedure within 10 seconds of establishing the
    // Bluetooth link.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories
    if (!HAPSessionIsSecured(session_)) {
        if (bleSession->linkTimer) {
            HAPPlatformTimerDeregister(bleSession->linkTimer);
            bleSession->linkTimer = 0;
            bleSession->linkTimerDeadline = 0;
        }
    }

    // 5. Must implement a security session timeout and tTerminate the security session after 30 seconds of inactivity
    // (i.e without any HAP Transactions).
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.2 Accessory Requirements
    if (HAPSessionIsSecured(session_)) {
#if !DEBUG_DISABLE_TIMEOUTS
        if (bleSession->linkTimer) {
            HAPPlatformTimerDeregister(bleSession->linkTimer);
            bleSession->linkTimer = 0;
            bleSession->linkTimerDeadline = 0;
        }
        bleSession->linkTimerDeadline = HAPPlatformClockGetCurrent() + 30 * HAPSecond;
        err = HAPPlatformTimerRegister(
                &bleSession->linkTimer,
                bleSession->linkTimerDeadline,
                LinkTimerOrPairingProcedureTimerExpired,
                bleSession);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLog(&logObject, "Not enough resources to start link timer. Invalidating session!");
            HAPSessionInvalidate(server, session_, /* terminateLink: */ true);
        }
#endif
    }
}

void HAPBLESessionDidStartPairingProcedure(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPPairingProcedureType pairingProcedureType HAP_UNUSED) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);

    HAPBLESession* bleSession = &session->_.ble;

#if !DEBUG_DISABLE_TIMEOUTS
    HAPError err;
#endif
    HAPLogDebug(&logObject, "%s", __func__);

    if (bleSession->isTerminal) {
        return;
    }

    // 39. Accessories must implement a 10 second HAP procedure timeout, all HAP procedures [...] must complete within
    // 10 seconds, if a procedure fails to complete within the procedure timeout the accessory must drop the security
    // session and also drop the Bluetooth link.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories
    if (!bleSession->pairingProcedureTimer) {
#if !DEBUG_DISABLE_TIMEOUTS
        err = HAPPlatformTimerRegister(
                &bleSession->pairingProcedureTimer,
                HAPPlatformClockGetCurrent() + 10 * HAPSecond,
                LinkTimerOrPairingProcedureTimerExpired,
                bleSession);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLog(&logObject, "Not enough resources to start pairing procedure timer. Invalidating session!");
            HAPSessionInvalidate(server, session_, /* terminateLink: */ true);
        }
#endif
    }
}

void HAPBLESessionDidCompletePairingProcedure(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session_,
        HAPPairingProcedureType pairingProcedureType) {
    HAPPrecondition(server);
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(session->transportType == kHAPTransportType_BLE);

    HAPBLESession* bleSession = &session->_.ble;

#if !DEBUG_DISABLE_TIMEOUTS
    HAPError err;
#endif
    HAPLogDebug(&logObject, "%s", __func__);

    if (bleSession->isTerminal) {
        return;
    }

    // Reset pairing procedure timeout.
    if (bleSession->pairingProcedureTimer) {
        HAPPlatformTimerDeregister(bleSession->pairingProcedureTimer);
        bleSession->pairingProcedureTimer = 0;
    }

    if (pairingProcedureType == kHAPPairingProcedureType_PairVerify && HAPSessionIsSecured(session_)) {
// Start session security timeout.
#if !DEBUG_DISABLE_TIMEOUTS
        if (bleSession->linkTimer) {
            HAPPlatformTimerDeregister(bleSession->linkTimer);
            bleSession->linkTimer = 0;
            bleSession->linkTimerDeadline = 0;
        }
        bleSession->linkTimerDeadline = HAPPlatformClockGetCurrent() + 30 * HAPSecond;
        err = HAPPlatformTimerRegister(
                &bleSession->linkTimer,
                bleSession->linkTimerDeadline,
                LinkTimerOrPairingProcedureTimerExpired,
                bleSession);
        if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPLog(&logObject, "Not enough resources to start link timer. Invalidating session!");
            HAPSessionInvalidate(server, session_, /* terminateLink: */ true);
        }
#endif
    }
}
