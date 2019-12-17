// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

#define kPattern1a 0x23                   //  6 bit, 1 byte: '#'
#define kPattern1b 0x61                   //  7 bit, 1 byte: 'a'
#define kPattern2a 0xC3, 0xA4             //  8 bit, 2 byte: 'ä'
#define kPattern2b 0xD0, 0x96             // 11 bit, 2 byte: cyrillic zhe
#define kPattern3a 0xE0, 0xBC, 0x80       // 12 bit, 3 byte: tibetan om
#define kPattern3b 0xEF, 0xB9, 0xA0       // 16 bit, 3 byte: small &
#define kPattern4a 0xF0, 0x90, 0x8C, 0xB2 // 17 bit, 4 byte: gothic giba

static const uint8_t test0[] = { kPattern1a, kPattern1b };
static const uint8_t test1[] = { kPattern2a, kPattern2b };
static const uint8_t test2[] = { kPattern1a, kPattern2a, kPattern2b };
static const uint8_t test3[] = { kPattern2a, kPattern2b, kPattern1b };
static const uint8_t test4[] = { kPattern1a, kPattern2a, kPattern2b, kPattern1b };
static const uint8_t test5[] = { kPattern3a, kPattern3b };
static const uint8_t test6[] = { kPattern1a, kPattern3a, kPattern3b, kPattern1b };
static const uint8_t test7[] = { kPattern4a };
static const uint8_t test8[] = { kPattern1a, kPattern4a, kPattern1b };
static const uint8_t test9[] = { kPattern1a, kPattern2a, kPattern3a, kPattern4a };

// Wrong continuation.
static const uint8_t testA[] = { 0xA4 };
static const uint8_t testB[] = { kPattern1a, 0x96 };
static const uint8_t testC[] = { kPattern2a, 0xB2 };

// Missing continuation.
static const uint8_t testD[] = { 0xC3, kPattern1a };
static const uint8_t testE[] = { kPattern1a, 0xC3, kPattern2b };
static const uint8_t testF[] = { 0xF0, 0x96, 0xB9, kPattern3a };
static const uint8_t testG[] = { kPattern2b, 0xEF, 0xBC };

int main() {
    HAPAssert(HAPUTF8IsValidData(test0, 0));

    HAPAssert(HAPUTF8IsValidData(test0, sizeof test0));
    HAPAssert(HAPUTF8IsValidData(test1, sizeof test1));
    HAPAssert(HAPUTF8IsValidData(test2, sizeof test2));
    HAPAssert(HAPUTF8IsValidData(test3, sizeof test3));
    HAPAssert(HAPUTF8IsValidData(test4, sizeof test4));
    HAPAssert(HAPUTF8IsValidData(test5, sizeof test5));
    HAPAssert(HAPUTF8IsValidData(test6, sizeof test6));
    HAPAssert(HAPUTF8IsValidData(test7, sizeof test7));
    HAPAssert(HAPUTF8IsValidData(test8, sizeof test8));
    HAPAssert(HAPUTF8IsValidData(test9, sizeof test9));
    HAPAssert(HAPUTF8IsValidData(test0, sizeof test0));

    HAPAssert(!HAPUTF8IsValidData(testA, sizeof testA));
    HAPAssert(!HAPUTF8IsValidData(testB, sizeof testB));
    HAPAssert(!HAPUTF8IsValidData(testC, sizeof testC));
    HAPAssert(!HAPUTF8IsValidData(testD, sizeof testD));
    HAPAssert(!HAPUTF8IsValidData(testE, sizeof testE));
    HAPAssert(!HAPUTF8IsValidData(testF, sizeof testF));
    HAPAssert(!HAPUTF8IsValidData(testG, sizeof testG));

    return 0;
}
