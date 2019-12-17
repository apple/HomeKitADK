// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

void* _Nullable HAPTLVScratchBufferAlloc(
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* maxScratchBytes,
        size_t numBytes) {
    if (!scratchBytes || !*scratchBytes || !maxScratchBytes || !*maxScratchBytes) {
        return NULL;
    }

    uint8_t* bytes = *scratchBytes;
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_MSVC(4146)
    uintptr_t o = (uintptr_t)(-(uintptr_t) bytes & 0x03);
    if (*maxScratchBytes < numBytes + o) {
        return NULL;
    }
    HAP_DIAGNOSTIC_POP

    *scratchBytes = &bytes[o + numBytes];
    *maxScratchBytes -= o + numBytes;
    return &bytes[o];
}

void* _Nullable HAPTLVScratchBufferAllocUnaligned(
        void* _Nonnull* _Nonnull scratchBytes,
        size_t* numScratchBytes,
        size_t numBytes) {
    if (!scratchBytes || !*scratchBytes || !numScratchBytes || !*numScratchBytes) {
        return NULL;
    }

    uint8_t* bytes = *scratchBytes;

    if (*numScratchBytes < numBytes) {
        return NULL;
    }
    *scratchBytes = &bytes[numBytes];
    *numScratchBytes -= numBytes;
    return bytes;
}
