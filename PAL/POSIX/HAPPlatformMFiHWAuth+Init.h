// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_MFI_HW_AUTH_INIT_H
#define HAP_PLATFORM_MFI_HW_AUTH_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * Apple Authentication Coprocessor provider based on the i2c-dev module.
 *
 * The implementation accesses the Apple Authentication Coprocessor directly over I2C.
 * It is assumed that the Apple Authentication Coprocessor uses I2C address 0x10
 * and that the i2c-dev module makes it accessible through the path "/dev/i2c-1".
 * If a different path or I2C address is used, the implementation needs to be adjusted.
 *
 * **Example**

   @code{.c}

   // Allocate Apple Authentication Coprocessor provider.
   static HAPPlatformMFiHWAuth mfiHWAuth;

   // Initialize Apple Authentication Coprocessor provider.
   HAPPlatformMFiHWAuthCreate(&mfiHWAuth);

   // Before process exits, ensure that resources are properly released.
   HAPPlatformMFiHWAuthRelease(&mfiHWAuth);

   @endcode
 */

/**
 * Apple Authentication Coprocessor provider.
 */
struct HAPPlatformMFiHWAuth {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    int i2cFile;
    bool enabled;
    /**@endcond */
};

/**
 * Initializes an Apple Authentication Coprocessor provider.
 *
 * @param[out] mfiHWAuth            Pointer to an allocated but uninitialized HAPPlatformMFiHWAuth structure.
 */
void HAPPlatformMFiHWAuthCreate(HAPPlatformMFiHWAuthRef mfiHWAuth);

/**
 * Deinitializes an Apple Authentication Coprocessor.
 *
 * @param      mfiHWAuth            Initialized Apple Authentication Coprocessor provider.
 */
void HAPPlatformMFiHWAuthRelease(HAPPlatformMFiHWAuthRef mfiHWAuth);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
