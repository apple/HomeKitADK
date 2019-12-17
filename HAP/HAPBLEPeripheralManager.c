// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static const HAPLogObject logObject = { .subsystem = kHAP_LogSubsystem, .category = "BLEPeripheralManager" };

#define DEBUG_DISABLE_TIMEOUTS (false)

/**
 * Fallback procedure status.
 *
 * - Fallback procedures can only return very simple information and can't access characteristics.
 *   /!\ If this is ever extended, proper checking for transient Pair Setup procedures is necessary!
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEFallbackProcedureStatus) {
    /** Max-Procedures. */
    kHAPBLEFallbackProcedureStatus_MaxProcedures = 1,

    /** Invalid instance ID. */
    kHAPBLEFallbackProcedureStatus_InvalidInstanceID,

    /** Operation is service signature read, and instance ID was 0. */
    kHAPBLEFallbackProcedureStatus_ZeroInstanceIDServiceSignatureRead
} HAP_ENUM_END(uint8_t, HAPBLEFallbackProcedureStatus);

/**
 * Fallback procedure state.
 *
 * - This keeps track of procedures beyond the maximum procedure limit.
 */
typedef struct {
    /**
     * Timer after which the procedure expires.
     *
     * - If this is 0, the procedure is not active.
     */
    HAPPlatformTimerRef timer;

    /** Remaining body bytes in the request before a response may be sent. */
    uint16_t remainingBodyBytes;

    /** Transaction ID of the procedure. */
    uint8_t transactionID;

    /** Status of the procedure. */
    HAPBLEFallbackProcedureStatus status;
} HAPBLEFallbackProcedure;

HAP_STATIC_ASSERT(sizeof(HAPBLEFallbackProcedure) <= 16, HAPBLEFallbackProcedureMustBeKeptSmall);

typedef struct {
    /**
     * The linked HomeKit characteristic.
     *
     * - If this is NULL, the entry is only linked to a HomeKit service.
     */
    const HAPCharacteristic* _Nullable characteristic;

    /**
     * The linked HomeKit service.
     *
     * - If this is NULL, the table entry is not used.
     */
    const HAPService* _Nullable service;

    /**
     * The linked HomeKit accessory.
     *
     * - If this is NULL, the table entry is not used.
     */
    const HAPAccessory* _Nullable accessory;

    /**
     * Attribute handle of the Characteristic Value declaration.
     */
    HAPPlatformBLEPeripheralManagerAttributeHandle valueHandle;

    /**
     * Attribute handle of the added Client Characteristic Configuration descriptor.
     *
     * - This is only available for HomeKit characteristics that support HAP Events.
     *
     * - If BLE Indications are enabled, the value of this descriptor contains ((uint16_t) 0x0002) in little endian.
     *   If BLE Indications are disabled, the value of this descriptor contains ((uint16_t) 0x0000) in little endian.
     */
    HAPPlatformBLEPeripheralManagerAttributeHandle cccDescriptorHandle;

    /**
     * For HomeKit characteristics: Attribute handle of the added Characteristic Instance ID descriptor.
     * For HomeKit services: Characteristic Value declaration of the added Service Instance ID characteristic.
     */
    HAPPlatformBLEPeripheralManagerAttributeHandle iidHandle;

    /**
     * State related about the connected controller.
     */
    struct {
        /**
         * Fallback procedure in case there are not enough resources to use a full-featured one.
         */
        HAPBLEFallbackProcedure fallbackProcedure;

        /**
         * Whether or not the connected central subscribed to this characteristic.
         *
         * - This is only available for HomeKit characteristics that support HAP Events.
         */
        bool centralSubscribed : 1;

        /**
         * Whether or not the characteristic value changed since the last read by the connected controller.
         *
         * - This is only maintained for HomeKit characteristics that support HAP Events.
         */
        bool pendingEvent : 1;
    } connectionState;
} HAPBLEGATTTableElement;
HAP_STATIC_ASSERT(sizeof(HAPBLEGATTTableElementRef) >= sizeof(HAPBLEGATTTableElement), HAPBLEGATTTableElement);
HAP_NONNULL_SUPPORT(HAPBLEGATTTableElement)

/**
 * Resets the state of HAP Events.
 *
 * @param      server_              Accessory server.
 */
static void ResetEventState(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "%s", __func__);

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];

        if (!gattAttribute->accessory) {
            break;
        }

        gattAttribute->connectionState.centralSubscribed = false;
        gattAttribute->connectionState.pendingEvent = false;
    }
}

/**
 * Aborts all fallback HAP-BLE procedures.
 *
 * @param      server_              Accessory server.
 */
static void AbortAllFallbackProcedures(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "%s", __func__);

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];

        if (!gattAttribute->accessory) {
            break;
        }

        if (gattAttribute->connectionState.fallbackProcedure.timer) {
            const HAPAccessory* accessory = gattAttribute->accessory;
            HAPAssert(gattAttribute->service);
            HAPAssert(gattAttribute->characteristic);
            const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;

            HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Aborting fallback procedure.");

#if !DEBUG_DISABLE_TIMEOUTS
            HAPPlatformTimerDeregister(gattAttribute->connectionState.fallbackProcedure.timer);
#endif

            HAPRawBufferZero(
                    &gattAttribute->connectionState.fallbackProcedure,
                    sizeof gattAttribute->connectionState.fallbackProcedure);
        }
    }
}

void HAPBLEPeripheralManagerRelease(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManager* blePeripheralManager = server->platform.ble.blePeripheralManager;

    // Abort procedures.
    AbortAllFallbackProcedures(server_);
    if (server->ble.connection.procedureAttached) {
        HAPBLEProcedureDestroy(&server->ble.storage->procedures[0]);
        server->ble.connection.procedureAttached = false;
    }

    // Abort connections.
    if (server->ble.connection.connected) {
        HAPAssert(server->ble.storage->session);
        HAPSessionRelease(server_, server->ble.storage->session);
        server->ble.connection.connected = false;
    }

    // Deregister platform callbacks.
    HAPPlatformBLEPeripheralManagerRemoveAllServices(blePeripheralManager);
    HAPPlatformBLEPeripheralManagerSetDelegate(blePeripheralManager, NULL);
}

static void HandleConnectedCentral(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        void* _Nullable context) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;

    HAPError err;

    HAPLogInfo(&logObject, "%s(0x%04x)", __func__, connectionHandle);
    HAPPrecondition(!server->ble.connection.connected);

    AbortAllFallbackProcedures(server_);
    ResetEventState(server_);
    server->ble.connection.connectionHandle = connectionHandle;
    server->ble.connection.connected = true;

    err = HAPBLEAccessoryServerDidConnect(server_);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }

    HAPSessionCreate(server_, session, kHAPTransportType_BLE);
}

