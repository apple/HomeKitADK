// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_ACCESSORY_SETUP_NFC_INIT_H
#define HAP_PLATFORM_ACCESSORY_SETUP_NFC_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_NFC
#define HAVE_NFC 0
#endif

#if HAVE_NFC
#include <pthread.h>
#include <nfc/nfc.h>
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**@file
 * Accessory setup programmable NFC tag.
 *
 * - Programmable NFC functionality uses libnfc <http://libnfc.org>.
 *   This library supports many popular NFC chips and is configured using a connection string.
 *   If programmable NFC is accessed differently the implementation needs to be adjusted.
 *
 * **Example**

   @code{.c}

   // Allocate Accessory setup programmable NFC tag.
   static HAPPlatformAccessorySetupNFC setupNFC;

   // Initialize Accessory setup programmable NFC tag.
   HAPPlatformAccessorySetupNFCCreate(&setupNFC,
       &(const HAPPlatformAccessorySetupNFCOptions) {
           .libnfcConnectionString = LIBNFC_CONNECTION_STRING // or NULL for default NFC device.
       });

   // Before accessory restarts, ensure that resources are properly released.
   HAPPlatformAccessorySetupNFCRelease(&setupNFC);

   @endcode
*/

/**
 * Accessory setup programmable NFC tag initialization options.
 */
typedef struct {
    /**
     * The libnfc specific device connection string if specific NFC device is wanted, NULL otherwise.
     */
    const char* _Nullable libnfcConnectionString;
} HAPPlatformAccessorySetupNFCOptions;

/**
 * Accessory setup programmable NFC tag.
 */
struct HAPPlatformAccessorySetupNFC {
    // Opaque type. Do not access the instance fields directly.
    /**@cond */
    const char* _Nullable libnfcConnectionString;

#if HAVE_NFC
    struct {
        HAPSetupPayload setupPayload;
        pthread_t thread;
        bool isActive : 1;

        volatile bool nfcLock;
        volatile int isAborted;
        volatile int isEmulating;

        nfc_context* nfcContext;
        nfc_device* nfcDevice;
    } nfc;
#endif
    /**@endcond */
};

/**
 * Initializes Accessory setup programmable NFC tag.
 *
 * @param[out] setupNFC             Accessory setup programmable NFC tag.
 * @param      options              Initialization options.
 */
void HAPPlatformAccessorySetupNFCCreate(
        HAPPlatformAccessorySetupNFCRef setupNFC,
        const HAPPlatformAccessorySetupNFCOptions* options);

/**
 * Deinitializes an Accessory setup programmable NFC tag.
 *
 * @param      setupNFC             Initialized Accessory setup programmable NFC tag.
 */
void HAPPlatformAccessorySetupNFCRelease(HAPPlatformAccessorySetupNFCRef setupNFC);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
