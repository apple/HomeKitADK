// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

/**
 * Length of AAD data in the IP security protocol.
 */
#define kHAPIPSecurityProtocol_NumAADBytes ((size_t) 2)

HAP_RESULT_USE_CHECK
size_t HAPIPSecurityProtocolGetNumEncryptedBytes(size_t numPlaintextBytes) {
    size_t numEncryptedBytes =
            (numPlaintextBytes / kHAPIPSecurityProtocol_MaxFrameBytes) *
            (kHAPIPSecurityProtocol_NumAADBytes + kHAPIPSecurityProtocol_MaxFrameBytes + CHACHA20_POLY1305_TAG_BYTES);
    if (numPlaintextBytes % kHAPIPSecurityProtocol_MaxFrameBytes != 0) {
        numEncryptedBytes += kHAPIPSecurityProtocol_NumAADBytes +
                             (numPlaintextBytes % kHAPIPSecurityProtocol_MaxFrameBytes) + CHACHA20_POLY1305_TAG_BYTES;
    }
    return numEncryptedBytes;
}

void HAPIPSecurityProtocolEncryptData(HAPAccessoryServerRef* server_, HAPSessionRef* session, HAPIPByteBuffer* buffer) {
    HAPPrecondition(server_);
    HAPPrecondition(session);
    HAPPrecondition(buffer);
    HAPPrecondition(buffer->data);
    HAPPrecondition(buffer->position <= buffer->limit);
    HAPPrecondition(buffer->limit <= buffer->capacity);

    HAPError err;

    size_t numEncryptedBytes = HAPIPSecurityProtocolGetNumEncryptedBytes(buffer->limit - buffer->position);

    HAPAssert(numEncryptedBytes <= buffer->capacity);
    HAPAssert(buffer->position <= buffer->capacity - numEncryptedBytes);

    size_t position = buffer->position;

    while (position < buffer->limit) {
        size_t numFrameBytes = buffer->limit - position > kHAPIPSecurityProtocol_MaxFrameBytes ?
                                       kHAPIPSecurityProtocol_MaxFrameBytes :
                                       buffer->limit - position;

        HAPRawBufferCopyBytes(
                &buffer
                         ->data[position + numFrameBytes + kHAPIPSecurityProtocol_NumAADBytes +
                                CHACHA20_POLY1305_TAG_BYTES],
                &buffer->data[position + numFrameBytes],
                buffer->limit - (position + numFrameBytes));

        HAPRawBufferCopyBytes(&buffer->data[position + sizeof(uint16_t)], &buffer->data[position], numFrameBytes);
        HAPWriteLittleUInt16(&buffer->data[position], numFrameBytes);

        err = HAPSessionEncryptControlMessageWithAAD(
                server_,
                session,
                /* ciphertext: */
                &buffer->data[position + kHAPIPSecurityProtocol_NumAADBytes],
                /* plaintext: */
                &buffer->data[position + kHAPIPSecurityProtocol_NumAADBytes],
                /* plaintext length: */
                numFrameBytes,
                /* aad: */
                &buffer->data[position],
                /* aad length: */
                kHAPIPSecurityProtocol_NumAADBytes);
        HAPAssert(!err);

        position += numFrameBytes + kHAPIPSecurityProtocol_NumAADBytes + CHACHA20_POLY1305_TAG_BYTES;
        buffer->limit += kHAPIPSecurityProtocol_NumAADBytes + CHACHA20_POLY1305_TAG_BYTES;

        HAPAssert(position <= buffer->limit);
        HAPAssert(buffer->limit <= buffer->capacity);
    }
}

HAP_RESULT_USE_CHECK
HAPError HAPIPSecurityProtocolDecryptData(
        HAPAccessoryServerRef* server_,
        HAPSessionRef* session,
        HAPIPByteBuffer* buffer) {
    HAPPrecondition(server_);
    HAPPrecondition(session);
    HAPPrecondition(buffer);
    HAPPrecondition(buffer->data);
    HAPPrecondition(buffer->position <= buffer->limit);
    HAPPrecondition(buffer->limit <= buffer->capacity);

    HAPError err;

    for (;;) {
        if (buffer->limit - buffer->position < kHAPIPSecurityProtocol_NumAADBytes) {
            break;
        }

        size_t numFrameBytes = HAPReadLittleUInt16(&buffer->data[buffer->position]);
        if (numFrameBytes > kHAPIPSecurityProtocol_MaxFrameBytes) {
            return kHAPError_InvalidData;
        }

        if (buffer->limit - buffer->position <
            +numFrameBytes + kHAPIPSecurityProtocol_NumAADBytes + CHACHA20_POLY1305_TAG_BYTES) {
            break;
        }

        err = HAPSessionDecryptControlMessageWithAAD(
                server_,
                session,
                /* plaintext: */
                &buffer->data[buffer->position],
                /* ciphertext: */
                &buffer->data[buffer->position + kHAPIPSecurityProtocol_NumAADBytes],
                /* ciphertext length: */
                numFrameBytes + CHACHA20_POLY1305_TAG_BYTES,
                /* aad: */
                &buffer->data[buffer->position],
                /* aad length: */
                kHAPIPSecurityProtocol_NumAADBytes);
        if (err) {
            return kHAPError_InvalidData;
        }

        HAPRawBufferCopyBytes(
                &buffer->data[buffer->position + numFrameBytes],
                &buffer
                         ->data[buffer->position + numFrameBytes + kHAPIPSecurityProtocol_NumAADBytes +
                                CHACHA20_POLY1305_TAG_BYTES],
                buffer->limit - (buffer->position + numFrameBytes + kHAPIPSecurityProtocol_NumAADBytes +
                                 CHACHA20_POLY1305_TAG_BYTES));

        buffer->position += numFrameBytes;
        buffer->limit -= kHAPIPSecurityProtocol_NumAADBytes + CHACHA20_POLY1305_TAG_BYTES;

        HAPAssert(buffer->position <= buffer->limit);
        HAPAssert(buffer->limit <= buffer->capacity);
    }

    return kHAPError_None;
}
