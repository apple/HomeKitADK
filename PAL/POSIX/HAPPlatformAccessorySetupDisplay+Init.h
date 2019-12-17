// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_DISPLAY_INIT_H
#define HAP_PLATFORM_ACCESSORY_SETUP_DISPLAY_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

#ifndef HAVE_DISPLAY
#define HAVE_DISPLAY 0
#endif

/**@file
 * Accessory setup display.
 *
 * - The HAPLog APIs are used to display the setup payload and setup code.
 *   For a real display the implementation needs to be adjusted.
 *
 * **Example**

   @code{.c}

   // Allocate Accessory setup display.
   static HAPPlatformAccessorySetupDisplay setupDisplay;

   // Initialize Accessory setup display.
   HAPPlatformAccessorySetupDisplayCreate(&setupDisplay);

   @endcode
*/

/**
 * Accessory setup display.
 */
struct HAPPlatformAccessorySetupDisplay {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    HAPSetupPayload setupPayload;
    HAPSetupCode setupCode;
    bool setupPayloadIsSet : 1;
    bool setupCodeIsSet : 1;
    /**@endcond */
};

/**
 * Initializes Accessory setup display.
 *
 * @param[out] setupDisplay         Accessory setup display.
 */
void HAPPlatformAccessorySetupDisplayCreate(HAPPlatformAccessorySetupDisplayRef setupDisplay);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
