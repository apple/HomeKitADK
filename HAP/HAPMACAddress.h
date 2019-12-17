// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_MAC_ADDRESS_H
#define HAP_MAC_ADDRESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Gets the random (static) MAC address for a Bluetooth interface.
 *
 * @param      server               Accessory server.
 * @param      bleInterface         Bluetooth interface.
 * @param[out] macAddress           MAC address, if successful.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If persistent store access failed.
 *
 * @see Bluetooth Core Specification Version 5
 *      Vol 6 Part B Section 1.3.2.1 Static Device Address
 */
HAP_RESULT_USE_CHECK
HAPError HAPMACAddressGetRandomStaticBLEDeviceAddress(
        HAPAccessoryServerRef* server,
        const char* _Nullable bleInterface,
        HAPMACAddress* macAddress);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
