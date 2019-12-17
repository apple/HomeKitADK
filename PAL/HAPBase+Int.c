// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

#define RETURN_INT_FROM_STRING(type, description, value, maxValue, minValue) \
    do { \
        HAPPrecondition(description); \
        HAPPrecondition(value); \
\
        const char* c = description; \
\
        /* Read optional sign. */ \
        bool isNegative = false; \
        if (*c == '+' || *c == '-') { \
            isNegative = *c == '-'; \
            c++; \
        } \
\
        /* Read value. */ \
        if (!*c) { \
            return kHAPError_InvalidData; \
        } \
        *(value) = 0; \
        for (; *c; c++) { \
            if (*c < '0' || *c > '9') { \
                return kHAPError_InvalidData; \
            } \
            type digit = (type)(*c - '0'); \
\
            if (!isNegative) { \
                if (*(value) > (maxValue) / 10) { \
                    return kHAPError_InvalidData; \
                } \
                *(value) *= 10; \
\
                if (*(value) > (maxValue) -digit) { \
                    return kHAPError_InvalidData; \
                } \
                *(value) += digit; \
            } else { \
                if (*(value) < (minValue) / 10) { \
                    return kHAPError_InvalidData; \
                } \
                *(value) *= 10; \
\
                if (*(value) < (minValue) + digit) { \
                    return kHAPError_InvalidData; \
                } \
                *(value) -= digit; \
            } \
        } \
\
        return kHAPError_None; \
    } while (0)

HAP_RESULT_USE_CHECK
HAPError HAPUInt64FromString(const char* description, uint64_t* value) {
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_ARMCC(186)
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wtype-limits")
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe186)
    RETURN_INT_FROM_STRING(uint64_t, description, value, UINT64_MAX, 0);
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe186)
    HAP_DIAGNOSTIC_POP
}

HAP_RESULT_USE_CHECK
HAPError HAPInt64FromString(const char* description, int64_t* value) {
    RETURN_INT_FROM_STRING(int64_t, description, value, INT64_MAX, INT64_MIN);
}

#define RETURN_INT_NUM_DESCRIPTION_BYTES(value) \
    do { \
        size_t numBytes = 0; \
        if ((value) < 0) { \
            numBytes++; \
        } \
        do { \
            numBytes++; \
            (value) /= 10; \
        } while (value); \
        return numBytes; \
    } while (0)

HAP_RESULT_USE_CHECK
size_t HAPInt32GetNumDescriptionBytes(int32_t value) {
    RETURN_INT_NUM_DESCRIPTION_BYTES(value);
}

HAP_RESULT_USE_CHECK
size_t HAPUInt64GetNumDescriptionBytes(uint64_t value) {
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_ARMCC(186)
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wtype-limits")
    HAP_DIAGNOSTIC_IGNORED_ICCARM(Pe186)
    RETURN_INT_NUM_DESCRIPTION_BYTES(value);
    HAP_DIAGNOSTIC_RESTORE_ICCARM(Pe186)
    HAP_DIAGNOSTIC_POP
}

#define RETURN_INT_DESCRIPTION(value, bytes, maxBytes) \
    do { \
        char* b = (bytes) + (maxBytes); \
        size_t numBytes = 0; \
        if (numBytes++ >= (maxBytes)) { \
            return kHAPError_OutOfResources; \
        } \
        *--b = '\0'; \
        do { \
            if (numBytes++ >= (maxBytes)) { \
                return kHAPError_OutOfResources; \
            } \
            *--b = '0' + (value) % 10; \
            (value) /= 10; \
        } while (value); \
        HAPRawBufferCopyBytes(bytes, b, numBytes); \
        return kHAPError_None; \
    } while (0)

HAP_RESULT_USE_CHECK
HAPError HAPUInt64GetDescription(uint64_t value, char* bytes, size_t maxBytes) {
    HAP_DIAGNOSTIC_PUSH
    HAP_DIAGNOSTIC_IGNORED_ARMCC(186)
    HAP_DIAGNOSTIC_IGNORED_GCC("-Wtype-limits")
    RETURN_INT_DESCRIPTION(value, bytes, maxBytes);
    HAP_DIAGNOSTIC_POP
}

HAP_RESULT_USE_CHECK
HAPError HAPUInt64GetHexDescription(uint64_t value, char* bytes, size_t maxBytes, HAPLetterCase letterCase) {
    size_t chars = 16;
    int shift = 60;
    while (chars > 1 && ((value >> shift) & 0xF) == 0) {
        shift -= 4;
        chars--;
    }
    if (chars >= maxBytes) {
        return kHAPError_OutOfResources;
    }
    int i = 0;
    while (shift >= 0) {
        int digit = (int) ((value >> shift) & 0xF);
        bytes[i++] = (char) (digit + (digit < 10 ? '0' : (char) letterCase - 10));
        shift -= 4;
    }
    bytes[i] = 0;
    return kHAPError_None;
}
