// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEAccessoryServer" };

static void Create(HAPAccessoryServerRef* server_, const HAPAccessoryServerOptions* options) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(options);

    HAPPrecondition(server->platform.ble.blePeripheralManager);

    // Initialize BLE storage.
    HAPPrecondition(options->ble.accessoryServerStorage);
    HAPBLEAccessoryServerStorage* storage = options->ble.accessoryServerStorage;
    HAPPrecondition(storage->gattTableElements);
    HAPPrecondition(storage->sessionCacheElements);
    HAPPrecondition(storage->numSessionCacheElements >= kHAPBLESessionCache_MinElements);
    HAPPrecondition(storage->session);
    HAPPrecondition(storage->procedures);
    HAPPrecondition(storage->numProcedures >= 1);
    HAPPrecondition(storage->procedureBuffer.bytes);
    HAPPrecondition(storage->procedureBuffer.numBytes >= 1);
    HAPRawBufferZero(storage->gattTableElements, storage->numGATTTableElements * sizeof *storage->gattTableElements);
    HAPRawBufferZero(
            storage->sessionCacheElements, storage->numSessionCacheElements * sizeof *storage->sessionCacheElements);
    HAPRawBufferZero(storage->session, sizeof *storage->session);
    HAPRawBufferZero(storage->procedures, storage->numProcedures * sizeof *storage->procedures);
    HAPRawBufferZero(storage->procedureBuffer.bytes, storage->procedureBuffer.numBytes);
    server->ble.storage = storage;

    // Copy advertising configuration.
    HAPPrecondition(options->ble.preferredAdvertisingInterval >= kHAPBLEAdvertisingInterval_Minimum);
    HAPPrecondition(options->ble.preferredAdvertisingInterval <= kHAPBLEAdvertisingInterval_Maximum);
    HAPPrecondition(options->ble.preferredNotificationDuration >= kHAPBLENotification_MinDuration);
    server->ble.adv.interval = options->ble.preferredAdvertisingInterval;
    server->ble.adv.ev_duration = options->ble.preferredNotificationDuration;
}

static void ValidateAccessory(const HAPAccessory* accessory) {
    HAPPrecondition(accessory);

    if (accessory->services) {
        for (size_t i = 0; accessory->services[i]; i++) {
            const HAPService* service = accessory->services[i];
            HAPPrecondition(service->iid <= UINT16_MAX);
            if (service->characteristics) {
                for (size_t j = 0; service->characteristics[j]; j++) {
                    const HAPBaseCharacteristic* characteristic = service->characteristics[j];
                    HAPPrecondition(characteristic->iid <= UINT16_MAX);
                }
            }
        }
    }
}

static void PrepareStart(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPBLEAccessoryServerStorage* storage = HAPNonnull(server->ble.storage);
    HAPRawBufferZero(storage->gattTableElements, storage->numGATTTableElements * sizeof *storage->gattTableElements);
    HAPRawBufferZero(
            storage->sessionCacheElements, storage->numSessionCacheElements * sizeof *storage->sessionCacheElements);
    HAPRawBufferZero(storage->session, sizeof *storage->session);
    HAPRawBufferZero(storage->procedures, storage->numProcedures * sizeof *storage->procedures);
    HAPRawBufferZero(storage->procedureBuffer.bytes, storage->procedureBuffer.numBytes);
}

