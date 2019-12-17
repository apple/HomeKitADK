// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "util_base64.h"

static void TestString(const char* string, const char* encodedString) {
    HAPPrecondition(string);
    HAPPrecondition(encodedString);

    HAPLogInfo(&kHAPLog_Default, "util_base64_test: BASE64(\"%s\") = \"%s\"\n", string, encodedString);

    {
        char bytes[1024];
        size_t numBytes;
        util_base64_encode(string, HAPStringGetNumBytes(string), bytes, sizeof bytes, &numBytes);
        HAPAssert(numBytes == HAPStringGetNumBytes(encodedString));
        HAPAssert(HAPRawBufferAreEqual(bytes, encodedString, numBytes));
    }

    {
        char bytes[1024];
        size_t numBytes;
        HAPError err =
                util_base64_decode(encodedString, HAPStringGetNumBytes(encodedString), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == HAPStringGetNumBytes(string));
        HAPAssert(HAPRawBufferAreEqual(bytes, string, numBytes));
    }
}

static void TestRawBuffer(const uint8_t* testBytes, size_t numTestBytes, const char* encodedString) {
    HAPPrecondition(testBytes);
    HAPPrecondition(encodedString);

    HAPLogBufferInfo(
            &kHAPLog_Default, testBytes, numTestBytes, "util_base64_test: BASE64(<buffer>) = \"%s\"\n", encodedString);

    {
        char bytes[1024];
        size_t numBytes;
        util_base64_encode(testBytes, numTestBytes, bytes, sizeof bytes, &numBytes);
        HAPAssert(numBytes == HAPStringGetNumBytes(encodedString));
        HAPAssert(HAPRawBufferAreEqual(bytes, encodedString, numBytes));
    }

    {
        uint8_t bytes[1024];
        size_t numBytes;
        HAPError err =
                util_base64_decode(encodedString, HAPStringGetNumBytes(encodedString), bytes, sizeof bytes, &numBytes);
        HAPAssert(!err);
        HAPAssert(numBytes == numTestBytes);
        HAPAssert(HAPRawBufferAreEqual(bytes, testBytes, numBytes));
    }
}

static void TestInvalidDecode(const char* encodedString) {
    HAPPrecondition(encodedString);

    HAPLogInfo(&kHAPLog_Default, "util_base64_test: Illegal string: %s", encodedString);

    {
        uint8_t bytes[1024];
        size_t numBytes;
        HAPError err =
                util_base64_decode(encodedString, HAPStringGetNumBytes(encodedString), bytes, sizeof bytes, &numBytes);
        HAPAssert(err);
    }
}

int main() {
    // See https://tools.ietf.org/html/rfc4648
    TestString("", "");
    TestString("f", "Zg==");
    TestString("fo", "Zm8=");
    TestString("foo", "Zm9v");
    TestString("foob", "Zm9vYg==");
    TestString("fooba", "Zm9vYmE=");
    TestString("foobar", "Zm9vYmFy");

    TestRawBuffer(
            (uint8_t[]) { 0x01, 0x15, 0x02, 0x01, 0x00, 0x01, 0x10, 0x27, 0x6D, 0x49, 0x8E, 0x54,
                          0xE9, 0x46, 0x66, 0xB0, 0xE5, 0x35, 0xA9, 0x66, 0x44, 0x12, 0x64 },
            23,
            "ARUCAQABECdtSY5U6UZmsOU1qWZEEmQ=");

    TestInvalidDecode("\"");
}
