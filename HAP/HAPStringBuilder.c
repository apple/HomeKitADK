// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include "HAP+Internal.h"

typedef struct {
    char* bytes;
    size_t maxBytes;
    size_t numBytes;
    bool didOverflow : 1;
} HAPStringBuilder;
HAP_STATIC_ASSERT(sizeof(HAPStringBuilderRef) >= sizeof(HAPStringBuilder), HAPStringBuilder);

void HAPStringBuilderCreate(HAPStringBuilderRef* stringBuilder_, char* bytes, size_t maxBytes) {
    HAPPrecondition(stringBuilder_);
    HAPStringBuilder* stringBuilder = (HAPStringBuilder*) stringBuilder_;
    HAPPrecondition(bytes);

    HAPRawBufferZero(stringBuilder_, sizeof *stringBuilder_);
    stringBuilder->bytes = bytes;
    stringBuilder->maxBytes = maxBytes;
    if (stringBuilder->maxBytes < 1) {
        stringBuilder->didOverflow = true;
        return;
    }
    stringBuilder->bytes[0] = '\0';
}

HAP_RESULT_USE_CHECK
bool HAPStringBuilderDidOverflow(HAPStringBuilderRef* stringBuilder_) {
    HAPPrecondition(stringBuilder_);
    HAPStringBuilder* stringBuilder = (HAPStringBuilder*) stringBuilder_;

    return stringBuilder->didOverflow;
}

HAP_RESULT_USE_CHECK
const char* HAPStringBuilderGetString(HAPStringBuilderRef* stringBuilder_) {
    HAPPrecondition(stringBuilder_);
    HAPStringBuilder* stringBuilder = (HAPStringBuilder*) stringBuilder_;

    if (stringBuilder->numBytes == 0) {
        return "";
    }
    return stringBuilder->bytes;
}

HAP_RESULT_USE_CHECK
size_t HAPStringBuilderGetNumBytes(HAPStringBuilderRef* stringBuilder_) {
    HAPPrecondition(stringBuilder_);
    HAPStringBuilder* stringBuilder = (HAPStringBuilder*) stringBuilder_;

    return stringBuilder->numBytes;
}

HAP_PRINTFLIKE(2, 3)
void HAPStringBuilderAppend(HAPStringBuilderRef* stringBuilder, const char* format, ...) {
    va_list args;
    va_start(args, format);
    HAPStringBuilderAppendWithArguments(stringBuilder, format, args);
    va_end(args);
}

HAP_PRINTFLIKE(2, 0)
void HAPStringBuilderAppendWithArguments(HAPStringBuilderRef* stringBuilder_, const char* format, va_list arguments) {
    HAPPrecondition(stringBuilder_);
    HAPStringBuilder* stringBuilder = (HAPStringBuilder*) stringBuilder_;
    HAPPrecondition(format);

    HAPError err;

    if (stringBuilder->didOverflow) {
        return;
    }

    err = HAPStringWithFormatAndArguments(
            &stringBuilder->bytes[stringBuilder->numBytes],
            stringBuilder->maxBytes - stringBuilder->numBytes,
            format,
            arguments);
    if (err) {
        HAPAssert(err == kHAPError_OutOfResources);
        stringBuilder->bytes[stringBuilder->maxBytes - 1] = '\0';
        stringBuilder->didOverflow = true;
    }
    stringBuilder->numBytes += HAPStringGetNumBytes(&stringBuilder->bytes[stringBuilder->numBytes]);
    HAPAssert(stringBuilder->numBytes < stringBuilder->maxBytes);
    HAPAssert(stringBuilder->bytes[stringBuilder->numBytes] == '\0');
}