static void HandleDisconnectedCentral(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        void* _Nullable context) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;

    HAPError err;

    HAPLogInfo(&logObject, "%s(0x%04x)", __func__, connectionHandle);
    HAPPrecondition(server->ble.connection.connected);
    HAPPrecondition(connectionHandle == server->ble.connection.connectionHandle);

    server->ble.connection.connected = false;
    if (server->ble.connection.procedureAttached) {
        HAPAssert(server->ble.storage->numProcedures >= 1);
        HAPBLEProcedureDestroy(&server->ble.storage->procedures[0]);
    }
    AbortAllFallbackProcedures(server_);
    HAPSessionRelease(server_, session);
    ResetEventState(server_);
    HAPRawBufferZero(&server->ble.connection, sizeof server->ble.connection);

    err = HAPBLEAccessoryServerDidDisconnect(server_);
    if (err) {
        HAPAssert(err == kHAPError_Unknown);
        HAPFatalError();
    }
}

/**
 * Continues sending of pending HAP event notifications.
 *
 * @param      server_              Accessory server.
 */
static void SendPendingEventNotifications(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(server->ble.connection.connected);
    HAPPrecondition(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;

    HAPError err;

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];
        const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;
        const HAPService* service = gattAttribute->service;
        const HAPAccessory* accessory = gattAttribute->accessory;
        if (!accessory) {
            break;
        }
        HAPAssert(service);
        if (!characteristic) {
            continue;
        }
        if (!characteristic->properties.supportsEventNotification) {
            HAPAssert(!gattAttribute->connectionState.centralSubscribed);
            HAPAssert(!gattAttribute->connectionState.pendingEvent);
            continue;
        }
        if (characteristic->iid > UINT16_MAX) {
            HAPLogCharacteristicError(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not sending Handle Value Indication because characteristic instance ID is not supported.");
            continue;
        }
        HAPAssert(gattAttribute->valueHandle);
        HAPAssert(gattAttribute->cccDescriptorHandle);
        HAPAssert(gattAttribute->iidHandle);

        if (!gattAttribute->connectionState.centralSubscribed) {
            continue;
        }
        if (!gattAttribute->connectionState.pendingEvent) {
            continue;
        }
        if (!HAPSessionIsSecured(session)) {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not sending Handle Value Indication because the session is not secured.");
            return;
        }
        if (HAPSessionIsTransient(session)) {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not sending Handle Value Indication because the session is transient.");
            return;
        }
        if (HAPCharacteristicReadRequiresAdminPermissions(characteristic) && !HAPSessionControllerIsAdmin(session)) {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not sending Handle Value Indication because event notification values will only be delivered to "
                    "controllers with admin permissions.");
            continue;
        }

        err = HAPPlatformBLEPeripheralManagerSendHandleValueIndication(
                blePeripheralManager,
                server->ble.connection.connectionHandle,
                gattAttribute->valueHandle,
                /* bytes: */ NULL,
                /* numBytes: */ 0);
        if (err == kHAPError_InvalidState) {
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Delayed event sending until ready to update subscribers.");
            return;
        } else if (err) {
            HAPAssert(err == kHAPError_OutOfResources);
            HAPFatalError();
        }
        gattAttribute->connectionState.pendingEvent = false;
        HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Sent event.");

        err = HAPBLEAccessoryServerDidSendEventNotification(server_, characteristic, service, accessory);
        if (err) {
            HAPAssert(err == kHAPError_Unknown);
            HAPFatalError();
        }
    }
}

/**
 * Gets the GATT attribute structure associated with an attribute handle.
 *
 * @param      server_              Accessory server.
 * @param      attributeHandle      GATT attribute handle.
 *
 * @return GATT attribute structure If found.
 * @return NULL                     Otherwise.
 */
static HAPBLEGATTTableElement* _Nullable GetGATTAttribute(
        HAPAccessoryServerRef* server_,
        HAPPlatformBLEPeripheralManagerAttributeHandle attributeHandle) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(attributeHandle);

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];
        if (!gattAttribute->accessory) {
            break;
        }

        // Validate GATT attribute.
        HAPAssert(gattAttribute->service);
        if (!gattAttribute->characteristic) {
            HAPAssert(!gattAttribute->valueHandle);
            HAPAssert(!gattAttribute->cccDescriptorHandle);
        } else {
            const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;
            HAPAssert(gattAttribute->valueHandle);
            if (!characteristic->properties.supportsEventNotification) {
                HAPAssert(!gattAttribute->cccDescriptorHandle);
            }
        }
        HAPAssert(gattAttribute->iidHandle);

        // Check for match.
        if (attributeHandle == gattAttribute->valueHandle || attributeHandle == gattAttribute->cccDescriptorHandle ||
            attributeHandle == gattAttribute->iidHandle) {
            return gattAttribute;
        }
    }
    HAPLog(&logObject, "GATT attribute structure not found for handle 0x%04x", (unsigned int) attributeHandle);
    return NULL;
}

HAP_RESULT_USE_CHECK
static bool AreNotificationsEnabled(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPBLEGATTTableElement* gattAttribute) {
    HAPPrecondition(server);
    HAPPrecondition(session);
    HAPPrecondition(gattAttribute);
    const HAPCharacteristic* characteristic = HAPNonnullVoid(gattAttribute->characteristic);
    const HAPService* service HAP_UNUSED = HAPNonnull(gattAttribute->service);
    const HAPAccessory* accessory = HAPNonnull(gattAttribute->accessory);

    HAPLogCharacteristicInfo(
            &logObject,
            characteristic,
            service,
            accessory,
            "Events are %s.",
            gattAttribute->connectionState.centralSubscribed ? "enabled" : "disabled");
    return gattAttribute->connectionState.centralSubscribed;
}

static void SetNotificationsEnabled(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPBLEGATTTableElement* gattAttribute,
        bool enable) {
    HAPPrecondition(server);
    HAPPrecondition(session);
    HAPPrecondition(gattAttribute);
    const HAPCharacteristic* characteristic = gattAttribute->characteristic;
    const HAPService* service = gattAttribute->service;
    const HAPAccessory* accessory = gattAttribute->accessory;

    HAPLogCharacteristicInfo(
            &logObject, characteristic, service, accessory, "%s events.", enable ? "Enabling" : "Disabling");
    if (gattAttribute->connectionState.centralSubscribed == enable) {
        return;
    }
    gattAttribute->connectionState.centralSubscribed = enable;

    // Inform application.
    if (HAPSessionIsSecured(session)) {
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "Informing application about %s of events.",
                enable ? "enabling" : "disabling");
        if (enable) {
            HAPAccessoryServerHandleSubscribe(server, session, characteristic, service, accessory);
        } else {
            HAPAccessoryServerHandleUnsubscribe(server, session, characteristic, service, accessory);
        }
    } else {
        HAPLogCharacteristic(
                &logObject,
                characteristic,
                service,
                accessory,
                "Session is not secured. Delaying to inform application about %s of events.",
                enable ? "enabling" : "disabling");
    }

    // Subscription state changed. Continue sending events.
    SendPendingEventNotifications(server);
}

