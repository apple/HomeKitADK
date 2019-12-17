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

/**
 * Apple Authentication Coprocessor provider.
 */
struct HAPPlatformMFiHWAuth {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    bool poweredOn;
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
