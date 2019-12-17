// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

void HAPIPByteBufferClear(HAPIPByteBuffer* byteBuffer) {
    HAPPrecondition(byteBuffer);

    byteBuffer->position = 0;
    byteBuffer->limit = byteBuffer->capacity;
}

void HAPIPByteBufferFlip(HAPIPByteBuffer* byteBuffer) {
    HAPPrecondition(byteBuffer);

    byteBuffer->limit = byteBuffer->position;
    byteBuffer->position = 0;
}

void HAPIPByteBufferShiftLeft(HAPIPByteBuffer* byteBuffer, size_t numBytes) {
    HAPPrecondition(byteBuffer);
    HAPPrecondition(byteBuffer->data);
    HAPPrecondition(numBytes <= byteBuffer->position);
    HAPPrecondition(byteBuffer->position <= byteBuffer->limit);
    HAPPrecondition(byteBuffer->limit <= byteBuffer->capacity);

    HAPRawBufferCopyBytes(byteBuffer->data, &byteBuffer->data[numBytes], byteBuffer->position - numBytes);
    byteBuffer->position -= numBytes;
    byteBuffer->limit -= numBytes;
}

HAP_PRINTFLIKE(2, 3)
HAP_RESULT_USE_CHECK
HAPError HAPIPByteBufferAppendStringWithFormat(HAPIPByteBuffer* byteBuffer, const char* format, ...) {
    HAPPrecondition(byteBuffer);
    HAPPrecondition(byteBuffer->data);
    HAPPrecondition(byteBuffer->position <= byteBuffer->limit);
    HAPPrecondition(byteBuffer->limit <= byteBuffer->capacity);

    HAPError err;

    va_list args;
    va_start(args, format);
    err = HAPStringWithFormatAndArguments(
            &byteBuffer->data[byteBuffer->position], byteBuffer->limit - byteBuffer->position, format, args);
    va_end(args);
    if (err) {
        HAPAssert(kHAPError_OutOfResources);
        return err;
    }
    byteBuffer->position += HAPStringGetNumBytes(&byteBuffer->data[byteBuffer->position]);
    return kHAPError_None;
}
