// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_MFI_AUTH_H
#define HAP_MFI_AUTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * MFiAuth implementation.
 */
typedef struct {
    /**
     * Retrieves a copy of the MFi certificate.
     *
     * @param      server               Accessory server.
     * @param[out] certificateBytes     MFi certificate buffer.
     * @param      maxCertificateBytes  Capacity of MFi certificate buffer.
     * @param[out] numCertificateBytes  Effective length of MFi certificate buffer.
     *
     * @return kHAPError_None           If successful.
     * @return kHAPError_Unknown        If communication with the MFi component failed.
     * @return kHAPError_OutOfResources If out of resources to process request.
     */
    HAP_RESULT_USE_CHECK
    HAPError (*copyCertificate)(
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
     * @return kHAPError_Unknown        If communication with the MFi component failed.
     * @return kHAPError_OutOfResources If out of resources to process request.
     */
    HAP_RESULT_USE_CHECK
    HAPError (*createSignature)(
            HAPAccessoryServerRef* server,
            const void* challengeBytes,
            size_t numChallengeBytes,
            void* signatureBytes,
            size_t maxSignatureBytes,
            size_t* numSignatureBytes);
} HAPMFiAuth;

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