#if !DEBUG_DISABLE_TIMEOUTS
static void FallbackProcedureTimerExpired(HAPPlatformTimerRef timer, void* _Nullable context) {
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;

    HAPLogDebug(&logObject, "%s", __func__);

    // 39. Accessories must implement a 10 second HAP procedure timeout, all HAP procedures [...] must complete within
    // 10 seconds, if a procedure fails to complete within the procedure timeout the accessory must drop the security
    // session and also drop the Bluetooth link.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];

        if (!gattAttribute->accessory) {
            break;
        }
        if (gattAttribute->connectionState.fallbackProcedure.timer != timer) {
            continue;
        }

        const HAPAccessory* accessory = gattAttribute->accessory;
        HAPAssert(gattAttribute->service);
        HAPAssert(gattAttribute->characteristic);
        const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;

        HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Fallback procedure expired.");

#if !DEBUG_DISABLE_TIMEOUTS
        HAPPlatformTimerDeregister(gattAttribute->connectionState.fallbackProcedure.timer);
#endif
        HAPRawBufferZero(
                &gattAttribute->connectionState.fallbackProcedure,
                sizeof gattAttribute->connectionState.fallbackProcedure);
    }

    HAPAssert(server->ble.connection.connected);
    HAPAssert(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;
    HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
}
#endif

/**
 * HAP-BLE procedure type.
 */
HAP_ENUM_BEGIN(uint8_t, HAPBLEProcedureType) { /**
                                                * Full-featured procedure.
                                                *
                                                * - Associated type: HAPBLEProcedureRef
                                                */
                                               kHAPBLEProcedureType_Full = 1,

                                               /**
                                                * Fallback procedure.
                                                *
                                                * - Associated type: HAPBLEFallbackProcedure
                                                */
                                               kHAPBLEProcedureType_Fallback
} HAP_ENUM_END(uint8_t, HAPBLEProcedureType);

/**
 * Checks that a value matches the claimed HAPBLEProcedureType.
 *
 * - Argument numbers start at 1.
 *
 * @param      valueArg             Argument number of the value.
 * @param      typeArg              Argument number of the value type.
 */
#if __has_attribute(pointer_with_type_tag) && __has_attribute(type_tag_for_datatype)
/**@cond */
__attribute__((type_tag_for_datatype(HAPBLEProcedureType, HAPBLEProcedureRef*))) static const HAPBLEProcedureType
        _kHAPBLEProcedureType_Full HAP_UNUSED = kHAPBLEProcedureType_Full;

__attribute__((type_tag_for_datatype(HAPBLEProcedureType, HAPBLEFallbackProcedure*))) static const HAPBLEProcedureType
        _kHAPBLEProcedureType_Fallback HAP_UNUSED = kHAPBLEProcedureType_Fallback;
/**@endcond */

#define HAP_PWT_HAPBLEProcedureType(valueArg, typeArg) \
    __attribute__((pointer_with_type_tag(HAPBLEProcedureType, valueArg, typeArg)))
#else
#define HAP_PWT_HAPBLEProcedureType(valueArg, typeArg)
#endif

/**
 * Gets the HAP-BLE procedure for a GATT attribute.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      gattAttribute        The GATT attribute that is accessed.
 * @param[out] procedureType        Type of the attached procedure.
 * @param[out] procedure            HAP-BLE procedure.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no procedure can be fetched at this time.
 */
HAP_PWT_HAPBLEProcedureType(5, 4) HAP_RESULT_USE_CHECK static HAPError GetProcedure(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPBLEGATTTableElement* gattAttribute,
        HAPBLEProcedureType* procedureType,
        void* _Nonnull* _Nonnull procedure) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(gattAttribute);
    HAPPrecondition(gattAttribute->characteristic);
    const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;
    HAPPrecondition(gattAttribute->service);
    HAPPrecondition(gattAttribute->accessory);
    const HAPAccessory* accessory = gattAttribute->accessory;
    HAPPrecondition(procedureType);
    HAPPrecondition(procedure);

    // For now, we only support 1 concurrent full-featured procedure.
    HAPPrecondition(server->ble.storage->procedures);
    HAPPrecondition(server->ble.storage->procedures);
    HAPPrecondition(server->ble.storage->numProcedures >= 1);
    HAPBLEProcedureRef* fullProcedure = &server->ble.storage->procedures[0];

    // Every characteristic supports a fallback procedure.
    HAPBLEFallbackProcedure* fallbackProcedure = &gattAttribute->connectionState.fallbackProcedure;

    // If session is terminal, no more requests may be accepted.
    if (HAPBLESessionIsTerminal(&session->_.ble)) {
        HAPLogCharacteristic(&logObject, characteristic, service, accessory, "Rejecting request: Session is terminal.");
        HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                blePeripheralManager, server->ble.connection.connectionHandle);
        return kHAPError_InvalidState;
    }

    // An accessory must cancel any pending procedures when a new HAP secure session starts getting established.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.1 HAP Transactions and Procedures
    if (HAPBLECharacteristicDropsSecuritySession(characteristic)) {
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "Aborting fallback procedure (%s).",
                "Characteristic drops security session");
        AbortAllFallbackProcedures(server_);
    }

    // Check if already attached to the same characteristic (fallback procedure).
    if (gattAttribute->connectionState.fallbackProcedure.timer) {
        *procedureType = kHAPBLEProcedureType_Fallback;
        *procedure = fallbackProcedure;
        return kHAPError_None;
    }

    // Check if already attached to the same characteristic (full procedure).
    if (server->ble.connection.procedureAttached) {
        const HAPBaseCharacteristic* attachedCharacteristic = HAPBLEProcedureGetAttachedCharacteristic(fullProcedure);
        HAPAssert(attachedCharacteristic);

        if (attachedCharacteristic == characteristic) {
            *procedureType = kHAPBLEProcedureType_Full;
            *procedure = fullProcedure;
            return kHAPError_None;
        }
    }

    // Unsolicited read request.
    // 12. Accessory must reject GATT Read Requests on a HAP characteristic if it was not preceded by an
    // GATT Write Request with the same transaction ID at most 10 seconds prior.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.5 Testing Bluetooth LE Accessories
    return kHAPError_InvalidState;
}

