// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#undef HAP_DISALLOW_USE_IGNORED
#define HAP_DISALLOW_USE_IGNORED 1

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPBitSetContainsInternal(const uint8_t* bitSet, size_t numBytes, uint8_t bitIndex) {
    HAPPrecondition(bitSet);

    size_t byteIndex = bitIndex / CHAR_BIT;
    HAPPrecondition(byteIndex < numBytes);
    uint8_t bitMask = (uint8_t)(1u << (uint8_t)(bitIndex % CHAR_BIT));

    return bitSet[byteIndex] & bitMask;
}

void HAPBitSetInsertInternal(uint8_t* bitSet, size_t numBytes, uint8_t bitIndex) {
    HAPPrecondition(bitSet);

    size_t byteIndex = bitIndex / CHAR_BIT;
    HAPPrecondition(byteIndex < numBytes);
    uint8_t bitMask = (uint8_t)(1u << (uint8_t)(bitIndex % CHAR_BIT));

    bitSet[byteIndex] |= bitMask;
}

void HAPBitSetRemoveInternal(uint8_t* bitSet, size_t numBytes, uint8_t bitIndex) {
    HAPPrecondition(bitSet);

    size_t byteIndex = bitIndex / CHAR_BIT;
    HAPPrecondition(byteIndex < numBytes);
    uint8_t bitMask = (uint8_t)(1u << (uint8_t)(bitIndex % CHAR_BIT));

    bitSet[byteIndex] &= (uint8_t) ~bitMask;
}
