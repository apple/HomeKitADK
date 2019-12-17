// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

int main() {
    HAPError err;

    HAPIPWriteContextRef writeContexts[128];

    {
        char request[] =
                "{\"characteristics\":["
                "{\"aid\":1,\"iid\":1,\"value\":\"Home\"},"
                "{\"aid\":1,\"iid\":2,\"value\":\"Home \\\"A\\\"\"},"
                "{\"aid\":1,\"iid\":3,\"value\":\"Home \\u0041\"},"
                "{\"aid\":1,\"iid\":4,\"value\":\"\\uD83d\\udE01\"},"
                "{\"aid\":1,\"iid\":5,\"value\":\"ABCabc123+/\\/\"}"
                "]}";
        uint64_t pid;
        bool pid_valid;
        size_t contexts_count;
        err = HAPIPAccessoryProtocolGetCharacteristicWriteRequests(
                request,
                sizeof request - 1,
                writeContexts,
                HAPArrayCount(writeContexts),
                &contexts_count,
                &pid_valid,
                &pid);
        HAPAssert(!err);
        HAPAssert(contexts_count == 5);
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &writeContexts[0];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_String);
        HAPAssert(writeContext->value.stringValue.bytes);
        HAPAssert(writeContext->value.stringValue.numBytes == 4);
        HAPAssert(HAPRawBufferAreEqual(HAPNonnull(writeContext->value.stringValue.bytes), "Home", 4));
        writeContext = (HAPIPWriteContext*) &writeContexts[1];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_String);
        HAPAssert(writeContext->value.stringValue.bytes);
        HAPAssert(writeContext->value.stringValue.numBytes == 8);
        HAPAssert(HAPRawBufferAreEqual(HAPNonnull(writeContext->value.stringValue.bytes), "Home \"A\"", 8));
        writeContext = (HAPIPWriteContext*) &writeContexts[2];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_String);
        HAPAssert(writeContext->value.stringValue.bytes);
        HAPAssert(writeContext->value.stringValue.numBytes == 6);
        HAPAssert(HAPRawBufferAreEqual(HAPNonnull(writeContext->value.stringValue.bytes), "Home A", 6));
        writeContext = (HAPIPWriteContext*) &writeContexts[3];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_String);
        HAPAssert(writeContext->value.stringValue.bytes);
        HAPAssert(writeContext->value.stringValue.numBytes == 4);
        const char emoji[] = { (char) 0xf0, (char) 0x9f, (char) 0x98, (char) 0x81 };
        HAPAssert(HAPRawBufferAreEqual(HAPNonnull(writeContext->value.stringValue.bytes), emoji, 4));
        writeContext = (HAPIPWriteContext*) &writeContexts[4];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_String);
        HAPAssert(writeContext->value.stringValue.bytes);
        HAPAssert(writeContext->value.stringValue.numBytes == 12);
        HAPAssert(HAPRawBufferAreEqual(HAPNonnull(writeContext->value.stringValue.bytes), "ABCabc123+//", 12));
    }

    return 0;
}