HAP_RESULT_USE_CHECK
static HAPError HandleReadRequest(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle attributeHandle,
        void* bytes,
        size_t maxBytes,
        size_t* numBytes,
        void* _Nullable context) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(attributeHandle);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;

    HAPError err;

    HAPLogDebug(&logObject, "%s(0x%04x, 0x%04x)", __func__, connectionHandle, attributeHandle);
    HAPPrecondition(server->ble.connection.connected);
    HAPPrecondition(connectionHandle == server->ble.connection.connectionHandle);
    HAPBLEGATTTableElement* _Nullable gattAttribute = GetGATTAttribute(server_, attributeHandle);
    HAPPrecondition(gattAttribute);
    const HAPBaseCharacteristic* _Nullable characteristic = gattAttribute->characteristic;
    const HAPService* _Nullable service = gattAttribute->service;
    const HAPAccessory* _Nullable accessory = gattAttribute->accessory;

    if (attributeHandle == gattAttribute->valueHandle) {
        HAPAssert(characteristic);
        HAPAssert(service);
        HAPAssert(accessory);
        HAPLogCharacteristicDebug(&logObject, characteristic, service, accessory, "GATT Read value.");

        // Get HAP-BLE procedure.
        HAPBLEProcedureType procedureType;
        void* procedure;
        err = GetProcedure(server_, session, gattAttribute, &procedureType, &procedure);
        if (err) {
            HAPAssert(err == kHAPError_InvalidState);
            HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
            return err;
        }

        // Process request.
        switch (procedureType) {
            case kHAPBLEProcedureType_Full: {
                HAPBLEProcedureRef* fullProcedure = procedure;

                // Process request.
                err = HAPBLEProcedureHandleGATTRead(fullProcedure, bytes, maxBytes, numBytes);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
                    HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                    return err;
                }
            } break;
            case kHAPBLEProcedureType_Fallback: {
                HAPBLEFallbackProcedure* fallbackProcedure = procedure;

                HAPLogCharacteristicInfo(
                        &logObject, characteristic, service, accessory, "Processing response of fallback procedure.");

                if (fallbackProcedure->remainingBodyBytes) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Response of fallback procedure expected before request was fully sent.");
                    HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                    return kHAPError_InvalidState;
                }

                // Compute response length.
                *numBytes = 3;
                switch (fallbackProcedure->status) {
                    case kHAPBLEFallbackProcedureStatus_MaxProcedures:
                    case kHAPBLEFallbackProcedureStatus_InvalidInstanceID: {
                        *numBytes += 0;
                    } break;
                    case kHAPBLEFallbackProcedureStatus_ZeroInstanceIDServiceSignatureRead: {
                        *numBytes += 2; // Body length.
                        *numBytes += 2;
                    } break;
                }

                // When Pair Verify is accessed, all fallback procedures are cancelled.
                // Therefore, we do not need to remember whether or not the procedure has been secured at start.
                bool isSecured = HAPSessionIsSecured(session);
                if (isSecured) {
                    *numBytes += CHACHA20_POLY1305_TAG_BYTES;
                }
                if (maxBytes < *numBytes) {
                    HAPLogCharacteristic(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Response of fallback procedure on too long for available space.");
                    HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                    return kHAPError_OutOfResources;
                }

                // Serialize response.
                uint8_t* data = bytes;
                data[0] = (0 << 7) | (0 << 3) | (0 << 2) | (1 << 1) | (0 << 0);
                data[1] = fallbackProcedure->transactionID;
                switch (fallbackProcedure->status) {
                    case kHAPBLEFallbackProcedureStatus_MaxProcedures: {
                        HAPLogCharacteristic(
                                &logObject, characteristic, service, accessory, "Sending Max-Procedures error.");
                        data[2] = kHAPBLEPDUStatus_MaxProcedures;
                    } break;
                    case kHAPBLEFallbackProcedureStatus_InvalidInstanceID: {
                        HAPLogCharacteristic(
                                &logObject, characteristic, service, accessory, "Sending Invalid Instance ID error.");
                        data[2] = kHAPBLEPDUStatus_InvalidInstanceID;
                    } break;
                    case kHAPBLEFallbackProcedureStatus_ZeroInstanceIDServiceSignatureRead: {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Sending default service signature response (iid 0).");
                        data[2] = kHAPBLEPDUStatus_Success;
                        HAPWriteLittleUInt16(&data[3], 2);
                        data[5] = kHAPBLEPDUTLVType_HAPLinkedServices;
                        data[6] = 0;
                    } break;
                }

                // Encrypt response if necessary.
                if (isSecured) {
                    err = HAPSessionEncryptControlMessage(
                            server_, session, bytes, bytes, *numBytes - CHACHA20_POLY1305_TAG_BYTES);
                    if (err) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Response of fallback procedure could not be encrypted.");
                        HAPAssert(err == kHAPError_InvalidState);
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return err;
                    }
                }

                // Reset procedure.
                HAPAssert(fallbackProcedure->timer);
#if !DEBUG_DISABLE_TIMEOUTS
                HAPPlatformTimerDeregister(fallbackProcedure->timer);
#endif
                HAPRawBufferZero(fallbackProcedure, sizeof *fallbackProcedure);

                // Report response being sent.
                HAPBLESessionDidSendGATTResponse(server_, session);
            } break;
        }

        // Continue sending events (if security state changed).
        SendPendingEventNotifications(server_);
    } else if (attributeHandle == gattAttribute->cccDescriptorHandle) {
        HAPAssert(characteristic);
        HAPAssert(service);
        HAPAssert(accessory);
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "GATT Read Client Characteristic Configuration descriptor value.");

        // This descriptor value must support always being read in the clear, i.e. with or without a security session.
        // See HomeKit Accessory Protocol Specification R14
        // Section 7.4.4.5.3 Client Characteristic Configuration

        // Process request.
        if (maxBytes < 2) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Not enough space available to write Client Characteristic Configuration descriptor value.");
            return kHAPError_OutOfResources;
        }
        bool isEnabled = AreNotificationsEnabled(server_, session, gattAttribute);
        HAPWriteLittleUInt16(bytes, isEnabled ? 0x0002u : 0x0000u);
        *numBytes = sizeof(uint16_t);
    } else {
        HAPAssert(attributeHandle == gattAttribute->iidHandle);
        HAPAssert(service);
        HAPAssert(accessory);
        if (characteristic) {
            HAPLogCharacteristicDebug(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "GATT Read Characteristic Instance ID descriptor value.");

            // Process request.
            if (maxBytes < 2) {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Not enough space available to write Characteristic Instance ID descriptor value.");
                return kHAPError_OutOfResources;
            }
            HAPAssert(characteristic->iid <= UINT16_MAX);
            HAPWriteLittleUInt16(bytes, characteristic->iid);
            *numBytes = sizeof(uint16_t);
        } else {
            HAPLogServiceDebug(&logObject, service, accessory, "GATT Read Service Instance ID descriptor value.");

            // Process request.
            if (maxBytes < 2) {
                HAPLogService(
                        &logObject,
                        service,
                        accessory,
                        "Not enough space available to write Service Instance ID descriptor value.");
                return kHAPError_OutOfResources;
            }
            HAPAssert(service->iid <= UINT16_MAX);
            HAPWriteLittleUInt16(bytes, service->iid);
            *numBytes = sizeof(uint16_t);
        }
    }
    return kHAPError_None;
}

