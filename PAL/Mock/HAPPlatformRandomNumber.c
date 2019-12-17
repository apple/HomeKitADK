// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

void HAPPlatformRandomNumberFill(void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    for (size_t o = 0; o < numBytes; o++) {
        static uint64_t seed[2] = { 1, 1 };
        uint64_t x = seed[0];
        uint64_t y = seed[1];
        seed[0] = y;
        x ^= x << 23;       // a
        x ^= x >> 17;       // b
        x ^= y ^ (y >> 26); // c
        seed[1] = x;
        ((uint8_t*) bytes)[o] = (uint8_t)(x + y);
    }
}
