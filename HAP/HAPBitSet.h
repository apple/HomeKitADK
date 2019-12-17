// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#ifndef HAP_UTILS_H
#define HAP_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "HAP+Internal.h"

#if __has_feature(nullability)
#pragma clang assume_nonnull begin
#endif

/**
 * Indicates whether the specified bit is set in a bit set.
 *
 * @param      bitSet               Byte array representing the bit set.
 * @param      bitIndex             Bit index.
 *
 * @return true                     If the specified bit is set.
 * @return false                    Otherwise.
 */
#define HAPBitSetContains(bitSet, bitIndex) HAPBitSetContainsInternal((bitSet), sizeof(bitSet), (bitIndex))

/**
 * Inserts the specified bit into a bit set.
 *
 * @param      bitSet               Byte array representing the bit set.
 * @param      bitIndex             Bit index.
 */
#define HAPBitSetInsert(bitSet, bitIndex) HAPBitSetInsertInternal((bitSet), sizeof(bitSet), (bitIndex))

/**
 * Removes the specified bit from a bit set.
 *
 * @param      bitSet               Byte array representing the bit set.
 * @param      bitIndex             Bit index.
 */
#define HAPBitSetRemove(bitSet, bitIndex) HAPBitSetRemoveInternal((bitSet), sizeof(bitSet), (bitIndex))

//----------------------------------------------------------------------------------------------------------------------
// Internal functions. Do not use directly.

/**@cond */
HAP_RESULT_USE_CHECK
bool HAPBitSetContainsInternal(const uint8_t* bitSet, size_t numBytes, uint8_t bitIndex);
HAP_DISALLOW_USE(HAPBitSetContainsInternal)

void HAPBitSetInsertInternal(uint8_t* bitSet, size_t numBytes, uint8_t bitIndex);
HAP_DISALLOW_USE(HAPBitSetInsertInternal)

void HAPBitSetRemoveInternal(uint8_t* bitSet, size_t numBytes, uint8_t bitIndex);
HAP_DISALLOW_USE(HAPBitSetRemoveInternal)
/**@endcond */

#if __has_feature(nullability)
#pragma clang assume_nonnull end
#endif

#ifdef __cplusplus
}
#endif

#endif