/**
 * Attaches a HAP-BLE procedure.
 *
 * @param      server_              Accessory server.
 * @param      session_             The session over which the request has been received.
 * @param      gattAttribute        The GATT attribute that is accessed.
 * @param[out] procedureType        Type of the attached procedure.
 * @param[out] procedure            HAP-BLE procedure.
 * @param[out] isNewProcedure       Whether a new or existing procedure was attached. true = new.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidState   If no procedure can be fetched at this time.
 * @return kHAPError_OutOfResources If no procedure is available.
 */
HAP_PWT_HAPBLEProcedureType(5, 4) HAP_RESULT_USE_CHECK static HAPError AttachProcedure(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session_,
        HAPBLEGATTTableElement* gattAttribute,
        HAPBLEProcedureType* procedureType,
        void* _Nonnull* _Nonnull procedure,
        bool* isNewProcedure) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(session_);
    HAPSession* session = (HAPSession*) session_;
    HAPPrecondition(gattAttribute);
    HAPPrecondition(gattAttribute->characteristic);
    const HAPBaseCharacteristic* characteristic = gattAttribute->characteristic;
    HAPPrecondition(gattAttribute->service);
    const HAPService* service = gattAttribute->service;
    HAPPrecondition(gattAttribute->accessory);
    const HAPAccessory* accessory = gattAttribute->accessory;
    HAPPrecondition(procedureType);
    HAPPrecondition(procedure);
    HAPPrecondition(isNewProcedure);

#if !DEBUG_DISABLE_TIMEOUTS
    HAPError err;
