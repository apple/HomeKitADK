// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"
#include "HAPCrypto.h"

void HAPRawBufferZero(void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    uint8_t* b = bytes;

    for (size_t i = 0; i < numBytes; i++) {
        b[i] = 0;
    }
}

void HAPRawBufferCopyBytes(void* destinationBytes, const void* sourceBytes, size_t numBytes) {
    HAPPrecondition(destinationBytes);
    HAPPrecondition(sourceBytes);

    uint8_t* destination = destinationBytes;
    const uint8_t* source = sourceBytes;

    if (destinationBytes < sourceBytes) {
        for (size_t i = 0; i < numBytes; i++) {
            destination[i] = source[i];
        }
    } else if (destinationBytes > sourceBytes) {
        for (size_t i = numBytes; i > 0; i--) {
            destination[i - 1] = source[i - 1];
        }
    }
}

HAP_RESULT_USE_CHECK
bool HAPRawBufferAreEqual(const void* bytes, const void* otherBytes, size_t numBytes) {
    HAPPrecondition(bytes);
    HAPPrecondition(otherBytes);

    if (!numBytes) {
        return true;
    }
    return HAP_constant_time_equal(bytes, otherBytes, numBytes) == 1;
}

HAP_RESULT_USE_CHECK
bool HAPRawBufferIsZero(const void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    if (!numBytes) {
        return true;
    }
    return HAP_constant_time_is_zero(bytes, numBytes) == 1;
}
