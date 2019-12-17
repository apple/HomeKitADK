// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_MFI_HW_AUTH_H
#define HAP_MFI_HW_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Apple Authentication Coprocessor manager.
 */
typedef struct {
    /**
     * Apple Authentication Coprocessor provider.
     */
    HAPPlatformMFiHWAuthRef _Nullable platformMFiHWAuth;

    /**
     * Time to check MFi power off.
     */
    HAPPlatformTimerRef powerOffTimer;
} HAPMFiHWAuth;
HAP_NONNULL_SUPPORT(HAPMFiHWAuth)

/**
 * Initializes Apple Authentication Coprocessor manager.
 *
 * @param[out] mfiHWAuth            Uninitialized Apple Authentication Coprocessor manager.
 * @param      platformMFiHWAuth    Apple Authentication Coprocessor provider if available.
 */
void HAPMFiHWAuthCreate(HAPMFiHWAuth* mfiHWAuth, HAPPlatformMFiHWAuthRef _Nullable platformMFiHWAuth);

/**
 * Deinitializes Apple Authentication Coprocessor manager.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor manager.
 */
void HAPMFiHWAuthRelease(HAPMFiHWAuth* mfiHWAuth);

/**
 * Check whether the MFi chip can be shut down.
 * HAPMFiHWAuthRelease should only be called when this returns true.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor manager.
 *
 * @return true                     If the Apple Authentication Coprocessor manager can be released safely.
 * @return false                    If release of the Apple Authentication Coprocessor manager should be delayed.
 */
HAP_RESULT_USE_CHECK
bool HAPMFiHWAuthIsSafeToRelease(HAPMFiHWAuth* mfiHWAuth);

/**
 * Check if the Apple Authentication Coprocessor is available.
 *
 * @param      mfiHWAuth            Apple Authentication Coprocessor manager.
 *
 * @return true                     If the Apple Authentication Coprocessor chip is available.
 * @return false                    Otherwise.
 */
HAP_RESULT_USE_CHECK
bool HAPMFiHWAuthIsAvailable(HAPMFiHWAuth* mfiHWAuth);

/**
 * Retrieves a copy of the MFi certificate.
 *
 * @param      server               Accessory server.
 * @param[out] certificateBytes     MFi certificate buffer.
 * @param      maxCertificateBytes  Capacity of MFi certificate buffer.
 * @param[out] numCertificateBytes  Effective length of MFi certificate buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with the MFi Authentication Secure Task failed.
 * @return kHAPError_OutOfResources If out of resources to process request.
 */
HAP_RESULT_USE_CHECK
HAPError HAPMFiHWAuthCopyCertificate(
        HAPAccessoryServerRef* server,
        void* certificateBytes,
        size_t maxCertificateBytes,
        size_t* numCertificateBytes);

/**
 * Signs the digest of a challenge with the MFi Private Key.
 *
 * @param      server               Accessory server.
 * @param      challengeBytes       Challenge buffer.
 * @param      numChallengeBytes    Length of challenge buffer.
 * @param[out] signatureBytes       Signature buffer.
 * @param      maxSignatureBytes    Capacity of signature buffer.
 * @param[out] numSignatureBytes    Effective length of signature buffer.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_Unknown        If communication with the MFi Authentication Secure Task failed.
 * @return kHAPError_OutOfResources If out of resources to process request.
 */
HAP_RESULT_USE_CHECK
HAPError HAPMFiHWAuthCreateSignature(
        HAPAccessoryServerRef* server,
        const void* challengeBytes,
        size_t numChallengeBytes,
        void* signatureBytes,
        size_t maxSignatureBytes,
        size_t* numSignatureBytes);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
