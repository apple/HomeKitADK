// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_ACCESSORY_SETUP_H
#define HAP_ACCESSORY_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Checks whether a string represents a valid setup code.
 *
 * @param      stringValue          Value to check. NULL-terminated.
 *
 * @return true                     If the string is a valid setup code.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPAccessorySetupIsValidSetupCode(const char* stringValue);

/**
 * Generates a random setup code.
 *
 * @param[out] setupCode            Setup code.
 */
void HAPAccessorySetupGenerateRandomSetupCode(HAPSetupCode* setupCode);

/**
 * Checks whether a string represents a valid setup ID.
 *
 * @param      stringValue          Value to check. NULL-terminated.
 *
 * @return true                     If the string is a valid setup ID.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPAccessorySetupIsValidSetupID(const char* stringValue);

/**
 * Generates a random setup ID.
 *
 * @param[out] setupID              Setup ID.
 */
void HAPAccessorySetupGenerateRandomSetupID(HAPSetupID* setupID);

/**
 * Setup payload flags.
 */
typedef struct {
    /**
     * Accessory is paired with a controller.
     * (only for accessories using programmable NFC tags to advertise the setup payload).
     *
     * If paired, no setup code or setup ID must be encoded.
     */
    bool isPaired : 1;

    /** Accessory supports HAP over IP transport; */
    bool ipSupported : 1;

    /** Accessory supports HAP over BLE transport. */
    bool bleSupported : 1;
} HAPAccessorySetupSetupPayloadFlags;

/**
 * Generates the setup payload for a given setup code and setup ID.
 *
 * @param[out] setupPayload         Setup payload.
 * @param      setupCode            Setup code.
 * @param      setupID              Setup ID.
 * @param      flags                Setup payload flags.
 * @param      category             Accessory category.
 */
void HAPAccessorySetupGetSetupPayload(
        HAPSetupPayload* setupPayload,
        const HAPSetupCode* _Nullable setupCode,
        const HAPSetupID* _Nullable setupID,
        HAPAccessorySetupSetupPayloadFlags flags,
        HAPAccessoryCategory category);

/**
 * Setup hash.
 */
typedef struct {
    uint8_t bytes[4]; /**< Value. */
} HAPAccessorySetupSetupHash;
HAP_STATIC_ASSERT(sizeof(HAPAccessorySetupSetupHash) == 4, HAPAccessorySetupSetupHash);

/**
 * Derives the setup hash for a given setup ID and Device ID.
 *
 * @param[out] setupHash            Setup hash.
 * @param      setupID              Setup ID.
 * @param      deviceIDString       Device ID.
 */
void HAPAccessorySetupGetSetupHash(
        HAPAccessorySetupSetupHash* setupHash,
        const HAPSetupID* setupID,
        const HAPDeviceIDString* deviceIDString);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
