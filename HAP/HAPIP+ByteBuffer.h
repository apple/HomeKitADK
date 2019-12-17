// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_IP_BYTE_BUFFER_H
#define HAP_IP_BYTE_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Byte buffer data structure used by IP specific functions.
 *
 * Example usage:
 * 1. The buffer is cleared using HAPIPByteBufferClear.
 * 2. The buffer is filled by appending to the data member and increasing position up to limit.
 * 3. The appended data is finalized using HAPIPByteBufferFlip.
 * 4. The data is read back by accessing the data member and increasing position up to limit.
 * 5. Read data may be discarded using HAPIPByteBufferShiftLeft.
 */
typedef struct {
    size_t capacity;
    size_t position;
    size_t limit;
    char* data;
} HAPIPByteBuffer;

/**
 * Clears a byte buffer.
 *
 * @param      byteBuffer           Byte buffer.
 */
void HAPIPByteBufferClear(HAPIPByteBuffer* byteBuffer);

/**
 * Flips a byte buffer, making available appended data for reading.
 *
 * @param      byteBuffer           Byte buffer.
 */
void HAPIPByteBufferFlip(HAPIPByteBuffer* byteBuffer);

/**
 * Discards bytes form a byte buffer.
 *
 * @param      byteBuffer           Byte buffer.
 * @param      numBytes             Number of bytes to discard.
 */
void HAPIPByteBufferShiftLeft(HAPIPByteBuffer* byteBuffer, size_t numBytes);

/**
 * Appends a formatted string to a byte buffer.
 *
 * @param      byteBuffer           Byte buffer.
 * @param      format               A format string.
 * @param      ...                  Arguments for the format string.
 *
 * @return kHAPError_None           If successful.
 * @return kHAPError_OutOfResources If the supplied buffer is not large enough.
 */
HAP_PRINTFLIKE(2, 3)
HAP_RESULT_USE_CHECK
HAPError HAPIPByteBufferAppendStringWithFormat(HAPIPByteBuffer* byteBuffer, const char* format, ...);

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
