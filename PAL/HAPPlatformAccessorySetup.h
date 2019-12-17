// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_H
#define HAP_PLATFORM_ACCESSORY_SETUP_H

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
 * Each accessory must be provisioned for use with HomeKit during manufacturing.
 * This platform module must be implemented to provide access to the provisioned information.
 *
 *
 * Setup code:
 *
 * During HomeKit pairing, a setup code is used by the controller to set up an encrypted link with the accessory.
 * - If the accessory has a display that supports showing a setup code, a random setup code is used while pairing.
 * - Otherwise, a static setup code has to be generated during manufacturing that is deployed to the accessory.
 *
 * If a static setup code is used, the following steps must be followed for each accessory:
 *
 * 1. A random setup code in format "XXX-XX-XXX" with X being a digit from 0-9 must be generated.
 *    - The setup code must be generated from a cryptographically secure random number generator.
 *    - Setup codes that only consist of a repeating digit are not allowed.
 *    - 123-45-678 and 876-54-321 are not allowed.
 *
 * 2. A random SRP salt (16 random bytes) must be generated.
 *    - The SRP salt must be generated from a cryptographically secure random number generator.
 *
 * 3. The corresponding SRP verifier is derived from the setup code and the SRP salt.
 *
 * 4. The SRP salt and SRP verifier are deployed to the accessory.
 *
 * 5. If the accessory has a programmable NFC tag, the setup code must also be deployed.
 *    Otherwise, the raw setup code must not be deployed.
 *
 *
 * Setup ID:
 *
 * To improve the setup experience, a random setup ID is generated and deployed to each accessory during manufacturing.
 * During pairing, the setup ID is used to identify the accessory to which a scanned label belongs.
 *
 * The following steps must be followed for each accessory:
 *
 * 1. A random setup ID in format "XXXX" with X being a digit from 0-9 or a character from A-Z must be generated.
 *    - The setup ID must be generated from a cryptographically secure random number generator.
 *    - Lowercase characters are not allowed.
 *
 * 2. The setup ID is deployed to the accessory.
 *
 * - If no setup ID is deployed, certain features like QR code displays or programmable NFC are unavailable.
 *
 *
 * Labels:
 *
 * If the accessory has a static setup code, a label must be affixed to the accessory and its packaging.
 * Labels are based on the setup payload that is derived from the setup ID and the setup code.
 *
 * There are different kinds of labels:
 * - Labels consisting of a QR code that encodes the setup payload, and containing the setup code.
 * - Labels with an embedded NFC tag that provides the setup payload when read, and containing the setup code.
 * - Legacy labels containing only the setup code.
 *
 * If an accessory has a programmable NFC tag, only QR code labels should be used.
 * If the accessory is a security class device and uses NFC pairing, it must have a programmable NFC tag.
 */

/**
 * Accessory setup manager.
 */
typedef struct HAPPlatformAccessorySetup HAPPlatformAccessorySetup;
typedef struct HAPPlatformAccessorySetup* HAPPlatformAccessorySetupRef;
HAP_NONNULL_SUPPORT(HAPPlatformAccessorySetup)

/**
 * Loads SRP salt and verifier of a static setup code.
 *
 * - If the accessory can display a dynamic setup code, implementation is not required.
 *
 * @param      accessorySetup       Accessory setup manager.
 * @param[out] setupInfo            Setup info.
 */
void HAPPlatformAccessorySetupLoadSetupInfo(HAPPlatformAccessorySetupRef accessorySetup, HAPSetupInfo* setupInfo);

/**
 * Loads statically provisioned setup code in plaintext.
 *
 * - Must only be implemented if the accessory is connected to a programmable NFC tag
 *   but can't display a dynamic setup code.
 *
 * - The static setup info must be loadable and must be compatible with the setup code.
 *
 * @param      accessorySetup       Accessory setup manager.
 * @param[out] setupCode            Setup code.
 */
void HAPPlatformAccessorySetupLoadSetupCode(HAPPlatformAccessorySetupRef accessorySetup, HAPSetupCode* setupCode);

/**
 * Loads statically provisioned setup ID.
 *
 * - Required to display QR codes and to use programmable NFC tags.
 *
 * @param      accessorySetup       Accessory setup manager.
 * @param[out] valid                True if a setup ID is available. False otherwise.
 * @param[out] setupID              Setup ID, if available.
 */
void HAPPlatformAccessorySetupLoadSetupID(
        HAPPlatformAccessorySetupRef accessorySetup,
        bool* valid,
        HAPSetupID* setupID);

//----------------------------------------------------------------------------------------------------------------------
// Deprecated APIs.

/**
 * Accessory setup capabilities.
 */
typedef struct {
    /**
     * Whether the accessory can display a dynamic 8-digit setup code.
     *
     * - A QR code may optionally be displayed to simplify scanning by the controller.
     *
     * - If the accessory cannot display a dynamic setup code, static setup info must be loadable.
     */
    bool supportsDisplay : 1;

    /**
     * Whether the accessory is connected to a programmable NFC tag that supports NDEF records.
     *
     * - NFC tag types must be Type 2 or greater and must support payload lengths of 30 bytes.
     */
    bool supportsProgrammableNFC : 1;
} HAPPlatformAccessorySetupCapabilities;

/**
 * Returns the accessory setup capabilities.
 *
 * @param      accessorySetup       Accessory setup manager.
 *
 * @return Accessory setup capabilities.
 */
HAP_DEPRECATED_MSG("Return false and use HAPPlatformAccessorySetupDisplay / HAPPlatformAccessorySetupNFC instead.")
HAP_RESULT_USE_CHECK
HAPPlatformAccessorySetupCapabilities
        HAPPlatformAccessorySetupGetCapabilities(HAPPlatformAccessorySetupRef accessorySetup);

/**
 * Updates the setup payload and setup code for displays and programmable NFC tags.
 *
 * Display:
 * - If setupCode is NULL, the display must stop showing setup information.
 * - If setupCode is set, the display must show the given setup code.
 *   If setupPayload is also set, a scannable QR code describing the setup payload may be presented instead.
 *
 * Programmable NFC:
 * - If setupPayload is set, the programmable NFC tag must be reprogrammed using the updated setup payload.
 *
 * @param      accessorySetup       Accessory setup manager.
 * @param      setupPayload         Setup payload, if available.
 * @param      setupCode            Setup code to display, if available.
 */
HAP_DEPRECATED_MSG("Use HAPPlatformAccessorySetupDisplay / HAPPlatformAccessorySetupNFC instead.")
void HAPPlatformAccessorySetupUpdateSetupPayload(
        HAPPlatformAccessorySetupRef accessorySetup,
        const HAPSetupPayload* _Nullable setupPayload,
        const HAPSetupCode* _Nullable setupCode);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