#endif

    // For now, we only support 1 concurrent full-featured procedure.
    HAPPrecondition(server->ble.storage->procedures);
    HAPPrecondition(server->ble.storage->procedures);
    HAPPrecondition(server->ble.storage->numProcedures >= 1);
    HAPBLEProcedureRef* fullProcedure = &server->ble.storage->procedures[0];

    // Every characteristic supports a fallback procedure.
    HAPBLEFallbackProcedure* fallbackProcedure = &gattAttribute->connectionState.fallbackProcedure;

    // If session is terminal, no more requests may be accepted.
    if (HAPBLESessionIsTerminal(&session->_.ble)) {
        HAPLogCharacteristic(&logObject, characteristic, service, accessory, "Rejecting request: Session is terminal.");
        HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                blePeripheralManager, server->ble.connection.connectionHandle);
        return kHAPError_InvalidState;
    }

    // Handle shut down.
    if (server->state != kHAPAccessoryServerState_Running) {
        if (server->ble.connection.procedureAttached && HAPBLEProcedureIsInProgress(fullProcedure)) {
            // Allow finishing procedure to avoid dealing with bugs from halfway completed procedures.
            // Fallback procedures do not modify any state, so it's okay to abort them while they are ongoing.
            // Procedures have a timeout so this cannot delay forever.
            HAPLogCharacteristicInfo(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Shutdown has been requested. Allowing current HAP-BLE procedure to finish.");
        } else {
            // Do not start new procedures and abort pending fallback procedures.
            HAPLogCharacteristic(
                    &logObject, characteristic, service, accessory, "Rejecting request: Shutdown requested.");
            HAPPlatformBLEPeripheralManagerCancelCentralConnection(
                    blePeripheralManager, server->ble.connection.connectionHandle);
            return kHAPError_InvalidState;
        }
    }

    // An accessory must cancel any pending procedures when a new HAP secure session starts getting established.
    // See HomeKit Accessory Protocol Specification R14
    // Section 7.3.1 HAP Transactions and Procedures
    if (HAPBLECharacteristicDropsSecuritySession(characteristic)) {
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "Aborting fallback procedure (%s).",
                "Characteristic drops security session");
        AbortAllFallbackProcedures(server_);
    }

    // Check if already attached to the same characteristic (fallback procedure).
    if (gattAttribute->connectionState.fallbackProcedure.timer) {
        *procedureType = kHAPBLEProcedureType_Fallback;
        *procedure = fallbackProcedure;
        *isNewProcedure = false;
        return kHAPError_None;
    }

    // Detach full-featured procedure from previous characteristic if necessary.
    if (server->ble.connection.procedureAttached) {
        const HAPBaseCharacteristic* attachedCharacteristic = HAPBLEProcedureGetAttachedCharacteristic(fullProcedure);
        HAPAssert(attachedCharacteristic);

        // Check if already attached to the same characteristic.
        if (attachedCharacteristic == characteristic) {
            *procedureType = kHAPBLEProcedureType_Full;
            *procedure = fullProcedure;
            *isNewProcedure = false;
            return kHAPError_None;
        }

        // Check if previous procedure is detachable.
        if (HAPBLEProcedureIsInProgress(fullProcedure)) {
            // An accessory must cancel any pending procedures when a new HAP secure session starts getting established.
            // See HomeKit Accessory Protocol Specification R14
            // Section 7.3.1 HAP Transactions and Procedures
            if (HAPBLECharacteristicDropsSecuritySession(characteristic)) {
                HAPLogCharacteristicInfo(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "Aborting existing procedure on [%016llX %s] (%s).",
                        (unsigned long long) attachedCharacteristic->iid,
                        attachedCharacteristic->debugDescription,
                        "Characteristic drops security session");

                AbortAllFallbackProcedures(server_);
            } else {
                HAPLogCharacteristic(
                        &logObject,
                        characteristic,
                        service,
                        accessory,
                        "HAP-BLE procedure on [%016llX %s] is in progress. Attaching fallback procedure.",
                        (unsigned long long) attachedCharacteristic->iid,
                        attachedCharacteristic->debugDescription);

#if !DEBUG_DISABLE_TIMEOUTS
                err = HAPPlatformTimerRegister(
                        &fallbackProcedure->timer,
                        HAPPlatformClockGetCurrent() + 10 * HAPSecond,
                        FallbackProcedureTimerExpired,
                        server_);
                if (err) {
                    HAPAssert(err == kHAPError_OutOfResources);
                    HAPLogCharacteristicError(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Not enough resources to start timer. Disconnecting immediately!");
                    return err;
                }
#else
                fallbackProcedure->timer = 1;
#endif
                HAPBLESessionDidStartBLEProcedure(server_, session_);

                *procedureType = kHAPBLEProcedureType_Fallback;
                *procedure = fallbackProcedure;
                *isNewProcedure = true;
                return kHAPError_None;
            }
        }

        // Detach from previous procedure.
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "Detaching procedure from [%016llX %s] to start procedure.",
                (unsigned long long) attachedCharacteristic->iid,
                attachedCharacteristic->debugDescription);
        HAPBLEProcedureDestroy(fullProcedure);
        server->ble.connection.procedureAttached = false;
    }

    // Attach to new characteristic.
    HAPLogCharacteristicDebug(&logObject, characteristic, service, accessory, "Attaching procedure.");
    HAPBLEProcedureAttach(
            fullProcedure,
            server->ble.storage->procedureBuffer.bytes,
            server->ble.storage->procedureBuffer.numBytes,
            server_,
            session_,
            characteristic,
            service,
            accessory);
    server->ble.connection.procedureAttached = true;

    *procedureType = kHAPBLEProcedureType_Full;
    *procedure = fullProcedure;
    *isNewProcedure = true;
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
static HAPError HandleWriteRequest(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        HAPPlatformBLEPeripheralManagerAttributeHandle attributeHandle,
        void* bytes,
        size_t numBytes,
        void* _Nullable context) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(attributeHandle);
    HAPPrecondition(bytes);
    HAPPrecondition(numBytes);
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.storage->session);
    HAPSessionRef* session = server->ble.storage->session;

    HAPError err;

    HAPLogDebug(&logObject, "%s(0x%04x, 0x%04x)", __func__, connectionHandle, attributeHandle);
    HAPPrecondition(server->ble.connection.connected);
    HAPPrecondition(connectionHandle == server->ble.connection.connectionHandle);
    HAPBLEGATTTableElement* _Nullable gattAttribute = GetGATTAttribute(server_, attributeHandle);
    HAPPrecondition(gattAttribute);
    const HAPBaseCharacteristic* _Nullable characteristic = gattAttribute->characteristic;
    const HAPService* _Nullable service = gattAttribute->service;
    const HAPAccessory* _Nullable accessory = gattAttribute->accessory;

    if (attributeHandle == gattAttribute->valueHandle) {
        HAPAssert(characteristic);
        HAPAssert(service);
        HAPAssert(accessory);
        HAPLogCharacteristicDebug(&logObject, characteristic, service, accessory, "GATT Write value.");

        // Get HAP-BLE procedure.
        HAPBLEProcedureType procedureType;
        void* procedure;
        bool isNewProcedure;
        err = AttachProcedure(server_, session, gattAttribute, &procedureType, &procedure, &isNewProcedure);
        if (err) {
            HAPAssert(err == kHAPError_InvalidState || err == kHAPError_OutOfResources);
            HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
            return err;
        }

        // Process request.
        switch (procedureType) {
            case kHAPBLEProcedureType_Full: {
                HAPBLEProcedureRef* fullProcedure = procedure;

                // Process request.
                err = HAPBLEProcedureHandleGATTWrite(fullProcedure, bytes, numBytes);
                if (err) {
                    HAPAssert(
                            err == kHAPError_InvalidState || err == kHAPError_InvalidData ||
                            err == kHAPError_OutOfResources);
                    HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                    return err;
                }
            } break;
            case kHAPBLEProcedureType_Fallback: {
                HAPBLEFallbackProcedure* fallbackProcedure = procedure;

                // When Pair Verify is accessed, all fallback procedures are cancelled.
                // Therefore, we do not need to remember whether or not the procedure has been secured at start.
                if (HAPSessionIsSecured(session)) {
                    if (numBytes < CHACHA20_POLY1305_TAG_BYTES) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Write to fallback procedure malformed (too short for auth tag).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }
                    err = HAPSessionDecryptControlMessage(server_, session, bytes, bytes, numBytes);
                    if (err) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "First fragment of fallback procedure malformed (decryption failed).");
                        HAPAssert(err == kHAPError_InvalidState || err == kHAPError_InvalidData);
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return err;
                    }
                    numBytes -= CHACHA20_POLY1305_TAG_BYTES;
                }

                if (isNewProcedure) {
                    HAPLogCharacteristicInfo(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Processing first fragment of fallback procedure.");

                    uint8_t* data = bytes;
                    if (numBytes < 5) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "First fragment of fallback procedure malformed (too short).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }
                    if (data[0] != ((0 << 7) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0))) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "First fragment of fallback procedure malformed (control field).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }

                    // Store minimal information to be able to throw error.
                    fallbackProcedure->transactionID = data[2];
                    fallbackProcedure->status = kHAPBLEFallbackProcedureStatus_MaxProcedures;

                    // Handle simple errors.
                    uint8_t operation = data[1];
                    uint16_t iid = HAPReadLittleUInt16(&data[3]);
                    if (HAPPDUIsValidOpcode(operation)) {
                        uint16_t expectedIID;
                        if (HAPBLEPDUOpcodeIsServiceOperation((HAPPDUOpcode) operation)) {
                            HAPAssert(service->iid <= UINT16_MAX);
                            expectedIID = (uint16_t) service->iid;
                        } else {
                            HAPAssert(characteristic->iid <= UINT16_MAX);
                            expectedIID = (uint16_t) characteristic->iid;
                        }

                        if (iid != expectedIID) {
                            HAPLogCharacteristic(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "Invalid IID %u in fallback procedure.",
                                    iid);

                            fallbackProcedure->status = kHAPBLEFallbackProcedureStatus_InvalidInstanceID;

                            // If the accessory receives an invalid (eg., 0) Service instance ID in the
                            // HAP-Service-Signature-Read-Request, it must respond with a valid
                            // HAP-Service-Signature-Read-Response with Svc Properties set to 0 and Linked Svc
                            // (if applicable) set to 0 length.
                            // See HomeKit Accessory Protocol Specification R14
                            // Section 7.3.4.13 HAP-Service-Signature-Read-Response
                            if (operation == kHAPPDUOpcode_ServiceSignatureRead && !iid) {
                                fallbackProcedure->status =
                                        kHAPBLEFallbackProcedureStatus_ZeroInstanceIDServiceSignatureRead;
                            }
                        }
                    }

                    // Skip body.
                    if (numBytes > 5) {
                        if (numBytes < 7) {
                            HAPLogCharacteristic(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "First fragment of fallback procedure on malformed (body length).");
                            HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                            return kHAPError_InvalidData;
                        }

                        fallbackProcedure->remainingBodyBytes = HAPReadLittleUInt16(&data[5]);

                        // Skip body.
                        if (fallbackProcedure->remainingBodyBytes < numBytes - 7) {
                            HAPLogCharacteristic(
                                    &logObject,
                                    characteristic,
                                    service,
                                    accessory,
                                    "First fragment of fallback procedure on malformed (body too long).");
                            HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                            return kHAPError_InvalidData;
                        }
                        fallbackProcedure->remainingBodyBytes -= (uint16_t)(numBytes - 7);
                    } else {
                        fallbackProcedure->remainingBodyBytes = 0;
                    }
                } else {
                    HAPLogCharacteristicInfo(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "Processing continuation of fallback procedure.");

                    uint8_t* data = bytes;
                    if (numBytes < 2) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Continuation of fallback procedure malformed (too short).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }
                    if (data[0] != ((1 << 7) | (0 << 3) | (0 << 2) | (0 << 1) | (0 << 0))) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Continuation of fallback procedure malformed (control field).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }
                    if (data[1] != fallbackProcedure->transactionID) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Continuation of fallback procedure malformed (invalid TID).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }

                    // Skip body.
                    if (fallbackProcedure->remainingBodyBytes < numBytes - 2) {
                        HAPLogCharacteristic(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "Continuation of fallback procedure malformed (body too long).");
                        HAPSessionInvalidate(server_, session, /* terminateLink: */ true);
                        return kHAPError_InvalidData;
                    }
                    fallbackProcedure->remainingBodyBytes -= (uint16_t)(numBytes - 2);
                }

                // Report response being sent.
                HAPBLESessionDidSendGATTResponse(server_, session);
            } break;
        }

        // Continue sending events (if security state changed).
        SendPendingEventNotifications(server_);
    } else if (attributeHandle == gattAttribute->cccDescriptorHandle) {
        HAPAssert(characteristic);
        HAPAssert(service);
        HAPAssert(accessory);
        HAPLogCharacteristicDebug(
                &logObject,
                characteristic,
                service,
                accessory,
                "GATT Write Client Characteristic Configuration descriptor value.");

        // Process request.
        if (numBytes != 2) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Unexpected Client Characteristic Configuration descriptor length: %lu.",
                    (unsigned long) numBytes);
            return kHAPError_InvalidData;
        }
        uint16_t v = HAPReadLittleUInt16(bytes);
        if (v & ~0x0002) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Unexpected Client Characteristic Configuration descriptor value: 0x%04x.",
                    (unsigned int) v);
            return kHAPError_InvalidData;
        }
        bool eventsEnabled = (v & 0x0002) != 0;
        SetNotificationsEnabled(server_, session, gattAttribute, eventsEnabled);
    } else {
        HAPAssert(attributeHandle == gattAttribute->iidHandle);
        HAPAssert(service);
        HAPAssert(accessory);
        if (characteristic) {
            HAPLogCharacteristicDebug(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "GATT Write Characteristic Instance ID descriptor value.");

            // Process request.
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Rejecting write to Characteristic Instance ID descriptor value.");
            return kHAPError_InvalidState;
        } else {
            HAPLogServiceDebug(&logObject, service, accessory, "GATT Write Service Instance ID descriptor value.");

            // Process request.
            HAPLogService(&logObject, service, accessory, "Rejecting write to Service Instance ID descriptor value.");
            return kHAPError_InvalidState;
        }
    }
    return kHAPError_None;
}

