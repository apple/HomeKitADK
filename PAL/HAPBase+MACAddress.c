// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

HAP_RESULT_USE_CHECK
HAPError HAPMACAddressGetDescription(const HAPMACAddress* value, char* bytes, size_t maxBytes) {
    HAPPrecondition(value);
    HAPPrecondition(bytes);

    return HAPStringWithFormat(
            bytes,
            maxBytes,
            "%02X:%02X:%02X:%02X:%02X:%02X",
            value->bytes[0],
            value->bytes[1],
            value->bytes[2],
            value->bytes[3],
            value->bytes[4],
            value->bytes[5]);
}

HAP_RESULT_USE_CHECK
bool HAPMACAddressAreEqual(const HAPMACAddress* value, const HAPMACAddress* otherValue) {
    HAPPrecondition(value);
    HAPPrecondition(otherValue);

    return HAPRawBufferAreEqual(value->bytes, otherValue->bytes, sizeof value->bytes);
}

HAP_RESULT_USE_CHECK
static HAPError UInt8FromHexDigit(char description, uint8_t* value) {
    HAPPrecondition(value);

    if ('0' <= description && description <= '9') {
        *value = (uint8_t)(description - '0');
    } else if ('A' <= description && description <= 'F') {
        *value = ((uint8_t)(description - 'A')) + (uint8_t) 10;
    } else if ('a' <= description && description <= 'f') {
        *value = ((uint8_t)(description - 'a')) + (uint8_t) 10;
    } else {
        return kHAPError_InvalidData;
    }
    HAPAssert(*value <= 0xf);
    return kHAPError_None;
}

HAP_RESULT_USE_CHECK
HAPError HAPMACAddressFromString(const char* description, HAPMACAddress* value) {
    HAPPrecondition(description);
    HAPPrecondition(value);

    HAPError err;

    HAPRawBufferZero(value, sizeof *value);

    size_t numDescriptionBytes = HAPStringGetNumBytes(description);
    if (numDescriptionBytes != 3 + 3 + 3 + 3 + 3 + 2) {
        return kHAPError_InvalidData;
    }

    for (size_t i = 0; i < numDescriptionBytes; i++) {
        switch (i % 3) {
            case 0: {
                uint8_t hexDigit;
                err = UInt8FromHexDigit(description[i], &hexDigit);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    return err;
                }

                HAPAssert(i / 3 < sizeof value->bytes);
                value->bytes[i / 3] = (uint8_t)(hexDigit << 4);
            } break;
            case 1: {
                uint8_t hexDigit;
                err = UInt8FromHexDigit(description[i], &hexDigit);
                if (err) {
                    HAPAssert(err == kHAPError_InvalidData);
                    return err;
                }

                HAPAssert(i / 3 < sizeof value->bytes);
                value->bytes[i / 3] |= hexDigit;
            } break;
            case 2: {
                if (description[i] != ':') {
                    return kHAPError_InvalidData;
                }
            } break;
            default: {
            }
                HAPFatalError();
        }
    }

    return kHAPError_None;
}
