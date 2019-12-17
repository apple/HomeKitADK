// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_DISPLAY_H
#define HAP_PLATFORM_ACCESSORY_SETUP_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * @file
 *
 * During HomeKit pairing, a setup code is used by the controller to set up an encrypted link with the accessory.
 * If an accessory has a display that supports showing a setup code, a random setup code is used while pairing.
 * This platform module must be implemented if the accessory has a display that supports showing a setup code.
 *
 * - Note that for displays that support showing a scannable QR code it is still necessary to provision each accessory
 *   with a unique setup ID during manufacturing. This setup ID is used to identify the accessory.
 *
 * - Example flow (User scans setup code):
 *   1. HAPPlatformAccessorySetupDisplayUpdateSetupPayload with setup code that changes periodically.
 *   2. User scans accessory setup information.
 *   3. HAPPlatformAccessorySetupDisplayHandleStartPairing.
 *   4. Pairing may take a while to complete.
 *   5. HAPPlatformAccessorySetupDisplayUpdateSetupPayload with NULL setup code.
 *   6. HAPPlatformAccessorySetupDisplayHandleStopPairing
 *
 * - Example flow (accessory browser):
 *   1. HAPPlatformAccessorySetupDisplayUpdateSetupPayload with setup code that changes periodically.
 *   2. User selects accessory from accessory browser
 *   3. HAPPlatformAccessorySetupDisplayHandleStartPairing.
 *   4. Accessory directs user to a screen that displays the setup code.
 *   5. User scans accessory setup information.
 *   6. Pairing may take a while to complete.
 *   7. HAPPlatformAccessorySetupDisplayUpdateSetupPayload with NULL setup code.
 *   8. HAPPlatformAccessorySetupDisplayHandleStopPairing
 *
 * When using Software Authentication multiple pairing attempts may be registered.
 */

/**
 * Accessory setup display.
 */
typedef struct HAPPlatformAccessorySetupDisplay HAPPlatformAccessorySetupDisplay;
typedef struct HAPPlatformAccessorySetupDisplay* HAPPlatformAccessorySetupDisplayRef;
HAP_NONNULL_SUPPORT(HAPPlatformAccessorySetupDisplay)

/**
 * Updates the accessory setup information that is shown on a connected display.
 *
 * @param      setupDisplay         Accessory setup display.
 * @param      setupPayload         Setup payload. If available, may be encoded and shown as a scannable QR code.
 * @param      setupCode            Setup code to display. If NULL, the display must stop showing setup information.
 */
void HAPPlatformAccessorySetupDisplayUpdateSetupPayload(
        HAPPlatformAccessorySetupDisplayRef setupDisplay,
        const HAPSetupPayload* _Nullable setupPayload,
        const HAPSetupCode* _Nullable setupCode);

/**
 * Indicates that a pairing attempt has been registered.
 *
 * - This may be used to direct the user to the display that shows accessory setup information.
 *
 * - Example use case:
 *   Accessories with a complex UI may opt to keep track of the current accessory setup information in background.
 *   When a pairing attempt is registered instructions may be shown directing the user to switch to the screen
 *   that displays the accessory setup information.
 *
 * @param      setupDisplay         Accessory setup display.
 */
void HAPPlatformAccessorySetupDisplayHandleStartPairing(HAPPlatformAccessorySetupDisplayRef setupDisplay);

/**
 * Indicates that a pairing attempt has completed or has been cancelled.
 *
 * @param      setupDisplay         Accessory setup display.
 */
void HAPPlatformAccessorySetupDisplayHandleStopPairing(HAPPlatformAccessorySetupDisplayRef setupDisplay);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
