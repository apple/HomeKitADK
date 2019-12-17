// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"
#include "HAPPlatform+Init.h"

#include "Harness/HAPTestController.c"
#include "Harness/TemplateDB.c"

typedef struct {
    char _;
} TestContext;

static void HandleUpdatedAccessoryServerState(HAPAccessoryServerRef* server, void* _Nullable context HAP_UNUSED) {
    HAPPrecondition(server);

    switch (HAPAccessoryServerGetState(server)) {
        case kHAPAccessoryServerState_Idle: {
            HAPLogInfo(&kHAPLog_Default, "Accessory server state: %s.", "Idle");
            return;
        }
        case kHAPAccessoryServerState_Running: {
            HAPLogInfo(&kHAPLog_Default, "Accessory server state: %s.", "Running");
            return;
        }
        case kHAPAccessoryServerState_Stopping: {
            HAPLogInfo(&kHAPLog_Default, "Accessory server state: %s.", "Stopping");
            return;
        }
    }
    HAPFatalError();
}

HAP_RESULT_USE_CHECK
static HAPError IdentifyAccessory(
        HAPAccessoryServerRef* server HAP_UNUSED,
        const HAPAccessoryIdentifyRequest* request HAP_UNUSED,
        void* _Nullable context HAP_UNUSED) {
    HAPFatalError();
}

static const HAPAccessory accessory = { .aid = 1,
                                        .category = kHAPAccessoryCategory_Other,
                                        .name = "Acme Test",
                                        .manufacturer = "Acme",
                                        .model = "Test1,1",
                                        .serialNumber = "099DB48E9E28",
                                        .firmwareVersion = "1",
                                        .hardwareVersion = "1",
                                        .services = (const HAPService* const[]) { &accessoryInformationService,
                                                                                  &hapProtocolInformationService,
                                                                                  &pairingService,
                                                                                  NULL },
                                        .callbacks = { .identify = IdentifyAccessory } };

int main() {
    HAPError err;
    HAPPlatformCreate();

    //    static TestContext test;

    // Prepare accessory server storage.
    static HAPBLEGATTTableElementRef gattTableElements[kAttributeCount];
    static HAPBLESessionCacheElementRef sessionCacheElements[kHAPBLESessionCache_MinElements];
    static HAPSessionRef session;
    static uint8_t procedureBytes[2048];
    static HAPBLEProcedureRef procedures[1];
    static HAPBLEAccessoryServerStorage bleAccessoryServerStorage = {
        .gattTableElements = gattTableElements,
        .numGATTTableElements = HAPArrayCount(gattTableElements),
        .sessionCacheElements = sessionCacheElements,
        .numSessionCacheElements = HAPArrayCount(sessionCacheElements),
        .session = &session,
        .procedures = procedures,
        .numProcedures = HAPArrayCount(procedures),
        .procedureBuffer = { .bytes = procedureBytes, .numBytes = sizeof procedureBytes },
    };

    // Initialize accessory server.
    static HAPAccessoryServerRef accessoryServer;
    HAPAccessoryServerCreate(
            &accessoryServer,
            &(const HAPAccessoryServerOptions) {
                    .maxPairings = kHAPPairingStorage_MinElements,
                    .ble = { .transport = &kHAPAccessoryServerTransport_BLE,
                             .accessoryServerStorage = &bleAccessoryServerStorage,
                             .preferredAdvertisingInterval = kHAPBLEAdvertisingInterval_Minimum,
                             .preferredNotificationDuration = kHAPBLENotification_MinDuration } },
            &platform,
            &(const HAPAccessoryServerCallbacks) { .handleUpdatedState = HandleUpdatedAccessoryServerState },
            /* context: */ NULL);

    // Start accessory server.
    HAPAccessoryServerStart(&accessoryServer, &accessory);
    HAPPlatformClockAdvance(0);
    HAPAssert(HAPAccessoryServerGetState(&accessoryServer) == kHAPAccessoryServerState_Running);

    // Discover BLE accessory server.
    HAPAccessoryServerInfo serverInfo;
    HAPPlatformBLEPeripheralManagerDeviceAddress deviceAddress;
    err = HAPDiscoverBLEAccessoryServer(HAPNonnull(platform.ble.blePeripheralManager), &serverInfo, &deviceAddress);
    HAPAssert(!err);
    HAPAssert(serverInfo.statusFlags.isNotPaired);

    // TODO Pair, discover, test events... [NYI]
}
