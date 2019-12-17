// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

#define TEST_FROM_STRING(description, expectedValue) \
    do { \
        HAPPrecondition((description) != NULL); \
\
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s", description); \
        if ((expectedValue) >= 0) { \
            if ((uint64_t)(expectedValue) <= UINT64_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing UInt64..."); \
                uint64_t value; \
                err = HAPUInt64FromString(description, &value); \
                HAPAssert(!err); \
                HAPAssert(value == (uint64_t)(expectedValue)); \
            } \
            if ((uint64_t)(expectedValue) <= INT64_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int64..."); \
                int64_t value; \
                err = HAPInt64FromString(description, &value); \
                HAPAssert(!err); \
                HAPAssert(value == (int64_t)(expectedValue)); \
            } \
        } \
        if ((expectedValue) < 0) { \
            if ((int64_t)(expectedValue) >= INT64_MIN) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int64..."); \
                int64_t value; \
                err = HAPInt64FromString(description, &value); \
                HAPAssert(!err); \
                HAPAssert(value == (int64_t)(expectedValue)); \
            } \
        } \
    } while (0)

#define TEST_BORDER_CASE(description, limitThatIsExceeded) \
    do { \
        HAPPrecondition((description) != NULL); \
\
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s (expected fail)", description); \
        if ((limitThatIsExceeded) >= 0) { \
            if ((uint64_t)(limitThatIsExceeded) >= UINT64_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing UInt64..."); \
                uint64_t value; \
                err = HAPUInt64FromString(description, &value); \
                HAPAssert(err); \
            } \
            if ((uint64_t)(limitThatIsExceeded) >= INT64_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int64..."); \
                int64_t value; \
                err = HAPInt64FromString(description, &value); \
                HAPAssert(err); \
            } \
        } \
        if ((limitThatIsExceeded) <= 0) { \
            if ((int64_t)(limitThatIsExceeded) <= 0) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing UInt64..."); \
                uint64_t value; \
                err = HAPUInt64FromString(description, &value); \
                HAPAssert(err); \
            } \
            if ((int64_t)(limitThatIsExceeded) <= INT64_MIN) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int64..."); \
                int64_t value; \
                err = HAPInt64FromString(description, &value); \
                HAPAssert(err); \
            } \
        } \
    } while (0)

#define TEST_FAIL(description) \
    do { \
        HAPPrecondition((description) != NULL); \
\
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s (expected fail)", description); \
        { \
            HAPLogInfo(&kHAPLog_Default, "- Testing UInt64..."); \
            uint64_t value; \
            err = HAPUInt64FromString(description, &value); \
            HAPAssert(err); \
        } \
        { \
            HAPLogInfo(&kHAPLog_Default, "- Testing Int64..."); \
            int64_t value; \
            err = HAPInt64FromString(description, &value); \
            HAPAssert(err); \
        } \
    } while (0)

#define TEST_GET_DESCRIPTION(value, expectedDescription) \
    do { \
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s (get description)", #value); \
        if ((value) >= 0) { \
            if ((uint64_t)(value) <= UINT64_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing UInt64..."); \
                size_t actualNumBytes = HAPUInt64GetNumDescriptionBytes((uint64_t)(value)); \
                HAPAssert(actualNumBytes == sizeof(expectedDescription) - 1); \
                char description[sizeof(expectedDescription) + 1]; \
                err = HAPUInt64GetDescription((uint64_t)(value), description, sizeof(expectedDescription) + 1); \
                HAPAssert(!err); \
                HAPAssert(HAPStringAreEqual(description, (expectedDescription))); \
                err = HAPUInt64GetDescription((uint64_t)(value), description, sizeof(expectedDescription)); \
                HAPAssert(!err); \
                HAPAssert(HAPStringAreEqual(description, (expectedDescription))); \
                err = HAPUInt64GetDescription((uint64_t)(value), description, sizeof(expectedDescription) - 1); \
                HAPAssert(err == kHAPError_OutOfResources); \
            } \
            if ((uint64_t)(value) <= INT32_MAX) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int32..."); \
                size_t actualNumBytes = HAPInt32GetNumDescriptionBytes((int32_t)(value)); \
                HAPAssert(actualNumBytes == sizeof(expectedDescription) - 1); \
            } \
        } \
        if ((value) < 0) { \
            if ((int64_t)(value) >= INT32_MIN) { \
                HAPLogInfo(&kHAPLog_Default, "- Testing Int32..."); \
                size_t actualNumBytes = HAPInt32GetNumDescriptionBytes((int32_t)(value)); \
                HAPAssert(actualNumBytes == sizeof(expectedDescription) - 1); \
            } \
        } \
    } while (0)

int main() {
    // Zero.
    TEST_FROM_STRING("0", 0);
    TEST_FROM_STRING("+0", 0);
    TEST_FROM_STRING("-0", 0);
    TEST_FROM_STRING("00", 0);
    TEST_GET_DESCRIPTION(0, "0");

    // Some random numbers.
    TEST_FROM_STRING("1", 1);
    TEST_FROM_STRING("2", 2);
    TEST_FROM_STRING("123", 123);
    TEST_FROM_STRING("-1", -1);
    TEST_FROM_STRING("-2", -2);
    TEST_FROM_STRING("-123", -123);
    TEST_GET_DESCRIPTION(1, "1");
    TEST_GET_DESCRIPTION(2, "2");
    TEST_GET_DESCRIPTION(123, "123");
    TEST_GET_DESCRIPTION(-1, "-1");
    TEST_GET_DESCRIPTION(-2, "-2");
    TEST_GET_DESCRIPTION(2, "2");

    // Border cases (UInt64).
    TEST_BORDER_CASE("-10000000000000000000000000", 0);
    TEST_BORDER_CASE("-1", 0);
    TEST_FROM_STRING("-0", 0);
    TEST_FROM_STRING("+18446744073709551615", UINT64_MAX);
    TEST_GET_DESCRIPTION(UINT64_MAX, "18446744073709551615");
    TEST_BORDER_CASE("+18446744073709551616", UINT64_MAX);
    TEST_BORDER_CASE("+10000000000000000000000000", UINT64_MAX);

    // Border cases (Int64).
    TEST_BORDER_CASE("-10000000000000000000000000", INT64_MIN);
    TEST_BORDER_CASE("-9223372036854775809", INT64_MIN);
    TEST_FROM_STRING("-9223372036854775808", INT64_MIN);
    TEST_FROM_STRING("+9223372036854775807", INT64_MAX);
    TEST_GET_DESCRIPTION(INT64_MIN, "-9223372036854775808");
    TEST_GET_DESCRIPTION(INT64_MAX, "9223372036854775807");
    TEST_BORDER_CASE("+9223372036854775808", INT64_MAX);
    TEST_BORDER_CASE("+10000000000000000000000000", INT64_MAX);

    // Empty string.
    TEST_FAIL("");
    TEST_FAIL("+");
    TEST_FAIL("-");

    // Whitespace.
    TEST_FAIL(" 100");
    TEST_FAIL("1 00");
    TEST_FAIL("100 ");
    TEST_FAIL("+ 100");
    TEST_FAIL("+1 00");
    TEST_FAIL("+100 ");
    TEST_FAIL("- 100");
    TEST_FAIL("-1 00");
    TEST_FAIL("-100 ");

    // Invalid format.
    TEST_FAIL("21-50");
    TEST_FAIL("ff6600");
}
