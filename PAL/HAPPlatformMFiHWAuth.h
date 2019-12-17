// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_MFI_HW_AUTH_H
#define HAP_PLATFORM_MFI_HW_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Apple Authentication Coprocessor provider.
 */
typedef struct HAPPlatformMFiHWAuth HAPPlatformMFiHWAuth;
typedef struct HAPPlatformMFiHWAuth* HAPPlatformMFiHWAuthRef;
HAP_NONNULL_SUPPORT(HAPPlatformMFiHWAuth)

/**
 * Returns whether the Apple Authentication Coprocessor is powered on.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor provider.
 *
 * @return true                     If the Apple Authentication Coprocessor is enabled.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPPlatformMFiHWAuthIsPoweredOn(HAPPlatformMFiHWAuthRef mfiHWAuth);

/**
 * Powers on and prepares the Apple Authentication Coprocessor for usage.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor provider.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with the Apple Authentication Coprocessor failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthPowerOn(HAPPlatformMFiHWAuthRef mfiHWAuth);

/**
 * Powers off the Apple Authentication Coprocessor.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor provider.
 */
void HAPPlatformMFiHWAuthPowerOff(HAPPlatformMFiHWAuthRef mfiHWAuth);

/**
 * Writes data to the Apple Authentication Coprocessor.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor provider.
 * @param      bytes                Buffer to write.
 * @param      numBytes             Length of buffer. Minimum 1. Maximum 128.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with the Apple Authentication Coprocessor failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthWrite(HAPPlatformMFiHWAuthRef mfiHWAuth, const void* bytes, size_t numBytes);

/**
 * Reads a register from the Apple Authentication Coprocessor.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor provider.
 * @param      registerAddress      Address of the Apple Authentication Coprocessor register to read.
 * @param      bytes                Result buffer.
 * @param      numBytes             Length of buffer. Minimum 1. Maximum 128.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with the Apple Authentication Coprocessor failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiHWAuthRead(
        HAPPlatformMFiHWAuthRef mfiHWAuth,
        uint8_t registerAddress,
        void* bytes,
        size_t numBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
