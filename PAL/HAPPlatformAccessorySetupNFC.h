// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_NFC_H
#define HAP_PLATFORM_ACCESSORY_SETUP_NFC_H

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
 * If the accessory has a programmable NFC tag, accessory setup information may be exchanged over NFC.
 * This platform module must be implemented if the accessory has a programmable NFC tag.
 *
 * - Programmable NFC requires the accessory to be provisioned with a setup ID.
 *   If no display is available the raw setup code must also be provisioned.
 *
 * - In contrast to displays that are activated automatically when a pairing request is registered,
 *   programmable NFC is only available after explicitly entering pairing mode.
 */

/**
 * Accessory setup programmable NFC tag.
 */
typedef struct HAPPlatformAccessorySetupNFC HAPPlatformAccessorySetupNFC;
typedef struct HAPPlatformAccessorySetupNFC* HAPPlatformAccessorySetupNFCRef;
HAP_NONNULL_SUPPORT(HAPPlatformAccessorySetupNFC)

/**
 * Updates the setup code that is shown on a connected display.
 *
 * @param      setupNFC             Accessory setup programmable NFC tag.
 * @param      setupPayload         Setup payload to make available using the programmable NFC tag.
 * @param      isPairable           Whether the setup payload may be used for pairing or indicates an error condition.
 */
void HAPPlatformAccessorySetupNFCUpdateSetupPayload(
        HAPPlatformAccessorySetupNFCRef setupNFC,
        const HAPSetupPayload* setupPayload,
        bool isPairable);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
