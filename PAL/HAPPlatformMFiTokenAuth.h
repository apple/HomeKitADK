// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_PLATFORM_MFI_TOKEN_AUTH_H
#define HAP_PLATFORM_MFI_TOKEN_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAPPlatform.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Software Token provider.
 */
typedef struct HAPPlatformMFiTokenAuth HAPPlatformMFiTokenAuth;
typedef struct HAPPlatformMFiTokenAuth* HAPPlatformMFiTokenAuthRef;
HAP_NONNULL_SUPPORT(HAPPlatformMFiTokenAuth)

/**
 * Maximum number of bytes that a Software Token may have.
 */
#define kHAPPlatformMFiTokenAuth_MaxMFiTokenBytes ((size_t) 1024)

/**
 * Software Token UUID.
 *
 * - The encoding of UUIDs uses reversed byte order compared to RFC 4122, i.e. network byte order backwards.
 *
 * Sample:
 *   UUID: 00112233-4455-6677-8899-AABBCCDDEEFF
 *   bytes: 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
 */
typedef struct {
    uint8_t bytes[16]; /**< UUID bytes in reversed network byte order. */
} HAPPlatformMFiTokenAuthUUID;
HAP_STATIC_ASSERT(sizeof(HAPPlatformMFiTokenAuthUUID) == 16, HAPPlatformMFiTokenAuthUUID);
HAP_NONNULL_SUPPORT(HAPPlatformMFiTokenAuthUUID)

/**
 * Loads the provisioned Software Token.
 *
 * - The initial Software Token must be provisioned during manufacturing or firmware update.
 *   The Software Token may later be changed through the HAPPlatformMFiTokenAuthUpdate function.
 *
 * - The Software Token must be decoded using base 64 and returned in raw data format.
 *
 * - If the accessory is connected to an Apple Authentication Coprocessor, this function is not used.
 *   Implement the HAPPlatformMFiHWAuth platform module instead.
 *
 * - If neither an Apple Authentication Coprocessor nor a MFi Software Token is available,
 *   a warning will be shown to the user during pairing claiming that the accessory has not been certified.
 *
 * - Software Authentication is only supported on iOS 11.3 or newer.
 *
 * @param      mfiTokenAuth         Software Token provider.
 * @param[out] valid                True if a Software Token is available. False otherwise.
 * @param[out] mfiTokenUUID         Software Token UUID, if available. Optional.
 * @param[out] mfiTokenBytes        Software Token buffer, if available. Optional.
 * @param      maxMFiTokenBytes     Capacity of the buffer.
 * @param[out] numMFiTokenBytes     Length of the Software Token.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while fetching the Software Token.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough to fit the Software Token.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthLoad(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        bool* valid,
        HAPPlatformMFiTokenAuthUUID* _Nullable mfiTokenUUID,
        void* _Nullable mfiTokenBytes,
        size_t maxMFiTokenBytes,
        size_t* _Nullable numMFiTokenBytes);

/**
 * Updates the provisioned Software Token.
 *
 * - /!\ WARNING: It is critical to ensure that the previous Software Token is not deleted
 *   before the update procedure is complete.
 *   Failure to do so may result in an accessory that can no longer be paired!
 *
 * - /!\ WARNING: This function must block until the new Software Token has been completely persisted.
 *   Alternatively, communication over Bluetooth LE and IP may be suppressed until the token is persisted.
 *   Allowing communication without waiting for persistence may result in an accessory that can no longer be paired!
 *
 * - If the accessory is connected to a physical Apple Authentication Coprocessor, this function is not used.
 *   Implement the HAPPlatformMFiHWAuth platform module instead.
 *
 * @param      mfiTokenAuth         Software Token provider.
 * @param      mfiTokenBytes        Software Token buffer.
 * @param      numMFiTokenBytes     Length of the Software Token.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If an error occurred while updating the Software Token.
 */
HAP_RESULT_USE_CHECK
HAPError HAPPlatformMFiTokenAuthUpdate(
        HAPPlatformMFiTokenAuthRef mfiTokenAuth,
        const void* mfiTokenBytes,
        size_t numMFiTokenBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
