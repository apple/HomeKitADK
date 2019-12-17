// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_SECURITY_PROTOCOL_H
#define HAP_IP_SECURITY_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Maximum frame length in the IP security protocol.
 */
#define kHAPIPSecurityProtocol_MaxFrameBytes ((size_t) 1024)

/**
 * Computes the number of encrypted bytes given the number of plaintext bytes.
 *
 * @param      numPlaintextBytes    Number of plaintext bytes.
 *
 * @return Number of encrypted bytes given the number of plaintext bytes.
 */
HAP_RESULT_USE_CHECK
size_t HAPIPSecurityProtocolGetNumEncryptedBytes(size_t numPlaintextBytes);

/**
 * Encrypts data to be sent over a HomeKit session.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the data will be sent.
 * @param      buffer               Plaintext data to be encrypted.
 */
void HAPIPSecurityProtocolEncryptData(HAPAccessoryServerRef* server, HAPSessionRef* session, HAPIPByteBuffer* buffer);

/**
 * Decrypts data received over a HomeKit session.
 *
 * @param      server               Accessory server.
 * @param      session              The session over which the data has been received.
 * @param      buffer               Encrypted data to be decrypted.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_InvalidData    If the controller sent a malformed data, or decryption failed.
 */
HAP_RESULT_USE_CHECK
HAPError HAPIPSecurityProtocolDecryptData(
        HAPAccessoryServerRef* server,
        HAPSessionRef* session,
        HAPIPByteBuffer* buffer);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
