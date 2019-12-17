// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_BLE_PERIPHERAL_MANAGER_H
#define HAP_BLE_PERIPHERAL_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Releases all resources that have been allocated by the peripheral manager.
 *
 * @param      server               Accessory server.
 */
void HAPBLEPeripheralManagerRelease(HAPAccessoryServerRef* server);

/**
 * Registers the accessory server's GATT DB.
 *
 * @param      server               Accessory server.
 */
void HAPBLEPeripheralManagerRegister(HAPAccessoryServerRef* server);

/**
 * Raises an event notification for a given characteristic in a given service provided by a given accessory object.
 *
 * @param      server               Accessory server.
 * @param      characteristic       The characteristic whose value has changed.
 * @param      service              The service that contains the characteristic.
 * @param      accessory            The accessory that provides the service.
 */
void HAPBLEPeripheralManagerRaiseEvent(
        HAPAccessoryServerRef* server,
        const HAPCharacteristic* characteristic,
        const HAPService* service,
        const HAPAccessory* accessory);

/**
 * Informs the peripheral manager that a HomeKit Session was accepted.
 *
 * - This is called after the application has been informed that the session was accepted.
 *
 * @param      server               Accessory server.
 * @param      session              The newly accepted session.
 */
void HAPBLEPeripheralManagerHandleSessionAccept(HAPAccessoryServerRef* server, HAPSessionRef* session);

/**
 * Informs the peripheral manager that a HomeKit Session was invalidated.
 *
 * - This is called before the application is informed that the session was invalidated.
 *
 * @param      server               Accessory server.
 * @param      session              The session being invalidated.
 */
void HAPBLEPeripheralManagerHandleSessionInvalidate(HAPAccessoryServerRef* server, HAPSessionRef* session);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
