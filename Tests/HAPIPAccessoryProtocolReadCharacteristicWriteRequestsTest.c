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
        // Test vector.
        // See HomeKit Accessory Protocol Specification R14
        // Section 6.7.2.4 Timed Write Procedures
        char request[] =
                "{\n"
                "\"characteristics\": [{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 6,\n"
                "\"value\" : 1\n"
                "\n"
                "},\n"
                "{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 7,\n"
                "\"value\" : 3\n"
                "\n"
                "},\n"
                "{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 8,\n"
                "\"value\" : 4\n"
                "\n"
                "}],\n"
                "\"pid\" : 11122333\n"
                "}\n";
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
        HAPAssert(contexts_count == 3);
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &writeContexts[0];
        HAPAssert(writeContext->aid == 2);
        HAPAssert(writeContext->iid == 6);
        HAPAssert(writeContext->status == 0);
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 1);
        HAPAssert(writeContext->ev == kHAPIPEventNotificationState_Undefined);
        HAPAssert(writeContext->authorizationData.bytes == NULL);
        HAPAssert(writeContext->authorizationData.numBytes == 0);
        HAPAssert(writeContext->remote == false);
        writeContext = (HAPIPWriteContext*) &writeContexts[1];
        HAPAssert(writeContext->aid == 2);
        HAPAssert(writeContext->iid == 7);
        HAPAssert(writeContext->status == 0);
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 3);
        HAPAssert(writeContext->ev == kHAPIPEventNotificationState_Undefined);
        HAPAssert(writeContext->authorizationData.bytes == NULL);
        HAPAssert(writeContext->authorizationData.numBytes == 0);
        HAPAssert(writeContext->remote == false);
        writeContext = (HAPIPWriteContext*) &writeContexts[2];
        HAPAssert(writeContext->aid == 2);
        HAPAssert(writeContext->iid == 8);
        HAPAssert(writeContext->status == 0);
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 4);
        HAPAssert(writeContext->ev == kHAPIPEventNotificationState_Undefined);
        HAPAssert(writeContext->authorizationData.bytes == NULL);
        HAPAssert(writeContext->authorizationData.numBytes == 0);
        HAPAssert(writeContext->remote == false);
        HAPAssert(pid_valid);
        HAPAssert(pid == 11122333);
    }
    {
        // Detect duplicate PID.
        char request[] =
                "{\n"
                "\"characteristics\": [{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 6,\n"
                "\"value\" : 1\n"
                "\n"
                "},\n"
                "{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 7,\n"
                "\"value\" : 3\n"
                "\n"
                "},\n"
                "{\n"
                "\"aid\" : 2,\n"
                "\"iid\" : 8,\n"
                "\"value\" : 4\n"
                "\n"
                "}],\n"
                "\"pid\" : 11122333,\n"
                "\"pid\" : 11122333,\n"
                "}\n";
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
        HAPAssert(err == kHAPError_InvalidData);
    }
    {
        char request[] =
                "{\"characteristics\":["
                "{\"aid\":1,\"iid\":1,\"value\":-2147483648},"
                "{\"aid\":1,\"iid\":2,\"value\":-1},"
                "{\"aid\":1,\"iid\":3,\"value\":0},"
                "{\"aid\":1,\"iid\":4,\"value\":1},"
                "{\"aid\":1,\"iid\":5,\"value\":2147483648},"
                "{\"aid\":1,\"iid\":6,\"value\":4294967296},"
                "{\"aid\":1,\"iid\":7,\"value\":9223372036854775808},"
                "{\"aid\":1,\"iid\":8,\"value\":18446744073709551615}]}";
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
        HAPAssert(contexts_count == 8);
        HAPIPWriteContext* writeContext = (HAPIPWriteContext*) &writeContexts[0];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_Int);
        HAPAssert(writeContext->value.intValue == -2147483648);
        writeContext = (HAPIPWriteContext*) &writeContexts[1];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_Int);
        HAPAssert(writeContext->value.intValue == -1);
        writeContext = (HAPIPWriteContext*) &writeContexts[2];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 0);
        writeContext = (HAPIPWriteContext*) &writeContexts[3];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 1);
        writeContext = (HAPIPWriteContext*) &writeContexts[4];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 2147483648);
        writeContext = (HAPIPWriteContext*) &writeContexts[5];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 4294967296);
        writeContext = (HAPIPWriteContext*) &writeContexts[6];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 9223372036854775808ULL);
        writeContext = (HAPIPWriteContext*) &writeContexts[7];
        HAPAssert(writeContext->type == kHAPIPWriteValueType_UInt);
        HAPAssert(writeContext->value.unsignedIntValue == 18446744073709551615ULL);
    }

    return 0;
}