static void Start(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    HAPAssert(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManager* blePeripheralManager = server->platform.ble.blePeripheralManager;

    // Set BD_ADDR.
    HAPMACAddress deviceAddress;
    err = HAPMACAddressGetRandomStaticBLEDeviceAddress(
            server_,
            /* bleInterface: */ NULL,
            &deviceAddress);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
    HAPAssert(sizeof deviceAddress == sizeof(HAPPlatformBLEPeripheralManagerDeviceAddress));
    HAPLogBufferInfo(&logObject, deviceAddress.bytes, sizeof deviceAddress.bytes, "BD_ADDR");
    HAPPlatformBLEPeripheralManagerDeviceAddress bdAddr;
    HAPRawBufferZero(&bdAddr, sizeof bdAddr);
    bdAddr.bytes[0] = deviceAddress.bytes[5];
    bdAddr.bytes[1] = deviceAddress.bytes[4];
    bdAddr.bytes[2] = deviceAddress.bytes[3];
    bdAddr.bytes[3] = deviceAddress.bytes[2];
    bdAddr.bytes[4] = deviceAddress.bytes[1];
    bdAddr.bytes[5] = deviceAddress.bytes[0];
    HAPPlatformBLEPeripheralManagerSetDeviceAddress(blePeripheralManager, &bdAddr);

    // Set GAP device name.
    const HAPAccessory* primaryAccessory = HAPNonnull(server->primaryAccessory);
    HAPAssert(primaryAccessory->name);
    HAPAssert(HAPStringGetNumBytes(primaryAccessory->name) <= 64);
    HAPPlatformBLEPeripheralManagerSetDeviceName(blePeripheralManager, primaryAccessory->name);

    // Register GATT db.
    HAPBLEPeripheralManagerRegister(server_);
}

static void TryStop(HAPAccessoryServerRef* server_, bool* didStop) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(didStop);

    *didStop = false;

    // Close all connections.
    if (server->ble.connection.connected) {
        HAPSession* session = (HAPSession*) server->ble.storage->session;
        if (HAPBLESessionIsSafeToDisconnect(&session->_.ble)) {
            HAPLogInfo(&logObject, "Disconnecting BLE connection - Server is shutting down.");
            HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                    blePeripheralManager, server->ble.connection.connectionHandle);
        } else {
            HAPLogInfo(&logObject, "Waiting for pending BLE data to be written.");
        }
        HAPLogInfo(&logObject, "Delaying shutdown. Waiting for BLE connection to terminate.");
        return;
    }

    // Stop listening.
    HAPPlatformBLEPeripheralManagerRemoveAllServices(blePeripheralManager);
    HAPPlatformBLEPeripheralManagerSetDelegate(blePeripheralManager, NULL);

    *didStop = true;
}

static void UpdateAdvertisingData(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPError err;

    HAPAssert(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;

    if (server->state == kHAPAccessoryServerState_Running) {
        // Fetch advertisement parameters.
        bool isActive;
        uint16_t advertisingInterval;
        uint8_t advertisingBytes[/* Maximum Bluetooth 4 limit: */ 31];
        size_t numAdvertisingBytes;
        uint8_t scanResponseBytes[/* Maximum Bluetooth 4 limit: */ 31];
        size_t numScanResponseBytes;
        err = HAPBLEAccessoryServerGetAdvertisingParameters(
                server_,
                &isActive,
                &advertisingInterval,
                advertisingBytes,
                sizeof advertisingBytes,
                &numAdvertisingBytes,
                scanResponseBytes,
                sizeof scanResponseBytes,
                &numScanResponseBytes);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }

        // Update advertisement.
        if (isActive) {
            HAPAssert(advertisingInterval);
            HAPPlatformBLEPeripheralManagerStartAdvertising(
                    blePeripheralManager,
                    advertisingInterval,
                    advertisingBytes,
                    numAdvertisingBytes,
                    numScanResponseBytes ? scanResponseBytes : NULL,
                    numScanResponseBytes);

            // Mark advertisement started.
            HAPBLEAccessoryServerDidStartAdvertising(server_);
        } else {
            HAPPlatformBLEPeripheralManagerStopAdvertising(blePeripheralManager);
        }
    } else {
        HAPLogInfo(&logObject, "Stopping advertisement - Server is shutting down.");
        HAPPlatformBLEPeripheralManagerStopAdvertising(blePeripheralManager);
    }
}

const HAPBLEAccessoryServerTransport kHAPAccessoryServerTransport_BLE = {
    .create = Create,
    .validateAccessory = ValidateAccessory,
    .prepareStart = PrepareStart,
    .start = Start,
    .tryStop = TryStop,
    .didRaiseEvent = HAPBLEAccessoryServerDidRaiseEvent,
    .updateAdvertisingData = UpdateAdvertisingData,
    .getGSN = HAPBLEAccessoryServerGetGSN,
    .broadcast = { .expireKey = HAPBLEAccessoryServerBroadcastExpireKey },
    .peripheralManager = { .release = HAPBLEPeripheralManagerRelease,
                           .handleSessionAccept = HAPBLEPeripheralManagerHandleSessionAccept,
                           .handleSessionInvalidate = HAPBLEPeripheralManagerHandleSessionInvalidate },
    .sessionCache = { .fetch = HAPPairingBLESessionCacheFetch,
                      .save = HAPPairingBLESessionCacheSave,
                      .invalidateEntriesForPairing = HAPPairingBLESessionCacheInvalidateEntriesForPairing },
    .session = { .create = HAPBLESessionCreate,
                 .release = HAPBLESessionRelease,
                 .invalidate = HAPBLESessionInvalidate,
                 .didStartPairingProcedure = HAPBLESessionDidStartPairingProcedure,
                 .didCompletePairingProcedure = HAPBLESessionDidCompletePairingProcedure }
};