static void HandleReadyToUpdateSubscribers(
        HAPPlatformBLEPeripheralManagerRef blePeripheralManager,
        HAPPlatformBLEPeripheralManagerConnectionHandle connectionHandle,
        void* _Nullable context) {
    HAPPrecondition(blePeripheralManager);
    HAPPrecondition(context);
    HAPAccessoryServerRef* server_ = context;
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->ble.connection.connected);

    HAPLogDebug(&logObject, "%s(0x%04x)", __func__, connectionHandle);
    HAPPrecondition(server->ble.connection.connected);
    HAPPrecondition(connectionHandle == server->ble.connection.connectionHandle);

    SendPendingEventNotifications(server_);
}

void HAPBLEPeripheralManagerRegister(HAPAccessoryServerRef* server_) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(server->platform.ble.blePeripheralManager);
    HAPPlatformBLEPeripheralManagerRef blePeripheralManager = server->platform.ble.blePeripheralManager;
    HAPPrecondition(server->primaryAccessory);
    const HAPAccessory* accessory = server->primaryAccessory;

    HAPError err;

    // Reset table.
    HAPRawBufferZero(
            server->ble.storage->gattTableElements,
            server->ble.storage->numGATTTableElements * sizeof *server->ble.storage->gattTableElements);
    HAPPlatformBLEPeripheralManagerRemoveAllServices(blePeripheralManager);

    // Set delegate.
    HAPPlatformBLEPeripheralManagerSetDelegate(
            blePeripheralManager,
            &(const HAPPlatformBLEPeripheralManagerDelegate) { .context = server_,
                                                               .handleConnectedCentral = HandleConnectedCentral,
                                                               .handleDisconnectedCentral = HandleDisconnectedCentral,
                                                               .handleReadRequest = HandleReadRequest,
                                                               .handleWriteRequest = HandleWriteRequest,
                                                               .handleReadyToUpdateSubscribers =
                                                                       HandleReadyToUpdateSubscribers });

    // Register DB.
    size_t o = 0;
    if (accessory->services) {
        for (size_t i = 0; accessory->services[i]; i++) {
            const HAPService* service = accessory->services[i];

            if (!HAPAccessoryServerSupportsService(server_, kHAPTransportType_BLE, service)) {
                continue;
            }

            // Map GATT attribute for service.
            if (o >= server->ble.storage->numGATTTableElements) {
                HAPLogServiceError(
                        &logObject, service, accessory, "GATT table capacity not large enough to store service.");
                HAPFatalError();
            }
            HAPBLEGATTTableElement* gattAttribute =
                    (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[o];
            gattAttribute->accessory = accessory;
            gattAttribute->service = service;

            // Register service.
            HAPAssert(sizeof *service->serviceType == sizeof(HAPPlatformBLEPeripheralManagerUUID));
            err = HAPPlatformBLEPeripheralManagerAddService(
                    blePeripheralManager,
                    (const HAPPlatformBLEPeripheralManagerUUID*) service->serviceType,
                    /* isPrimary: */ true);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                HAPFatalError();
            }

            // Register Service Instance ID characteristic.
            // This characteristic contains a static value and does not use HAP-BLE procedures.
            static const HAPPlatformBLEPeripheralManagerUUID kBLECharacteristicUUID_ServiceInstanceID = {
                { 0xD1, 0xA0, 0x83, 0x50, 0x00, 0xAA, 0xD3, 0x87, 0x17, 0x48, 0x59, 0xA7, 0x5D, 0xE9, 0x04, 0xE6 }
            };
            uint8_t iid[2];
            HAPWriteLittleUInt16(iid, service->iid);
            err = HAPPlatformBLEPeripheralManagerAddCharacteristic(
                    blePeripheralManager,
                    &kBLECharacteristicUUID_ServiceInstanceID,
                    (HAPPlatformBLEPeripheralManagerCharacteristicProperties) { .read = true,
                                                                                .writeWithoutResponse = false,
                                                                                .write = false,
                                                                                .notify = false,
                                                                                .indicate = false },
                    iid,
                    sizeof iid,
                    &gattAttribute->iidHandle,
                    /* cccDescriptorHandle: */ NULL);
            if (err) {
                HAPAssert(err == kHAPError_OutOfResources);
                HAPFatalError();
            }

            // Finalize GATT attribute.
            HAPLogServiceInfo(&logObject, service, accessory, "(service)");
            o++;

            // Register characteristics.
            if (service->characteristics) {
                for (size_t j = 0; service->characteristics[j]; j++) {
                    const HAPBaseCharacteristic* characteristic = service->characteristics[j];

                    // Map GATT attribute for characteristic.
                    if (o >= server->ble.storage->numGATTTableElements) {
                        HAPLogCharacteristicError(
                                &logObject,
                                characteristic,
                                service,
                                accessory,
                                "GATT table capacity not large enough to store characteristic.");
                        HAPFatalError();
                    }
                    gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[o];
                    gattAttribute->accessory = accessory;
                    gattAttribute->service = service;
                    gattAttribute->characteristic = characteristic;

                    // Register characteristic.
                    HAPAssert(
                            sizeof *characteristic->characteristicType == sizeof(HAPPlatformBLEPeripheralManagerUUID));
                    err = HAPPlatformBLEPeripheralManagerAddCharacteristic(
                            blePeripheralManager,
                            (const HAPPlatformBLEPeripheralManagerUUID*) characteristic->characteristicType,
                            (HAPPlatformBLEPeripheralManagerCharacteristicProperties) {
                                    .read = true,
                                    .writeWithoutResponse = false,
                                    .write = true,
                                    .notify = false,
                                    .indicate = characteristic->properties.supportsEventNotification },
                            NULL,
                            0,
                            &gattAttribute->valueHandle,
                            characteristic->properties.supportsEventNotification ? &gattAttribute->cccDescriptorHandle :
                                                                                   NULL);
                    if (err) {
                        HAPAssert(err == kHAPError_OutOfResources);
                        HAPFatalError();
                    }

                    // Register Characteristic Instance ID descriptor.
                    // This descriptor contains a static value and does not use HAP-BLE procedures.
                    static const HAPPlatformBLEPeripheralManagerUUID kBLEDescriptorUUID_CharacteristicInstanceID = {
                        { 0x9A,
                          0x93,
                          0x96,
                          0xD7,
                          0xBD,
                          0x6A,
                          0xD9,
                          0xB5,
                          0x16,
                          0x46,
                          0xD2,
                          0x81,
                          0xFE,
                          0xF0,
                          0x46,
                          0xDC }
                    };
                    HAPWriteLittleUInt16(iid, characteristic->iid);
                    err = HAPPlatformBLEPeripheralManagerAddDescriptor(
                            blePeripheralManager,
                            &kBLEDescriptorUUID_CharacteristicInstanceID,
                            (HAPPlatformBLEPeripheralManagerDescriptorProperties) { .read = true, .write = false },
                            iid,
                            sizeof iid,
                            &gattAttribute->iidHandle);
                    if (err) {
                        HAPAssert(err == kHAPError_OutOfResources);
                        HAPFatalError();
                    }

                    // Finalize GATT attribute.
                    HAPLogCharacteristicInfo(
                            &logObject,
                            characteristic,
                            service,
                            accessory,
                            "val %04x / iid %04x",
                            gattAttribute->valueHandle,
                            gattAttribute->iidHandle);
                    o++;
                }
            }
        }
    }

    // Finalize GATT database.
    HAPPlatformBLEPeripheralManagerPublishServices(blePeripheralManager);
}

