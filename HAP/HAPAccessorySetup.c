// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

HAP_RESULT_USE_CHECK
bool HAPAccessorySetupIsValidSetupCode(const char* stringValue) {
    HAPPrecondition(stringValue);

    if (HAPStringGetNumBytes(stringValue) != sizeof(HAPSetupCode) - 1) {
        return false;
    }

    uint8_t numEqual = 0, numAscending = 0, numDescending = 0;

    char previousCharacter = '\0';
    for (size_t i = 0; i < sizeof(HAPSetupCode) - 1; i++) {
        if (i == 3 || i == 6) {
            if (stringValue[i] != '-') {
                return false;
            }
        } else {
            if (stringValue[i] < '0' || stringValue[i] > '9') {
                return false;
            }
            numEqual += stringValue[i] == previousCharacter;
            numAscending += stringValue[i] == previousCharacter + 1;
            numDescending += stringValue[i] == previousCharacter - 1;
            previousCharacter = stringValue[i];
        }
    }

    // All equal, ascending, or descending?
    return numEqual != 7 && ((uint8_t)((uint8_t) stringValue[0] ^ '1') | (uint8_t)(numAscending ^ 7)) &&
           ((uint8_t)((uint8_t) stringValue[0] ^ '8') | (uint8_t)(numDescending ^ 7));
}

void HAPAccessorySetupGenerateRandomSetupCode(HAPSetupCode* setupCode) {
    HAPPrecondition(setupCode);

    do {
        // Format: XXX-XX-XXX with X being digit from 0-9.
        for (size_t i = 0; i < sizeof setupCode->stringValue - 1; i++) {
            if (i == 3 || i == 6) {
                setupCode->stringValue[i] = '-';
                continue;
            }

            // Add random digit.
            uint8_t randomByte;
            do {
                HAPPlatformRandomNumberFill(&randomByte, sizeof randomByte);
            } while ((uint8_t)(randomByte & 0xFU) > 9);
            setupCode->stringValue[i] = (char) ('0' + (char) (randomByte & 0xFU));
        }
        setupCode->stringValue[sizeof setupCode->stringValue - 1] = '\0';
    } while (!HAPAccessorySetupIsValidSetupCode(setupCode->stringValue));
}

HAP_RESULT_USE_CHECK
bool HAPAccessorySetupIsValidSetupID(const char* stringValue) {
    HAPPrecondition(stringValue);

    if (HAPStringGetNumBytes(stringValue) != sizeof(HAPSetupID) - 1) {
        return false;
    }

    for (size_t i = 0; i < sizeof(HAPSetupID) - 1; i++) {
        char c = stringValue[i];
        if ((uint8_t)((uint8_t)(c < '0') | (uint8_t)(c > '9')) & (uint8_t)((uint8_t)(c < 'A') | (uint8_t)(c > 'Z'))) {
            return false;
        }
    }

    return true;
}

void HAPAccessorySetupGenerateRandomSetupID(HAPSetupID* setupID) {
    HAPPrecondition(setupID);

    for (size_t i = 0; i < sizeof setupID->stringValue - 1; i++) {
        char c;
        do {
            HAPPlatformRandomNumberFill(&c, sizeof c);
        } while ((uint8_t)((uint8_t)(c < '0') | (uint8_t)(c > '9')) &
                 (uint8_t)((uint8_t)(c < 'A') | (uint8_t)(c > 'Z')));
        setupID->stringValue[i] = c;
    }
    setupID->stringValue[sizeof setupID->stringValue - 1] = '\0';
}

/** Prefix of the setup payload. */
#define HAPSetupPayloadPrefix ("X-HM://")

HAP_STATIC_ASSERT(
        sizeof(HAPSetupPayload) == sizeof HAPSetupPayloadPrefix - 1 + 9 + sizeof(HAPAccessorySetupSetupHash) + 1,
        HAPSetupPayload_FitsTemplate);

