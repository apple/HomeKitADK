// Copyright (c) 2015-2019 The HomeKit ADK Contributors
//
// Licensed under the Apache License, Version 2.0 (the “License”);
// you may not use this file except in compliance with the License.
// See [CONTRIBUTORS.md] for the list of HomeKit ADK project authors.

#include <stdio.h>

#include "HAPPlatform.h"

HAP_RESULT_USE_CHECK
HAPPlatformLogEnabledTypes HAPPlatformLogGetEnabledTypes(const HAPLogObject* log HAP_UNUSED) {
    return kHAPPlatformLogEnabledTypes_Debug;
}

void HAPPlatformLogCapture(
        const HAPLogObject* log,
        HAPLogType type,
        const char* message,
        const void* _Nullable bufferBytes,
        size_t numBufferBytes) HAP_DIAGNOSE_ERROR(!bufferBytes && numBufferBytes, "empty buffer cannot have a length") {
    // Color.
    switch (type) {
        case kHAPLogType_Debug: {
            fprintf(stderr, "\x1B[0m");
        } break;
        case kHAPLogType_Info: {
            fprintf(stderr, "\x1B[32m");
        } break;
        case kHAPLogType_Default: {
            fprintf(stderr, "\x1B[35m");
        } break;
        case kHAPLogType_Error: {
            fprintf(stderr, "\x1B[31m");
        } break;
        case kHAPLogType_Fault: {
            fprintf(stderr, "\x1B[1m\x1B[31m");
        } break;
    }

    // Highlight test logs.
    if (log == &kHAPLog_Default) {
        fprintf(stderr, "--------------------------------------------------------------------------------\n");
    }

    // Time.
    HAPTime now = HAPPlatformClockGetCurrent();
    (void) fprintf(
            stderr, "%8llu.%03llu", (unsigned long long) (now / HAPSecond), (unsigned long long) (now % HAPSecond));
    (void) fprintf(stderr, "\t");

    // Type.
    switch (type) {
        case kHAPLogType_Debug:
            (void) fprintf(stderr, "Debug");
            break;
        case kHAPLogType_Info:
            (void) fprintf(stderr, "Info");
            break;
        case kHAPLogType_Default:
            (void) fprintf(stderr, "Default");
            break;
        case kHAPLogType_Error:
            (void) fprintf(stderr, "Error");
            break;
        case kHAPLogType_Fault:
            (void) fprintf(stderr, "Fault");
            break;
    }
    (void) fprintf(stderr, "\t");

    // Subsystem / Category.
    if (log->subsystem) {
        (void) fprintf(stderr, "[%s", log->subsystem);
        if (log->category) {
            (void) fprintf(stderr, ":%s", log->category);
        }
        (void) fprintf(stderr, "] ");
    }

    // Message.
    (void) fprintf(stderr, "%s", message);
    (void) fprintf(stderr, "\n");

    // Buffer.
    if (bufferBytes) {
        size_t i, n;
        const uint8_t* b = bufferBytes;
        size_t length = numBufferBytes;
        if (length == 0) {
            (void) fprintf(stderr, "\n");
        } else {
            i = 0;
            do {
                (void) fprintf(stderr, "    %04zx ", i);
                for (n = 0; n != 8 * 4; n++) {
                    if (n % 4 == 0) {
                        (void) fprintf(stderr, " ");
                    }
                    if ((n <= length) && (i < length - n)) {
                        (void) fprintf(stderr, "%02x", b[i + n] & 0xff);
                    } else {
                        (void) fprintf(stderr, "  ");
                    }
                };
                (void) fprintf(stderr, "    ");
                for (n = 0; n != 8 * 4; n++) {
                    if (i != length) {
                        if ((32 <= b[i]) && (b[i] < 127)) {
                            (void) fprintf(stderr, "%c", b[i]);
                        } else {
                            (void) fprintf(stderr, ".");
                        }
                        i++;
                    }
                }
                (void) fprintf(stderr, "\n");
            } while (i != length);
        }
    }

    // Highlight test logs.
    if (log == &kHAPLog_Default) {
        fprintf(stderr, "--------------------------------------------------------------------------------\n");
    }

    // Reset color.
    fprintf(stderr, "\x1B[0m");

    // Finish log.
    (void) fflush(stderr);
}
