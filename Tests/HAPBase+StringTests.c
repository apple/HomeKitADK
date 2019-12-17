// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAPPlatform.h"

#define TEST(expectedString, format, ...) \
    do { \
        HAPError err; \
\
        HAPLogInfo(&kHAPLog_Default, "Testing %s", format); \
        char actualString[sizeof expectedString] = { 0 }; \
        err = HAPStringWithFormat(actualString, sizeof actualString, format, ##__VA_ARGS__); \
        HAPAssert(!err); \
        HAPAssert(HAPStringGetNumBytes(actualString) == sizeof expectedString - 1); \
        HAPAssert(HAPStringAreEqual(actualString, expectedString)); \
    } while (0)

static const char* null() {
    return NULL;
}

int main() {
    TEST("value: [77%] blabla", "value: [%d%%] blabla", 77);
    TEST("12:34:56:78:9A:BC", "%02X:%02X:%02X:%02X:%02X:%02X", 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC);

    TEST("77", "%d", 77);
    TEST("-77", "%d", -77);
    TEST("  77", "%4d", 77);
    TEST(" -77", "%4d", -77);
    TEST("77", "%02d", 77);
    TEST("-77", "%02d", -77);
    TEST("0077", "%04d", 77);
    TEST("-077", "%04d", -77);
    TEST(" +77", "%+4d", 77);
    TEST(" -77", "%+4d", -77);
    TEST("+77", "%+02d", 77);
    TEST("-77", "%+02d", -77);
    TEST("+077", "%+04d", 77);
    TEST("-077", "%+04d", -77);
    TEST("  77", "% 4d", 77);
    TEST(" -77", "% 4d", -77);
    TEST(" 77", "% 02d", 77);
    TEST("-77", "% 02d", -77);
    TEST(" 077", "% 04d", 77);
    TEST("-077", "% 04d", -77);

    TEST("77", "%ld", 77l);
    TEST("-77", "%ld", -77l);
    TEST("  77", "%4ld", 77l);
    TEST(" -77", "%4ld", -77l);
    TEST("77", "%02ld", 77l);
    TEST("-77", "%02ld", -77l);
    TEST("0077", "%04ld", 77l);
    TEST("-077", "%04ld", -77l);
    TEST(" +77", "%+4ld", 77l);
    TEST(" -77", "%+4ld", -77l);
    TEST("+77", "%+02ld", 77l);
    TEST("-77", "%+02ld", -77l);
    TEST("+077", "%+04ld", 77l);
    TEST("-077", "%+04ld", -77l);
    TEST("  77", "% 4ld", 77l);
    TEST(" -77", "% 4ld", -77l);
    TEST(" 77", "% 02ld", 77l);
    TEST("-77", "% 02ld", -77l);
    TEST(" 077", "% 04ld", 77l);
    TEST("-077", "% 04ld", -77l);

    TEST("7777777777777777", "%lld", 7777777777777777ll);
    TEST("-7777777777777777", "%lld", -7777777777777777ll);
    TEST("    7777777777777777", "%20lld", 7777777777777777ll);
    TEST("   -7777777777777777", "%20lld", -7777777777777777ll);
    TEST("7777777777777777", "%016lld", 7777777777777777ll);
    TEST("-7777777777777777", "%016lld", -7777777777777777ll);
    TEST("00007777777777777777", "%020lld", 7777777777777777ll);
    TEST("-0007777777777777777", "%020lld", -7777777777777777ll);
    TEST("   +7777777777777777", "%+20lld", 7777777777777777ll);
    TEST("   -7777777777777777", "%+20lld", -7777777777777777ll);
    TEST("+7777777777777777", "%+016lld", 7777777777777777ll);
    TEST("-7777777777777777", "%+016lld", -7777777777777777ll);
    TEST("+0007777777777777777", "%+020lld", 7777777777777777ll);
    TEST("-0007777777777777777", "%+020lld", -7777777777777777ll);
    TEST("    7777777777777777", "% 20lld", 7777777777777777ll);
    TEST("   -7777777777777777", "% 20lld", -7777777777777777ll);
    TEST(" 7777777777777777", "% 016lld", 7777777777777777ll);
    TEST("-7777777777777777", "% 016lld", -7777777777777777ll);
    TEST(" 0007777777777777777", "% 020lld", 7777777777777777ll);
    TEST("-0007777777777777777", "% 020lld", -7777777777777777ll);

    TEST("77", "%u", 77);
    TEST("  77", "%4u", 77);
    TEST("77", "%02u", 77);
    TEST("0077", "%04u", 77);

    TEST("77", "%lu", 77l);
    TEST("  77", "%4lu", 77l);
    TEST("77", "%02lu", 77l);
    TEST("0077", "%04lu", 77l);

    TEST("17777777777777777777", "%llu", 17777777777777777777llu);
    TEST("  17777777777777777777", "%22llu", 17777777777777777777llu);
    TEST("0017777777777777777777", "%022llu", 17777777777777777777llu);

    TEST("4d", "%x", 77);
    TEST("4d", "%02x", 77);
    TEST("  4d", "%4x", 77);
    TEST("004d", "%04x", 77);
    TEST("4D", "%X", 77);
    TEST("4D", "%02X", 77);
    TEST("  4D", "%4X", 77);
    TEST("004D", "%04X", 77);

    TEST("4d", "%lx", 77l);
    TEST("4d", "%02lx", 77l);
    TEST("  4d", "%4lx", 77l);
    TEST("004d", "%04lx", 77l);
    TEST("4D", "%lX", 77l);
    TEST("4D", "%02lX", 77l);
    TEST("  4D", "%4lX", 77l);
    TEST("004D", "%04lX", 77l);

    TEST("1234567890abcdef", "%llx", 0x1234567890ABCDEFll);
    TEST("    1234567890abcdef", "%20llx", 0x1234567890ABCDEFll);
    TEST("00001234567890abcdef", "%020llx", 0x1234567890ABCDEFll);
    TEST("1234567890ABCDEF", "%llX", 0x1234567890ABCDEFll);
    TEST("    1234567890ABCDEF", "%20llX", 0x1234567890ABCDEFll);
    TEST("00001234567890ABCDEF", "%020llX", 0x1234567890ABCDEFll);

    TEST("   123456789", "%12zu", (size_t) 123456789);
    TEST("  0x12345678", "%12p", (void*) 0x12345678);
    TEST("  0xbeeffeed", "%12p", (void*) 0xBeefFeed);

    TEST("$", "%c", '$');
    TEST("  $", "%3c", '$');

    do {
        HAPError err;
        HAPLogInfo(&kHAPLog_Default, "Testing >%%c<");
        char actualString[4] = { 0 };
        err = HAPStringWithFormat(actualString, sizeof actualString, ">%c<", 0);
        HAPAssert(!err);
        HAPAssert(actualString[0] == '>');
        HAPAssert(actualString[1] == 0);
        HAPAssert(actualString[2] == '<');
        HAPAssert(actualString[3] == 0);
    } while (0);

    TEST("(null)", "%s", null());
    TEST("abcdefg", "%s", "abcdefg");
    TEST("   abcdefg", "%10s", "abcdefg");

    return 0;
}