void HAPAccessorySetupGetSetupPayload(
        HAPSetupPayload* setupPayload,
        const HAPSetupCode* _Nullable setupCode,
        const HAPSetupID* _Nullable setupID,
        HAPAccessorySetupSetupPayloadFlags flags,
        HAPAccessoryCategory category) {
    HAPPrecondition(setupPayload);
    HAPPrecondition(!setupCode || !flags.isPaired);
    HAPPrecondition(!setupID || !flags.isPaired);
    HAPPrecondition((setupCode && setupID) || (!setupCode && !setupID));
    HAPPrecondition(flags.ipSupported || flags.bleSupported);
    HAPPrecondition(category > 0);

    HAPRawBufferZero(setupPayload->stringValue, sizeof setupPayload->stringValue);

    char* pl = setupPayload->stringValue;

    // Prefix.
    HAPRawBufferCopyBytes(pl, HAPSetupPayloadPrefix, sizeof HAPSetupPayloadPrefix - 1);
    pl += sizeof HAPSetupPayloadPrefix - 1;

    // Raw VersionCategoryFlagsAndSetupCode.
    uint64_t code = (uint64_t)(
            /* 45-43 - Version  */ ((uint64_t) 0x0U << 43U) |
            /* 42-39 - Reserved */ ((uint64_t) 0x0U << 39U) |
            /* 38-31 - Category */ ((uint64_t)(category & 0xFFU) << 31U) |
            /*    29 - BLE      */ ((uint64_t)(flags.bleSupported ? 1U : 0U) << 29U) |
            /*    28 - IP       */ ((uint64_t)(flags.ipSupported ? 1U : 0U) << 28U) |
            /*    27 - Paired   */ ((uint64_t)(flags.isPaired ? 1U : 0U) << 27U));

    if (setupCode) {
        code |= /* 26-00 - Setup code */ (uint64_t)(
                (uint64_t)(setupCode->stringValue[0] - '0') * 10000000U +
                (uint64_t)(setupCode->stringValue[1] - '0') * 1000000U +
                (uint64_t)(setupCode->stringValue[2] - '0') * 100000U +
                (uint64_t)(setupCode->stringValue[4] - '0') * 10000U +
                (uint64_t)(setupCode->stringValue[5] - '0') * 1000U +
                (uint64_t)(setupCode->stringValue[7] - '0') * 100U + (uint64_t)(setupCode->stringValue[8] - '0') * 10U +
                (uint64_t)(setupCode->stringValue[9] - '0') * 1U);
    }

    // Base36 encode.
    for (int i = 0; i < 9; i++) {
        // Divide code by 36 and get remainder.
        uint64_t q;
        uint32_t r, d;
        uint64_t x = code;
        q = x - (x >> 3U);
        q = q + (q >> 6U);
        q = q + (q >> 12U);
        q = q + (q >> 24U);
        q = q + (q >> 48U); // not needed for x < 2^48
        /* q = x * 8/9 +0/-5 */
        q = q >> 5U;
        /* q = x / 36 +0/-1 */
        r = (uint32_t) x - (uint32_t) q * 36U;
        /* 0 <= r < 72 */
        d = (r + 28U) >> 6U;
        /* d = 1 if r > 35 */
        code = q + d;
        uint8_t c = (uint8_t)(r - d * 36U);
        HAPAssert(c < 36U);

        // Constant time transformation to avoid leaking secret data through side channels.

        //    Index: 0          10
        // Alphabet: 0123456789 ABCDEFGHIJKLMNOPQRSTUVWXYZ
        //    ASCII: 48      57 65                      90

        // Transform alphabet index into ASCII.
        c += '0' + (((int8_t)(c + 0x80 - 10) >> 7) & ('A' - '9' - 1)); // Skip gap between 9 and A.

        // Big endian encode.
        pl[8 - i] = (char) c;
    }
    pl += 9;

    // SetupID.
    if (setupID) {
        HAPRawBufferCopyBytes(pl, setupID->stringValue, sizeof setupID->stringValue - 1);
        pl += sizeof setupID->stringValue - 1;
    } else {
        for (size_t i = 0; i < sizeof setupID->stringValue - 1; i++) {
            *pl = '0';
            pl++;
        }
    }

    // Done.
    HAPAssert(!*pl);
    HAPAssert(pl - setupPayload->stringValue <= (long) sizeof setupPayload->stringValue);
}

void HAPAccessorySetupGetSetupHash(
        HAPAccessorySetupSetupHash* setupHash,
        const HAPSetupID* setupID,
        const HAPDeviceIDString* deviceIDString) {
    HAPPrecondition(setupHash);
    HAPPrecondition(setupID);
    HAPPrecondition(deviceIDString);

    // Concatenate setup ID and Device ID.
    uint8_t hash[SHA512_BYTES];
    HAPAssert(sizeof setupID->stringValue - 1 + sizeof deviceIDString->stringValue - 1 <= sizeof hash);
    size_t o = 0;
    HAPRawBufferCopyBytes(&hash[o], setupID->stringValue, sizeof setupID->stringValue - 1);
    o += sizeof setupID->stringValue - 1;
    HAPRawBufferCopyBytes(&hash[o], deviceIDString->stringValue, sizeof deviceIDString->stringValue - 1);
    o += sizeof deviceIDString->stringValue - 1;

    // SHA512.
    HAP_sha512(hash, hash, o);

    // Truncate.
    HAPAssert(sizeof setupHash->bytes <= sizeof hash);
    HAPRawBufferCopyBytes(setupHash->bytes, hash, sizeof setupHash->bytes);
}