void HAPBLEPeripheralManagerRaiseEvent(
        HAPAccessoryServerRef* server_,
        const HAPCharacteristic* characteristic_,
        const HAPService* service,
        const HAPAccessory* accessory) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(characteristic_);
    const HAPBaseCharacteristic* characteristic = characteristic_;
    HAPPrecondition(service);
    HAPPrecondition(accessory);

    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];
        if (!gattAttribute->accessory) {
            break;
        }

        if (gattAttribute->characteristic == characteristic && gattAttribute->service == service &&
            gattAttribute->accessory == accessory) {
            HAPLogCharacteristicInfo(&logObject, characteristic, service, accessory, "Scheduling event.");
            gattAttribute->connectionState.pendingEvent = true;
            SendPendingEventNotifications(server_);
            return;
        }
    }
    HAPLogCharacteristic(&logObject, characteristic, service, accessory, "GATT attribute structure not found.");
}

void HAPBLEPeripheralManagerHandleSessionAccept(HAPAccessoryServerRef* server_, HAPSessionRef* session) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    HAPPrecondition(HAPSessionIsSecured(session));
    if (!server->transports.ble) {
        return;
    }
    if (session != server->ble.storage->session) {
        return;
    }

    // On BLE event subscriptions may be enabled before the HomeKit session is secured.
    // If this happens we have delayed informing the application about the updated subscription state
    // and need to inform it now that it has been informed.
    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];
        if (!gattAttribute->accessory) {
            break;
        }

        if (!gattAttribute->characteristic) {
            continue;
        }
        const HAPBaseCharacteristic* characteristic = HAPNonnullVoid(gattAttribute->characteristic);
        const HAPService* service = HAPNonnull(gattAttribute->service);
        const HAPAccessory* accessory = HAPNonnull(gattAttribute->accessory);
        if (!characteristic->properties.supportsEventNotification) {
            continue;
        }

        // Inform application.
        if (gattAttribute->connectionState.centralSubscribed) {
            HAPLogCharacteristic(
                    &logObject,
                    characteristic,
                    service,
                    accessory,
                    "Informing application about enabling of events that were enabled before session was accepted.");
            HAPAccessoryServerHandleSubscribe(server_, session, characteristic, service, accessory);
        }
    }

    // Continue sending events.
    SendPendingEventNotifications(server_);
}

void HAPBLEPeripheralManagerHandleSessionInvalidate(HAPAccessoryServerRef* server_, HAPSessionRef* session) {
    HAPPrecondition(server_);
    HAPAccessoryServer* server = (HAPAccessoryServer*) server_;
    HAPPrecondition(session);
    if (!server->transports.ble) {
        return;
    }
    if (session != server->ble.storage->session) {
        return;
    }

    // Inform application that controller has unsubscribed from all characteristics.
    // Note that on BLE the actual subscription state persists across sequential sessions until there is a disconnect.
    for (size_t i = 0; i < server->ble.storage->numGATTTableElements; i++) {
        HAPBLEGATTTableElement* gattAttribute = (HAPBLEGATTTableElement*) &server->ble.storage->gattTableElements[i];
        if (!gattAttribute->accessory) {
            break;
        }

        if (!gattAttribute->characteristic) {
            continue;
        }
        const HAPBaseCharacteristic* characteristic = HAPNonnullVoid(gattAttribute->characteristic);
        const HAPService* service = HAPNonnull(gattAttribute->service);
        const HAPAccessory* accessory = HAPNonnull(gattAttribute->accessory);
        if (!characteristic->properties.supportsEventNotification) {
            continue;
        }

        // Inform application.
        if (gattAttribute->connectionState.centralSubscribed) {
            HAPLogCharacteristicDebug(
                    &logObject, characteristic, service, accessory, "Informing application about disabling of events.");
            HAPAccessoryServerHandleUnsubscribe(server_, session, characteristic, service, accessory);
        }
    }
}
