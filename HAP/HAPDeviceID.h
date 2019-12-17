// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_DEVICE_ID_H
#define HAP_DEVICE_ID_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Length of a Device ID.
 */
#define kHAPDeviceID_NumBytes ((size_t) 6)

/**
 * Device ID.
 *
 * @see HomeKit Accessory Protocol Specification R14
 *      Section 5.4 Device ID
 * @see Accessory Interface Specification - Wi-Fi Accessory Configuration Addendum R1
 *      Table 2-7 Apple Device IE elements
 */
typedef struct {
    /** Value. */
    uint8_t bytes[kHAPDeviceID_NumBytes];
} HAPDeviceID;
HAP_NONNULL_SUPPORT(HAPDeviceID)

/**
 * Gets the Device ID.
 *
 * @param      keyValueStore        Key-value store.
 * @param[out] deviceID             Device ID.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPDeviceIDGet(HAPPlatformKeyValueStoreRef keyValueStore, HAPDeviceID* deviceID);

/**
 * NULL-terminated Device ID string (format: XX:XX:XX:XX:XX:XX, uppercase).
 */
typedef struct {
    char stringValue[17 + 1]; /**< NULL-terminated. */
} HAPDeviceIDString;

/**
 * Gets the Device ID as a string.
 *
 * @param      keyValueStore        Key-value store.
 * @param[out] deviceIDString       Device ID string.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPDeviceIDGetAsString(HAPPlatformKeyValueStoreRef keyValueStore, HAPDeviceIDString* deviceIDString);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
