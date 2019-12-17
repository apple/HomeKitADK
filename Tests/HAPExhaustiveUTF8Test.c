// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

static bool HAPUTF8IsValidDataRef(const void* bytes, size_t numBytes) {
    HAPPrecondition(bytes);

    // See http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - Table 3-7, page 94.

    const uint8_t* data = (const uint8_t*) bytes;
    bool isValidData = true;

    size_t i = 0;
    while ((i < numBytes) && isValidData) {
        uint8_t a = data[i];
        if (a <= 0x7f) {
            i += 1;
        } else if ((0xc2 <= a) && (a <= 0xdf)) { // 11000010 -> 0x80 - 11bit
            if (numBytes - i >= 2) {
                uint8_t b = data[i + 1];
                if ((0x80 <= b) && (b <= 0xbf)) {
                    i += 2;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if (a == 0xe0) {
            if (numBytes - i >= 3) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                if ((0xa0 <= b) && (b <= 0xbf) && (0x80 <= c) && (c <= 0xbf)) {
                    i += 3;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if ((0xe1 <= a) && (a <= 0xec)) {
            if (numBytes - i >= 3) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                if ((0x80 <= b) && (b <= 0xbf) && (0x80 <= c) && (c <= 0xbf)) {
                    i += 3;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if (a == 0xed) {
            if (numBytes - i >= 3) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                if ((0x80 <= b) && (b <= 0x9f) && (0x80 <= c) && (c <= 0xbf)) {
                    i += 3;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if ((0xee <= a) && (a <= 0xef)) {
            if (numBytes - i >= 3) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                if ((0x80 <= b) && (b <= 0xbf) && (0x80 <= c) && (c <= 0xbf)) {
                    i += 3;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if (a == 0xf0) {
            if (numBytes - i >= 4) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                uint8_t d = data[i + 3];
                if ((0x90 <= b) && (b <= 0xbf) && (0x80 <= c) && (c <= 0xbf) && (0x80 <= d) && (d <= 0xbf)) {
                    i += 4;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if ((0xf1 <= a) && (a <= 0xf3)) {
            if (numBytes - i >= 4) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                uint8_t d = data[i + 3];
                if ((0x80 <= b) && (b <= 0xbf) && (0x80 <= c) && (c <= 0xbf) && (0x80 <= d) && (d <= 0xbf)) {
                    i += 4;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else if (a == 0xf4) {
            if (numBytes - i >= 4) {
                uint8_t b = data[i + 1];
                uint8_t c = data[i + 2];
                uint8_t d = data[i + 3];
                if ((0x80 <= b) && (b <= 0x8f) && (0x80 <= c) && (c <= 0xbf) && (0x80 <= d) && (d <= 0xbf)) {
                    i += 4;
                } else {
                    isValidData = false;
                }
            } else {
                isValidData = false;
            }
        } else {
            isValidData = false;
        }
    }
    HAPAssert(((i == numBytes) && isValidData) || ((i < numBytes) && !isValidData));

    return isValidData;
}

int main() {
    for (uint32_t value = 0;; value++) {
        HAPAssert(HAPUTF8IsValidData(&value, sizeof value) == HAPUTF8IsValidDataRef(&value, sizeof value));
        if (value == UINT32_MAX) {
            break;
        }
    }

    return 0;
}
