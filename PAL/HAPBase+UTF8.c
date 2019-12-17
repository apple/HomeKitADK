// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

HAP_RESULT_USE_CHECK
bool HAPUTF8IsValidData(const void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - Table 3-7, page 94.

    int error = 0;  // Error state in bit 0.
    int state = 0;  // Number of leading 1 bits == number of outstanding continuation bytes.
    int prefix = 0; // Prefix byte if value is second byte.

    // state     value    -> more  first  second -> error  state'    prefix
    // 0xxxxxxx  0xxxxxxx     0      0      x        0    00000000  00000000
    // 0xxxxxxx  10xxxxxx     0      1      0        1
    // 0xxxxxxx  1110xxxx     0      1      1        0    110xxxxx  1110xxxx
    // 110xxxx0  0xxxxxxx     1      0      x        1
    // 110xxxx0  10xxxxxx     1      1      0        0    10xxxxxx  00000000
    // 110xxxx0  110xxxxx     1      1      1        1

    for (size_t i = 0; i < numBytes; i++) {
        int value = ((const uint8_t*) bytes)[i];
        int more = state >> 7;         // More continuation bytes expected.
        int first = value >> 7;        // First bit.
        int second = (value >> 6) & 1; // Second bit.

        // Illegal value.
        error |= ((uint8_t)(value - 0xC0) - 2) >> 8; // value == 1100000x
        error |= (0xF4 - value) >> 8;                // value >= 11110101

        // Illegal second byte.
        int bits = value >> 5;
        error |= (((uint8_t)(prefix - 0xE0) - 1) >> 8) & ~bits; // 11100000  xx0xxxxx
        error |= (((uint8_t)(prefix - 0xED) - 1) >> 8) & bits;  // 11101101  xx1xxxxx
        bits |= value >> 4;
        error |= (((uint8_t)(prefix - 0xF0) - 1) >> 8) & ~bits; // 11110000  xx00xxxx
        error |= (((uint8_t)(prefix - 0xF4) - 1) >> 8) & bits;  // 11110100  xx11xxxx

        // Illegal continuation.
        error |= (first & ~second) ^ more;

        // New state.
        prefix = -(first & second) & value;
        state = (uint8_t)((prefix | (-more & state)) << 1);
    }

    // Missing continuations.
    error |= state >> 7;

    return (bool) (1 & ~error);
}
