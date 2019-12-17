// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

HAP_DIAGNOSTIC_IGNORED_CLANG("-Wfloat-equal")

#define TEST_FROM_STRING(description, expectedValue) \
    do { \
        HAPPrecondition((description) != NULL); \
\
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s", description); \
        float value; \
        err = HAPFloatFromString(description, &value); \
        HAPAssert(!err); \
        HAPAssert(value == (expectedValue)); \
    } while (0)

#define TEST_FAIL(description) \
    do { \
        HAPPrecondition((description) != NULL); \
\
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s (expected fail)", description); \
        float value; \
        err = HAPFloatFromString(description, &value); \
        HAPAssert(err); \
    } while (0)

#define TEST_GET_DESCRIPTION(value) \
    do { \
        HAPError err; \
\
        float newValue; \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        err = HAPFloatGetDescription(string, sizeof string, value); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        err = HAPFloatFromString(string, &newValue); \
        HAPAssert(!err); \
        HAPAssert(value == newValue); \
    } while (0)

#define TEST_GET_FRACTION(input, expectedValue) \
    do { \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        HAPError err = HAPFloatGetDescription(string, sizeof string, input); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        float value = HAPFloatGetFraction(input); \
        if ((expectedValue) != (expectedValue)) { \
            HAPAssert(value != value); \
        } else { \
            HAPAssert(value == (expectedValue)); \
        } \
    } while (0)

#define TEST_ABSOLUTE_VALUE(input, expectedValue) \
    do { \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        HAPError err = HAPFloatGetDescription(string, sizeof string, input); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        float value = HAPFloatGetAbsoluteValue(input); \
        if ((expectedValue) != (expectedValue)) { \
            HAPAssert(value != value); \
        } else { \
            HAPAssert(value == (expectedValue)); \
        } \
    } while (0)

#define TEST_IS_ZERO(input, expectedValue) \
    do { \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        HAPError err = HAPFloatGetDescription(string, sizeof string, input); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        float value = HAPFloatIsZero(input); \
        HAPAssert(value == (expectedValue)); \
    } while (0)

#define TEST_IS_FINITE(input, expectedValue) \
    do { \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        HAPError err = HAPFloatGetDescription(string, sizeof string, input); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        float value = HAPFloatIsFinite(input); \
        HAPAssert(value == (expectedValue)); \
    } while (0)

#define TEST_IS_INFINITE(input, expectedValue) \
    do { \
        char string[kHAPFloat_MaxDescriptionBytes + 1]; \
        HAPError err = HAPFloatGetDescription(string, sizeof string, input); \
        HAPAssert(!err); \
        HAPLogInfo(&kHAPLog_Default, "Testing %s", string); \
        float value = HAPFloatIsInfinite(input); \
        HAPAssert(value == (expectedValue)); \
    } while (0)

#define INF (1.0F / 0.0F)
#define NAN (0.0F / 0.0F)

int main() {
    // Zero.
    TEST_FROM_STRING("0", 0.0F);
    TEST_FROM_STRING("-0", -0.0F);
    TEST_FROM_STRING("+0", 0.0F);
    TEST_FROM_STRING("00", 0.0F);
    TEST_FROM_STRING("0E7", 0.0F);

    // Some random numbers.
    TEST_FROM_STRING("1", 1.0F);
    TEST_FROM_STRING("2", 2.0F);
    TEST_FROM_STRING("123", 123.0F);
    TEST_FROM_STRING("12300000000", 12300000000.0F);
    TEST_FROM_STRING("0.123", 0.123F);
    TEST_FROM_STRING("0.00000000123", 0.00000000123F);
    TEST_FROM_STRING("12.3E0", 12.3E0F);
    TEST_FROM_STRING("12.3E1", 12.3E1F);
    TEST_FROM_STRING("12.3E-1", 12.3E-1F);
    TEST_FROM_STRING("12.3E+20", 12.3E20F);
    TEST_FROM_STRING("12.3E-20", 12.3E-20F);
    TEST_FROM_STRING("-1", -1.0F);
    TEST_FROM_STRING("-2", -2.0F);
    TEST_FROM_STRING("-123", -123.0F);
    TEST_FROM_STRING("-12300000000", -12300000000.0F);
    TEST_FROM_STRING("-0.123", -0.123F);
    TEST_FROM_STRING("-0.00000000123", -0.00000000123F);
    TEST_FROM_STRING("-12.3E0", -12.3E0F);
    TEST_FROM_STRING("-12.3E1", -12.3E1F);
    TEST_FROM_STRING("-12.3E-1", -12.3E-1F);
    TEST_FROM_STRING("-12.3E+20", -12.3E20F);
    TEST_FROM_STRING("-12.3E-20", -12.3E-20F);
    TEST_FROM_STRING("7.038531e-26", 0x0.AE43FDP-83F);

    // Rounding
    TEST_FROM_STRING("16384.0029296875", 0x800002P-9F);
    TEST_FROM_STRING("16384.0029296874999", 0x800001P-9F);
    TEST_FROM_STRING("16384.0048828125", 0x800002P-9F);
    TEST_FROM_STRING("16384.0048828125001", 0x800003P-9F);

    // Border cases.
    TEST_FROM_STRING("3.402823466E38", 3.402823466E38F);   // max float
    TEST_FROM_STRING("3.402823669E38", INF);               // overflow to infinity
    TEST_FROM_STRING("1.175494351E-38", 1.175494351E-38F); // min normalized
    TEST_FROM_STRING("1.4E-45", 1.4E-45F);                 // min float
    TEST_FROM_STRING("0.7E-45", 0.0F);                     // underflow to 0

    // Empty string.
    TEST_FAIL("");
    TEST_FAIL("+");
    TEST_FAIL("-");
    TEST_FAIL("e5");
    TEST_FAIL(".e5");
    TEST_FAIL("1.0e");

    // Whitespace.
    TEST_FAIL(" 10.0");
    TEST_FAIL("1 0.0");
    TEST_FAIL("10.0 ");
    TEST_FAIL("+ 10.0");
    TEST_FAIL("+1 0.0");
    TEST_FAIL("+10.0 ");
    TEST_FAIL("- 10.0");
    TEST_FAIL("-1 0.0");
    TEST_FAIL("-10.0 ");
    TEST_FAIL("1.0 e10");
    TEST_FAIL("1.0e 10");
    TEST_FAIL("1.0e1 0");

    // Invalid format.
    TEST_FAIL("21-5.0");
    TEST_FAIL("ff660.0");
    TEST_FAIL("1.0+10");
    TEST_FAIL("1.0e1.0");
    TEST_FAIL("1,0");

    // To string and back.
    TEST_GET_DESCRIPTION(0.0F);
    TEST_GET_DESCRIPTION(-0.0F);
    TEST_GET_DESCRIPTION(1.0F);
    TEST_GET_DESCRIPTION(2.0F);
    TEST_GET_DESCRIPTION(123.0F);
    TEST_GET_DESCRIPTION(12345678.0F);
    TEST_GET_DESCRIPTION(0.0007F);
    TEST_GET_DESCRIPTION(0.00007F);
    TEST_GET_DESCRIPTION(0x1.0P-149F);
    TEST_GET_DESCRIPTION(0x0.FFFFFEP-126F);
    TEST_GET_DESCRIPTION(0x1.000000P-126F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP-125F);
    TEST_GET_DESCRIPTION(0x1.000000P-125F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP-100F);
    TEST_GET_DESCRIPTION(0x1.000000P-100F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP-10F);
    TEST_GET_DESCRIPTION(0x1.000000P-10F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP-1F);
    TEST_GET_DESCRIPTION(0x1.000000P-1F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP0F);
    TEST_GET_DESCRIPTION(0x1.000000P0F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP1F);
    TEST_GET_DESCRIPTION(0x1.000000P1F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP10F);
    TEST_GET_DESCRIPTION(0x1.000000P10F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP100F);
    TEST_GET_DESCRIPTION(0x1.000000P100F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP127F);
    TEST_GET_DESCRIPTION(0x1.000000P127F);
    TEST_GET_DESCRIPTION(0x0.FFFFFFP128F);

#if defined(HAP_LONG_TESTS) && HAP_LONG_TESTS != 0
    // Full to string / from string test (runs for hours)
    uint32_t bitPattern;
    for (bitPattern = 0; bitPattern < 0x7F800000; bitPattern++) {
        float floatValue = HAPFloatFromBitPattern(bitPattern);
        TEST_GET_DESCRIPTION(floatValue);
    }
#endif

    // Fraction.
    TEST_GET_FRACTION(1.0F, 0.0F);
    TEST_GET_FRACTION(1.5F, 0.5F);
    TEST_GET_FRACTION(-1.5F, -0.5F);
    TEST_GET_FRACTION(0x1.000002P0F, 0x1P-23F);
    TEST_GET_FRACTION(0x1.FFFFFEP0F, 0x0.FFFFFEP0F);
    TEST_GET_FRACTION(0x0.FFFFFEP0F, 0x0.FFFFFEP0F);
    TEST_GET_FRACTION(1.4E-45F, 1.4E-45F);
    TEST_GET_FRACTION(0x7FFFFF.8P0F, 0.5F);
    TEST_GET_FRACTION(0xFFFFFF.0P0F, 0.0F);
    TEST_GET_FRACTION(INF, NAN);
    TEST_GET_FRACTION(NAN, NAN);

    // Absolute value.
    TEST_ABSOLUTE_VALUE(0.0F, 0.0F);
    TEST_ABSOLUTE_VALUE(-0.0F, 0.0F);
    TEST_ABSOLUTE_VALUE(1.0F, 1.0F);
    TEST_ABSOLUTE_VALUE(-1.0F, 1.0F);
    TEST_ABSOLUTE_VALUE(1.4E-45F, 1.4E-45F);
    TEST_ABSOLUTE_VALUE(-1.4E-45F, 1.4E-45F);
    TEST_ABSOLUTE_VALUE(INF, INF);
    TEST_ABSOLUTE_VALUE(-INF, INF);
    TEST_ABSOLUTE_VALUE(NAN, NAN);

    // Is zero.
    TEST_IS_ZERO(0.0F, true);
    TEST_IS_ZERO(-0.0F, true);
    TEST_IS_ZERO(1.0F, false);
    TEST_IS_ZERO(-1.0F, false);
    TEST_IS_ZERO(1.4E-45F, false);
    TEST_IS_ZERO(-1.4E-45F, false);
    TEST_IS_ZERO(INF, false);
    TEST_IS_ZERO(-INF, false);
    TEST_IS_ZERO(NAN, false);

    // Is finite.
    TEST_IS_FINITE(0.0F, true);
    TEST_IS_FINITE(-0.0F, true);
    TEST_IS_FINITE(1.0F, true);
    TEST_IS_FINITE(-1.0F, true);
    TEST_IS_FINITE(0x0.FFFFFFP128F, true);
    TEST_IS_FINITE(-0x0.FFFFFFP128F, true);
    TEST_IS_FINITE(INF, false);
    TEST_IS_FINITE(-INF, false);
    TEST_IS_FINITE(NAN, false);

    // Is infinite.
    TEST_IS_INFINITE(0.0F, false);
    TEST_IS_INFINITE(-0.0F, false);
    TEST_IS_INFINITE(1.0F, false);
    TEST_IS_INFINITE(-1.0F, false);
    TEST_IS_INFINITE(0x0.FFFFFFP128F, false);
    TEST_IS_INFINITE(-0x0.FFFFFFP128F, false);
    TEST_IS_INFINITE(INF, true);
    TEST_IS_INFINITE(-INF, true);
    TEST_IS_INFINITE(NAN, false);
}
